#define SCANNER_C

#ifndef COMMON_H
#include "common.h"
#endif
#ifndef ERROR_C
#include "error.c"
#endif
#ifndef TOKEN_C
#include "token.c"
#endif

typedef struct {
    Buffer *restrict buffer;
    char *cursor;
    char *token;
    bool eof;
    Token *tokens;

    // Loc *current;
    int line; // DELETE?
    int col;  // DELETE?
} Scanner;

void scanner_pp(const Scanner *s)
{
    printf("[Expr %p:%s] token:\"%s\" cursor:\"%s\"\n",
            (void *) s, bool_str(s->eof), unescaped(s->token), unescaped(s->cursor));
    printf("  Token: "); token_pp(&s->tokens[-1]);
}

bool is_alpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}

bool is_alphanumeric(const char c)
{
    return is_alpha(c) || is_digit(c);
}

int scanner_find_token_line_index(const Scanner *s)
{
    int line_index = buffer_find_line(s->buffer, s->token);
    if (line_index == -1) {
        fprintf(stderr, "Could not find token in scanner buffer; char %ld (%d lines)\n",
                s->token - s->buffer->head,
                s->buffer->num_lines);
        exit(ERR_SCANNER);
    }
    return line_index;
}

str scanner_buffer_line(const Scanner *s, int line_index)
{
    return buffer_get_line(s->buffer, line_index);
}

str scanner_token_range(const Scanner *s, str line)
{
    const char *end = line.head + line.len;
    int len = (end < s->cursor ? end : s->cursor) - s->token;
    return str_new_s(s->token, len);

}

void scanner_error(const Scanner *restrict s, const char *restrict message)
{
    int line_index = scanner_find_token_line_index(s);
    str line = scanner_buffer_line(s, line_index);
    str range = scanner_token_range(s, line);
    error(line_index+1, line, range, message);
}

void scanner_info(const Scanner *restrict s, const char *restrict message)
{
    int line_index = scanner_find_token_line_index(s);
    str line = scanner_buffer_line(s, line_index);
    str range = scanner_token_range(s, line);
    info(line_index+1, line, range, token_type_name(s->tokens-1));
}

Token *add_token_span(Scanner *restrict s, const TokenType type,
        const char *restrict from, const char *restrict to)
{
    Token *t = s->tokens++;
    t->type = type;
    t->lexeme = str_new_s(from, to-from);
    scanner_info(s, token_type_name(t));
    return t;
}

Token *add_token(Scanner *s, const TokenType type)
{
    return add_token_span(s, type, s->token, s->cursor);
}

char advance(Scanner *s)
{
    char c = *s->cursor;
    s->cursor++;
    s->eof = (*s->cursor == '\0');
    if (c == '\n') {
        // Or call a registered callback fn, e.g. s->newline_cb (take a char *)
        buffer_add_line(s->buffer, s->cursor);
    }
    return c;
}

bool scanner_match(Scanner *restrict s, const char expected)
{
    if (s->eof || *s->cursor != expected) {
        return false;
    }
    s->cursor++;
    return true;
}

Token *add_token_for(Scanner *s, const char c,
        const TokenType matched, const TokenType unmatched)
{
    return add_token(s, scanner_match(s, c) ? matched : unmatched);
}

void scan_until(Scanner *s, const char until)
{
    char c;
    for (c = *s->cursor; c != until && !s->eof; c = *s->cursor) {
        advance(s);
    }
}

Token *scan_comment(Scanner *s)
{
    scan_until(s, '\n');
    return add_token(s, TOKEN_COMMENT);
}

Token *scan_identifier(Scanner *s)
{
    while (is_alphanumeric(*s->cursor)) {
        advance(s);
    }
    Keyword *k = find_keyword(str_new_s(s->token, s->cursor - s->token));
    return add_token(s, k ? k->type : TOKEN_IDENTIFIER);
}

Token *scan_number(Scanner *s)
{
    while (is_digit(*s->cursor)) {
        advance(s);
    }
    if (*s->cursor == '.' && is_digit(s->cursor[1]))  {
        advance(s); // consume the `.`
        while (is_digit(*s->cursor)) {
            advance(s);
        }
    }
    return add_token(s, TOKEN_NUMBER);
}

Token *scan_string(Scanner *s)
{
    scan_until(s, '"');
    if (s->eof) {
        scanner_error(s, "Unterminated string.");
        // exit(1);
        return &TokenNone;
    }
    // Consume the closing double-quote and return string excluding quotes
    advance(s);
    return add_token_span(s, TOKEN_STRING, s->token+1, s->cursor-1);
}

Token *scan_token(Scanner *s)
{
    s->token = s->cursor;
    char c = advance(s);

    switch (c) {
        case ' ':  break;
        case '\t': break;
        case '\n': break;
        case '(':  return add_token(s, TOKEN_LEFT_PAREN);
        case ')':  return add_token(s, TOKEN_RIGHT_PAREN);
        case '{':  return add_token(s, TOKEN_LEFT_BRACE);
        case '}':  return add_token(s, TOKEN_RIGHT_BRACE);
        case ';':  return add_token(s, TOKEN_SEMICOLON);
        case ',':  return add_token(s, TOKEN_COMMA);
        case '.':  return add_token(s, TOKEN_DOT);
        case '-':  return add_token(s, TOKEN_MINUS);
        case '+':  return add_token(s, TOKEN_PLUS);
        case '*':  return add_token(s, TOKEN_STAR);
        case '!':  return add_token_for(s, '=', TOKEN_BANG_EQUAL, TOKEN_BANG);
        case '=':  return add_token_for(s, '=', TOKEN_EQUAL_EQUAL, TOKEN_EQUAL);
        case '<':  return add_token_for(s, '=', TOKEN_LESS_EQUAL, TOKEN_LESS);
        case '>':  return add_token_for(s, '=', TOKEN_GREATER_EQUAL, TOKEN_GREATER);
        case '"':  return scan_string(s);
        case '/':  return (scanner_match(s, '/')) ? scan_comment(s) : add_token(s, TOKEN_SLASH);
        case '\r': break;
        case '\0': return add_token(s, TOKEN_EOF);
        default:
                   if (is_digit(c)) {
                       return scan_number(s);
                   } else if (is_alpha(c)) {
                       return scan_identifier(s);
                   } else {
                       scanner_error(s, "Unexpected character.");
                       exit(1);
                   }
    }
    return &TokenNone;
}

static Token *scan(Scanner *s, Buffer *b, Token *tokens)
{
    s->buffer = b;
    s->cursor = b->head;
    s->token  = b->head;
    s->tokens = tokens;
    *s->tokens = TokenNone;
    s->eof = false;
    while (!s->eof) {
        scan_token(s);
    }
    return tokens;
}
