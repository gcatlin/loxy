#define PARSER_C

#ifndef COMMON_H
#include "common.h"
#endif
#ifndef ERROR_C
#include "error.c"
#endif
#ifndef TOKEN_C
#include "token.c"
#endif

typedef enum {
    EXPR_NONE,
    EXPR_NIL,
    EXPR_BOOL,
    EXPR_NUMBER,
    EXPR_STRING,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_GROUPING
} ExprType;

static const char *ExprTypeNames[] = {
    "EXPR_NONE",
    "EXPR_NIL",
    "EXPR_BOOL",
    "EXPR_NUMBER",
    "EXPR_STRING",
    "EXPR_UNARY",
    "EXPR_BINARY",
    "EXPR_GROUPING"
};

typedef struct Expr Expr;
struct Expr {
    ExprType type;
    union {
        struct {
            Token *token;
            union {
                bool boolean;
                double number;
                char *string;
            };
        } literal;
        struct {
            Token *op;
            Expr *rhs;
        } unary;
        struct {
            Token *op;
            Expr *lhs;
            Expr *rhs;
        } binary;
        Expr *grouping;
    };
};

typedef struct {
    // Buffer *buffer;
    Token *tokens; // FIXME rename to token
    Token *cursor; // FIXME replace with tokens
    bool eof;
} Parser;

#define EXPRS_MAX_COUNT 65536
static Expr exprs[EXPRS_MAX_COUNT];
static int exprs_count;

static Expr NoneExpr  = { .type = EXPR_NONE };
static Expr NilExpr   = { .type = EXPR_NIL };
static Expr FalseExpr = { .type = EXPR_BOOL, .literal.boolean = false };
static Expr TrueExpr  = { .type = EXPR_BOOL, .literal.boolean = true };

const char *expr_type_string(const Expr *e)
{
    return ExprTypeNames[e->type];
}

const char *expr_string(const Expr *e)
{
    switch (e->type) {
        case EXPR_NONE: return "";
        case EXPR_NIL: return "nil";
        case EXPR_BOOL: return (e->literal.boolean ? "true" : "false");
        case EXPR_NUMBER: return str_to_char(e->literal.token->name);
        case EXPR_STRING: return e->literal.string;
        case EXPR_UNARY: return "unary"; // FIXME ???
        case EXPR_BINARY: return "binary"; // FIXME ???
        case EXPR_GROUPING: return "group"; // FIXME ???
    }
}

void expr_println(const Expr *e)
{
    printf("[Expr %p:%s] %s\n", (void *) e, expr_type_string(e), expr_string(e));
}

char *expr_sprint(char *restrict str, const Expr *restrict e);

char *parenthesize(char *buf, const str name, int n, ...)
{
    va_list exprs;
    va_start(exprs, n);
    arr_push(buf, '(');
    arr_concat(buf, name.head, name.len);
    while (n--) {
        arr_push(buf, ' ');
        buf = expr_sprint(buf, va_arg(exprs, Expr *));
    }
    arr_push(buf, ')');

    va_end(exprs);
    return buf;
}

static const str expr_nil_s   = (str) { .head = "nil",   .len = 3};
static const str expr_true_s  = (str) { .head = "true",  .len = 4};
static const str expr_false_s = (str) { .head = "false", .len = 5};
static const str expr_group_s = (str) { .head = "group", .len = 5};

char *expr_sprint_str(char *buf, const str s)
{
    arr_concat(buf, s.head, s.len);
    return buf;
}

char *expr_sprint(char *buf, const Expr *e)
{
    switch (e->type) {
        case EXPR_GROUPING: return parenthesize(buf, expr_group_s, 1, e->grouping);
        case EXPR_BINARY: return parenthesize(buf, e->binary.op->name, 2, e->binary.lhs, e->binary.rhs);
        case EXPR_UNARY: return parenthesize(buf, e->binary.op->name, 1, e->binary.rhs);
        case EXPR_STRING: return expr_sprint_str(buf, e->literal.token->name);
        case EXPR_NUMBER: return expr_sprint_str(buf, e->literal.token->name);
        case EXPR_BOOL: return expr_sprint_str(buf, e->literal.boolean ? expr_true_s : expr_false_s);
        case EXPR_NIL: return expr_sprint_str(buf, expr_nil_s);
        case EXPR_NONE: break;
    }
    return buf;
}

Expr *make_expr(const ExprType t)
{
    Expr *e = &exprs[exprs_count++];
    e->type = t;
    return e;
}

Expr *make_literal_expr(const ExprType et, Token *tok)
{
    Expr *e = make_expr(et);
    e->literal.token = tok;
    return e;
}

Expr *make_nil_expr(Token *t)
{
    return make_literal_expr(EXPR_NIL, t);
}

Expr *make_bool_expr(Token *t, bool b)
{
    Expr *e = make_literal_expr(EXPR_BOOL, t);
    e->literal.boolean = b;
    return e;
}

Expr *make_number_expr(Token *t)
{
    static char *buf = NULL;
    arr_copy(buf, t->name.head, t->name.len); arr_push(buf, '\0');
    Expr *e = make_literal_expr(EXPR_NUMBER, t);
    e->literal.number = atof(buf);
    return e;
}

Expr *make_string_expr(Token *t)
{
    Expr *e = make_literal_expr(EXPR_STRING, t);
    e->literal.string = str_to_char(t->name);
    return e;
}

Expr *make_unary_expr(Token *restrict op, Expr *restrict expr)
{
    Expr *e = make_expr(EXPR_UNARY);
    e->unary.op = op;
    e->unary.rhs = expr;
    return e;
}

Expr *make_binary_expr(Expr *restrict lhs, Token *restrict op, Expr *restrict rhs)
{
    Expr *e = make_expr(EXPR_BINARY);
    e->binary.lhs = lhs;
    e->binary.op = op;
    e->binary.rhs = rhs;
    return e;
}

Expr *make_grouping_expr(Expr *expr)
{
    Expr *e = make_expr(EXPR_GROUPING);
    e->grouping = expr;
    return e;
}

void parser_error(const Parser *restrict p, const char *restrict message)
{
    Token t = *p->cursor;
    if (t.type == TOKEN_EOF) {
        // FIXME
        // report(t.line, t.col, "at end", message);
    } else {
        // FIXME
        // report(t.line, t.col, "at '?'", message);
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

// void synchronize(Parser *p) {
//     parser_advance(p);
//
//     while (!p->eof) {
//         if (p->cursor->type == TOKEN_SEMICOLON) {
//             return;
//         }
//
//         switch (parser_peek(p)->type) {
//             case TOKEN_CLASS:
//             case TOKEN_FN:
//             case TOKEN_VAR:
//             case TOKEN_FOR:
//             case TOKEN_IF:
//             case TOKEN_WHILE:
//             case TOKEN_PRINT:
//             case TOKEN_RETURN:
//                 return;
//             default:
//                 break;
//         }
//
//         parser_advance(p);
//     }
// }

Expr *expression(Parser *p);

Expr *primary(Parser *p)
{
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
    if (match(p, 2, TOKEN_BANG, TOKEN_MINUS)) {
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

Expr *parse(Parser *p)
{
    return expression(p);
}
