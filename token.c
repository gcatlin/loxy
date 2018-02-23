#define TOKEN_C

#ifndef COMMON_H
#include "common.h"
#endif

typedef enum {
    TOKEN_NONE,

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

    TOKEN_COMMENT,
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

static const char *token_type_names[] = {
    "TOKEN_NONE",

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
    str lexeme;
    // const Loc loc;
    // const char *pos; // delete?
    // int len; // delete?
    // int line; // can be calculated based on src and str
    // int col;
} Token;

typedef struct {
    TokenType type;
    const str name;
} Keyword;

static Token TokenNone = (Token) {0};

#define KEYWORD_MIN_LEN 2
#define KEYWORD_MAX_LEN 6
#define NUM_KEYWORDS 18
static Keyword keywords[NUM_KEYWORDS] = {
    {TOKEN_NONE,   {0}},
    {TOKEN_AND,    (str) {"and",    3}},
    {TOKEN_CLASS,  (str) {"class",  5}},
    {TOKEN_ELSE,   (str) {"else",   4}},
    {TOKEN_FALSE,  (str) {"false",  5}},
    {TOKEN_FOR,    (str) {"for",    3}},
    {TOKEN_FN,     (str) {"fn",     2}},
    {TOKEN_IF,     (str) {"if",     2}},
    {TOKEN_NIL,    (str) {"nil",    3}},
    {TOKEN_OR,     (str) {"or",     2}},
    {TOKEN_PRINT,  (str) {"print",  5}},
    {TOKEN_RETURN, (str) {"return", 6}},
    {TOKEN_SUPER,  (str) {"super" , 5}},
    {TOKEN_THIS,   (str) {"this",   4}},
    {TOKEN_TRUE,   (str) {"true",   4}},
    {TOKEN_VAR,    (str) {"var",    3}},
    {TOKEN_WHILE,  (str) {"while",  5}},
    {TOKEN_EOF,    {0}}
};

const char *token_type_name(const Token *t) {
    return token_type_names[t->type];
}

void token_pp(const Token *t) {
    printf("[Token %p:%s] \"%.*s\"\n", (void *) t, token_type_name(t),
            t->lexeme.len, t->lexeme.head);
}

Keyword *find_keyword(const str name) {
    int len = name.len;
    if (KEYWORD_MIN_LEN <= len && len <= KEYWORD_MAX_LEN) {
        Keyword k;
        for (int i = 1; i < NUM_KEYWORDS; ++i) {
            k = keywords[i];
            if (len == k.name.len && str_ncmp(name, k.name, len) == 0) {
                return &keywords[i];
            }
        }
    }
    return NULL;
}
