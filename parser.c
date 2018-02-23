#define PARSER_C

#ifndef COMMON_H
#include "common.h"
#endif
#ifndef ERROR_C
#include "error.c"
#endif
#ifndef EXPR_C
#include "expr.c"
#endif
#ifndef TOKEN_C
#include "token.c"
#endif

typedef struct {
    Token *tokens;
    Token *cursor;
    bool eof;
} Parser;

void parser_pp(const Parser *p)
{
    printf("[Parser * %p:%s]\n", (void *) p, bool_str(p->eof));
    printf("  Token: "); token_pp(p->tokens);
    printf("  Cursor: "); token_pp(p->cursor);
}

void parser_error(const Parser *restrict p, const char *restrict message)
{
    Token t = *p->cursor;
    if (t.type == TOKEN_EOF) {
        // FIXME
        // int num_lines = 111;
        // error(num_lines, scanner_last_line_str(s),
        //         str_new_s(s->token, s->cursor - s->token - 1), message);
    } else {
        // FIXME
        // int num_lines = 111;
        // error(num_lines, scanner_last_line_str(s),
        //         str_new_s(s->token, s->cursor - s->token - 1), message);
    }
}

Token *parser_advance(Parser *p)
{
    Token *t = p->cursor++;
    p->eof = (p->cursor->type == TOKEN_EOF);
    return t;
}

bool check(const Parser *p, const TokenType t)
{
    return !p->eof && p->cursor->type == t;
}

Token *consume(Parser *restrict p, const TokenType t, const char *restrict message)
{
    if (check(p, t)) return parser_advance(p);
    parser_error(p, message);
    return &TokenNone;
}

bool match(Parser *p, int n, ...)
{
    va_list token_types;
    va_start(token_types, n);
    while (n--) {
        if (check(p, va_arg(token_types, TokenType))) {
            parser_advance(p);
            return true;
        }
    }
    va_end(token_types);
    return false;
}

 void synchronize(Parser *p) {
     parser_advance(p);

     while (!p->eof) {
         if (p->cursor[-1].type == TOKEN_SEMICOLON) {
             return;
         }

         switch (p->cursor->type) {
             case TOKEN_CLASS:
             case TOKEN_FN:
             case TOKEN_VAR:
             case TOKEN_FOR:
             case TOKEN_IF:
             case TOKEN_WHILE:
             case TOKEN_PRINT:
             case TOKEN_RETURN:
                 return;
             default:
                 break;
         }

         parser_advance(p);
     }
 }

Expr *expression(Parser *p);

Expr *primary(Parser *p)
{
    // printf("primary: \"%.*s\"\n", (p->cursor-1)->lexeme.len, (p->cursor-1)->lexeme.head);
    if (match(p, 1, TOKEN_NIL))    return make_nil_expr(p->cursor-1);
    if (match(p, 1, TOKEN_FALSE))  return make_bool_expr(p->cursor-1, false);
    if (match(p, 1, TOKEN_TRUE))   return make_bool_expr(p->cursor-1, true);
    if (match(p, 1, TOKEN_NUMBER)) return make_number_expr(p->cursor-1);
    if (match(p, 1, TOKEN_STRING)) return make_string_expr(p->cursor-1);

    if (match(p, 1, TOKEN_LEFT_PAREN)) {
        Expr *e = expression(p);
        consume(p, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        return make_grouping_expr(e);
    }

    parser_error(p, "Expect expression");
    return &NoneExpr;
}

Expr *unary(Parser *p)
{
    if (match(p, 3, TOKEN_BANG, TOKEN_PLUS, TOKEN_MINUS)) {
        Token *op = p->cursor-1;
        Expr *rhs = unary(p);
        return make_unary_expr(op, rhs);
    }
    return primary(p);
}

Expr *multiplication(Parser *p)
{
    Expr *e = unary(p);
    while (match(p, 2, TOKEN_SLASH, TOKEN_STAR)) {
        Token *op = p->cursor-1;
        Expr *rhs = unary(p);
        e = make_binary_expr(e, op, rhs);
    }
    return e;
}

Expr *addition(Parser *p)
{
    Expr *e = multiplication(p);
    while (match(p, 2, TOKEN_MINUS, TOKEN_PLUS)) {
        Token *op = p->cursor-1;
        Expr *rhs = multiplication(p);
        e = make_binary_expr(e, op, rhs);
    }
    return e;
}

Expr *comparison(Parser *p)
{
    Expr *e = addition(p);
    while (match(p, 4, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL)) {
        Token *op = p->cursor-1;
        Expr *rhs = addition(p);
        e = make_binary_expr(e, op, rhs);
    }
    return e;
}

Expr *equality(Parser *p)
{
    Expr *e = comparison(p);
    while (match(p, 2, TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL)) {
        Token *op = p->cursor-1;
        Expr *rhs = comparison(p);
        e = make_binary_expr(e, op, rhs);
    }
    return e;
}

Expr *expression(Parser *p)
{
    return equality(p);
}

Expr *parse(Parser *p, Token *tokens)
{
    if (tokens->type == TOKEN_NONE) {
        had_error = true;
        return NULL;
    }
    p->tokens = tokens;
    p->cursor = p->tokens;
    p->eof = false;
    exprs_count = 0;

    Expr *e = expression(p);
    if (e->type == EXPR_NONE) {
        return NULL;
    }
    return e;
}
