#include <stdbool.h>
#include <stddef.h> // ptrdiff_t
#include <stdlib.h> // exit
#include <stdio.h>  // printf, fprintf
#include <string.h> // memcmp, strlen

#define STR(x) #x
#define XSTR(x) STR(x)

#define ERR_USAGE    64
#define ERR_COMPILE  65
#define ERR_RUNTIME  70
#define ERR_FILE     74

static bool had_error;

bool is_alpha(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}

bool is_alphanumeric(const char c) {
    return is_alpha(c) || is_digit(c);
}

void report(const int line, const char *where, const char *message) {
    fprintf(stderr, "[line %d] Error%s: %s\n", line, where, message);
    had_error = true;
}

void error(const int line, const char *message) {
    report(line, "", message);
}

typedef enum {
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LEFT_BRACE,
    TOKEN_LEFT_PAREN,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_RIGHT_BRACE,
    TOKEN_RIGHT_PAREN,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    // Literals
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Keywords
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    //
    TOKEN_COMMENT,
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

static const char *TokenTypeNames[] = {
    "TOKEN_BANG",
    "TOKEN_BANG_EQUAL",
    "TOKEN_COMMA",
    "TOKEN_DOT",
    "TOKEN_EQUAL",
    "TOKEN_EQUAL_EQUAL",
    "TOKEN_GREATER",
    "TOKEN_GREATER_EQUAL",
    "TOKEN_LEFT_BRACE",
    "TOKEN_LEFT_PAREN",
    "TOKEN_LESS",
    "TOKEN_LESS_EQUAL",
    "TOKEN_MINUS",
    "TOKEN_PLUS",
    "TOKEN_RIGHT_BRACE",
    "TOKEN_RIGHT_PAREN",
    "TOKEN_SEMICOLON",
    "TOKEN_SLASH",
    "TOKEN_STAR",

    // Literals
    "TOKEN_IDENTIFIER",
    "TOKEN_STRING",
    "TOKEN_NUMBER",

    // Keywords
    "TOKEN_AND",
    "TOKEN_CLASS",
    "TOKEN_ELSE",
    "TOKEN_FALSE",
    "TOKEN_FOR",
    "TOKEN_FN",
    "TOKEN_IF",
    "TOKEN_NIL",
    "TOKEN_OR",
    "TOKEN_PRINT",
    "TOKEN_RETURN",
    "TOKEN_SUPER",
    "TOKEN_THIS",
    "TOKEN_TRUE",
    "TOKEN_VAR",
    "TOKEN_WHILE",

    "TOKEN_COMMENT",
    "TOKEN_ERROR",
    "TOKEN_EOF"
};


typedef struct {
  TokenType type;
  const char *pos;
  int len;
  int line;
} Token;

typedef struct {
  TokenType type;
  const char *name;
  int len;
} Keyword;

// The table of reserved words and their associated token types.
static Keyword keywords[] = {
  {TOKEN_AND,    "and",    3},
  {TOKEN_CLASS,  "class",  5},
  {TOKEN_ELSE,   "else",   4},
  {TOKEN_FALSE,  "false",  5},
  {TOKEN_FOR,    "for",    3},
  {TOKEN_FN,     "fn",     2},
  {TOKEN_IF,     "if",     2},
  {TOKEN_NIL,    "nil",    3},
  {TOKEN_OR,     "or",     2},
  {TOKEN_PRINT,  "print",  5},
  {TOKEN_RETURN, "return", 6},
  {TOKEN_SUPER,  "super" , 5},
  {TOKEN_THIS,   "this",   4},
  {TOKEN_TRUE,   "true",   4},
  {TOKEN_VAR,    "var",    3},
  {TOKEN_WHILE,  "while",  5},
  {TOKEN_EOF,    NULL,     0}
};
#define MIN_KEYWORD_LEN 2
#define MAX_KEYWORD_LEN 6

#define MAX_SCANNER_TOKENS 1024
typedef struct {
  // const char *source;
  const char *cursor;
  const char *token_start;
  int line;
  int num_tokens;
  bool eof;
  Token tokens[MAX_SCANNER_TOKENS];
} Scanner;

void scanner_init(Scanner *s, const char *source) {
    // s->source = source;
    s->cursor = source;
    s->token_start = source;
    s->line = 1;
    s->num_tokens = 0;
    s->eof = false;
}

void scanner_add_token_range(Scanner *s, const TokenType type,
        const char *restrict from, const char *restrict to)
{
  s->tokens[s->num_tokens++] =
      (Token) {
          .type = type,
          .pos = from,
          .len = (int)(to - from),
          .line = s->line,
      };
}

void scanner_add_token(Scanner *s, const TokenType type) {
    scanner_add_token_range(s, type, s->token_start, s->cursor);
}

char scanner_advance(Scanner *s) {
    char c = *s->cursor++;
    s->eof = (*s->cursor == '\0');
    // fprintf(stdout, "advance:  char:%c eof:%s\n", c, s->eof ? "true" : "false");
    return c;
}

void scanner_error(Scanner *s, const char *message) {
    error(s->line, message);
}

bool scanner_match(Scanner *s, const char expected) {
    if (s->eof || *s->cursor != expected) {
        return false;
    }
    s->cursor++;
    return true;
}

// char scanner_peek(Scanner *s) {
//     return (s->eof ? '\0' : *s->cursor);
// }

void scan_to(Scanner *s, const char until) {
    char c;
    for (c = *s->cursor; c != until && !s->eof; c = *s->cursor) {
        // fprintf(stdout, "scan_to:   %c peek:%c\n", until, c);
        if (c == '\n') {
            s->line++;
        }
        scanner_advance(s);
    }
}

void scan_comment(Scanner *s) {
    scan_to(s, '\n');
    scanner_add_token(s, TOKEN_COMMENT);
}

void scan_identifier(Scanner *s) {
    while (is_alphanumeric(*s->cursor)) {
        scanner_advance(s);
    }

    TokenType type = TOKEN_IDENTIFIER;
    ptrdiff_t len = s->cursor - s->token_start;
    if (MIN_KEYWORD_LEN <= len && len <= MAX_KEYWORD_LEN) {
        for (Keyword *keyword = keywords; keyword->name; keyword++) {
            if (len == keyword->len && memcmp(s->token_start, keyword->name, len) == 0) {
                type = keyword->type;
                break;
            }
        }
    }

    scanner_add_token(s, type);
}

void scan_number(Scanner *s) {
    while (is_digit(*s->cursor)) {
        scanner_advance(s);
    }
    if (s->cursor[0] == '.' && is_digit(s->cursor[1]))  {
        scanner_advance(s); // consume the `.`
        while (is_digit(*s->cursor)) {
            scanner_advance(s);
        }
    }
    scanner_add_token(s, TOKEN_NUMBER);
}

void scan_string(Scanner *s) {
    scan_to(s, '"');
    if (s->eof) {
        scanner_error(s, "Unterminated string.");
        return;
    }
    scanner_advance(s); // The closing double-quote

    // Trim the surrounding quotes.
    scanner_add_token_range(s, TOKEN_STRING, s->token_start+1, s->cursor-1);
}

void scan_token(Scanner *s) {
  s->token_start = s->cursor;
  char c = scanner_advance(s);

  switch (c) {
      case ' ': break;
      case '(': scanner_add_token(s, TOKEN_LEFT_PAREN); break;
      case ')': scanner_add_token(s, TOKEN_RIGHT_PAREN); break;
      case '{': scanner_add_token(s, TOKEN_LEFT_BRACE); break;
      case '}': scanner_add_token(s, TOKEN_RIGHT_BRACE); break;
      case ';': scanner_add_token(s, TOKEN_SEMICOLON); break;
      case ',': scanner_add_token(s, TOKEN_COMMA); break;
      case '.': scanner_add_token(s, TOKEN_DOT); break;
      case '-': scanner_add_token(s, TOKEN_MINUS); break;
      case '+': scanner_add_token(s, TOKEN_PLUS); break;
      case '*': scanner_add_token(s, TOKEN_STAR); break;
      case '!': scanner_add_token(s, scanner_match(s, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG); break;
      case '=': scanner_add_token(s, scanner_match(s, '=') ? TOKEN_EQUAL_EQUAL: TOKEN_EQUAL); break;
      case '<': scanner_add_token(s, scanner_match(s, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS); break;
      case '>': scanner_add_token(s, scanner_match(s, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER); break;
      case '"': scan_string(s); break;
      case '/': scanner_match(s, '/') ? scan_comment(s) : scanner_add_token(s, TOKEN_SLASH); break;
      case '\n': s->line++; break;
      case '\r': break;
      case '\t': break;
      case '\0': scanner_add_token(s, TOKEN_EOF); break;
      default:
                 if (is_digit(c)) {
                     return scan_number(s);
                 } else if (is_alpha(c)) {
                     return scan_identifier(s);
                 } else {
                     scanner_error(s, "Unexpected character.");
                 }
  }
  // fprintf(stdout, "scan: token:%.*s (line:%d)\n",
  //         (int)(s->cursor - s->token_start), s->token_start, s->line);
}

void scanner_scan(Scanner *s) {
    while (!s->eof) {
      scan_token(s);
    }
}

void token_print(const Token t) {
    printf("  %.*s (%s)\n", t.len, t.pos, TokenTypeNames[t.type]);
    // printf("\t%s\n", t.pos);
}

size_t fsize(FILE *stream) {
    fseek(stream, 0L, SEEK_END);
    long size = ftell(stream);
    rewind(stream);
    return size;
}

// Reads the contents of the file at [path] and returns it as a heap allocated
// string. Exits if it could not be found or read.
char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not find file \"%s\".\n", path);
        exit(ERR_FILE);
    }

    size_t len = fsize(file);
    char *buf = malloc(len + 1);
    if (!buf) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(ERR_FILE);
    }

    size_t bytes_read = fread(buf, sizeof(char), len, file);
    if (bytes_read < len) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(ERR_FILE);
    }
    buf[bytes_read] = '\0';

    fclose(file);
    return buf;
}

