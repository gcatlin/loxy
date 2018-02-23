#define EXPR_C

#ifndef COMMON_H
#include "common.h"
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

#define EXPRS_MAX_COUNT 65536
static Expr exprs[EXPRS_MAX_COUNT];
static int exprs_count;

static Expr NoneExpr  = { .type = EXPR_NONE };
static Expr NilExpr   = { .type = EXPR_NIL };
static Expr FalseExpr = { .type = EXPR_BOOL, .literal.boolean = false };
static Expr TrueExpr  = { .type = EXPR_BOOL, .literal.boolean = true };

static const str expr_nil_s   = (str) { .head = "nil",   .len = 3};
static const str expr_true_s  = (str) { .head = "true",  .len = 4};
static const str expr_false_s = (str) { .head = "false", .len = 5};
static const str expr_group_s = (str) { .head = "group", .len = 5};

const char *expr_type_string(const Expr *e)
{
    return ExprTypeNames[e->type];
}

char *expr_parenthesize(char *buf, const str lexeme, int n, ...);
char *expr_sprint(char *restrict str, const Expr *restrict e);

const char *expr_string(const Expr *e)
{
    static char *buf = NULL;
    arr_reset(buf); // FIXME rather than reset the buffer, need to continue appending
    switch (e->type) {
        case EXPR_NONE: return "";
        case EXPR_NIL: return "nil";
        case EXPR_BOOL: return (e->literal.boolean ? "true" : "false");
        case EXPR_NUMBER: return str_to_char(e->literal.token->lexeme);
        case EXPR_STRING: return e->literal.string;
        // case EXPR_UNARY: arr_concat(buf, e->unary.op->lexeme.head, e->unary.op->lexeme.len); return buf;// // FIXME ???
        // case EXPR_UNARY: return expr_parenthesize(buf, e->unary.op->lexeme, 1, e->unary.rhs);
        case EXPR_UNARY: return "unary"; // FIXME ???
        case EXPR_BINARY: return "binary"; // FIXME ???
        case EXPR_GROUPING: return "group"; // FIXME ???
    }
}

void expr_pp(const Expr *e)
{
    printf("[Expr * %p:%s] \"%s\"\n", (void *) e, expr_type_string(e), expr_string(e));
}

char *expr_parenthesize(char *buf, const str lexeme, int n, ...)
{
    va_list exprs;
    va_start(exprs, n);
    arr_reserve(buf, 2+n); // (, ), ' ' per expr
    arr_push(buf, '(');
    arr_concat(buf, lexeme.head, lexeme.len);
    Expr *e;
    while (n--) {
        arr_push(buf, ' ');
        e = va_arg(exprs, Expr *);
        buf = expr_sprint(buf, e);
    }
    arr_push(buf, ')');

    va_end(exprs);
    return buf;
}

char *expr_sprint_str(char *buf, const str s)
{
    arr_concat(buf, s.head, s.len);
    return buf;
}

char *expr_sprint(char *buf, const Expr *e)
{
    switch (e->type) {
        case EXPR_GROUPING: return expr_parenthesize(buf, expr_group_s, 1, e->grouping);
        case EXPR_BINARY: return expr_parenthesize(buf, e->binary.op->lexeme, 2, e->binary.lhs, e->binary.rhs);
        case EXPR_UNARY: return expr_parenthesize(buf, e->unary.op->lexeme, 1, e->unary.rhs);
        case EXPR_STRING: return expr_sprint_str(buf, e->literal.token->lexeme);
        case EXPR_NUMBER: return expr_sprint_str(buf, e->literal.token->lexeme);
        case EXPR_BOOL: return expr_sprint_str(buf, e->literal.boolean ? expr_true_s : expr_false_s);
        case EXPR_NIL: return expr_sprint_str(buf, expr_nil_s);
        case EXPR_NONE: return buf;
    }
    // TODO assert unreachable
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
    arr_copy(buf, t->lexeme.head, t->lexeme.len); arr_push(buf, '\0');
    Expr *e = make_literal_expr(EXPR_NUMBER, t);
    e->literal.number = atof(buf);
    return e;
}

Expr *make_string_expr(Token *t)
{
    Expr *e = make_literal_expr(EXPR_STRING, t);
    e->literal.string = str_to_char(t->lexeme);
    return e;
}

Expr *make_unary_expr(Token *restrict op, Expr *restrict rhs)
{
    Expr *e = make_expr(EXPR_UNARY);
    e->unary.op = op;
    e->unary.rhs = rhs;
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