void eval(Scanner *s, const char *src) {
    scanner_init(s, src);
    scanner_scan(s);

    for (int i = 0; i < s->num_tokens; i++) {
        token_print(s->tokens[i]);
    }
}

void eval_file(Scanner *scanner, const char* path) {
    char *src = read_file(path);
    eval(scanner, src);
    free(src);

    if (had_error) exit(ERR_COMPILE);
}

#define REPL_MAX_LINE_LENGTH 1024
static char repl_buffer[REPL_MAX_LINE_LENGTH];

int repl_read_line() {
    if (fgets(repl_buffer, REPL_MAX_LINE_LENGTH, stdin)) {
        size_t len = strlen(repl_buffer);
        repl_buffer[len - 1] = '\0';
        return (int)len;
    }
    return 0;
}

void repl(Scanner *s) {
    for (;;) {
        fputs("loxy> ", stdout);
        if (!repl_read_line()) {
            fputc('\n', stdout);
            break;
        }

        eval(s, repl_buffer);
        had_error = false;
    }
}

int usage() {
    fputs("Usage: loxy [path]\n", stderr);
    return ERR_USAGE;
}

int main(int argc, const char *argv[]) {
    Scanner *s = malloc(sizeof(Scanner));
    switch (argc) {
        case 1: repl(s); break;
        case 2: eval_file(s, argv[1]); break;
        default: exit(usage());
    }
    free(s);
}
