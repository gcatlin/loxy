#ifndef COMMON_H
#include "common.h"
#endif
#ifndef ERROR_C
#include "error.c"
#endif
#ifndef TOKEN_C
#include "token.c"
#endif
#ifndef PARSER_C
#include "parser.c"
#endif
#ifndef SCANNER_C
#include "scanner.c"
#endif

#define BUFFER_MAX_LEN 65536
#define BUFFER_MAX_LINES 4096
#define MAX_TOKENS 1024

int read_file(char *restrict buf, const size_t n, const char *restrict path)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not find file \"%s\".\n", path);
        exit(ERR_FILE);
    }

    size_t sz = fsize(file);
    size_t count = (n <= sz ? n - 1 : sz);
    size_t bytes_read = fread(buf, sizeof(char), count, file);
    if (bytes_read < count) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        fclose(file);
        exit(ERR_FILE);
    }
    buf[bytes_read] = '\0';

    fclose(file);
    return (int)bytes_read;
}

void print(Expr *e)
{
    static char *buf = NULL;
    buf = expr_sprint(buf, e);
    printf("%.*s\n", arr_count(buf), buf);
}

Expr *eval(Buffer *restrict b, Scanner *restrict s, Parser *restrict p)
{
    scanner_init(s, b);
    scan(s);
    return parse(p);
}

void eval_file(Buffer *restrict b, Scanner *restrict s, Parser *restrict p, const char *restrict path)
{
    // TODO use arr instead?
    b->name = path;
    int n = read_file(b->head, BUFFER_MAX_LEN, path);
    b->len = n;

    print(eval(b, s, p));

    if (had_error) {
        exit(ERR_COMPILE);
    }
}

void repl(Buffer *restrict b, Scanner *restrict s, Parser *restrict p)
{
    b->name = "repl";
    for (;;) {
        fputs(ANSI_BOLD "loxy> " ANSI_RESET, stdout);
        if (!fgets(b->head, BUFFER_MAX_LEN, stdin)) {
            puts("");
            break;
        }
        if (b->head[0] != '\n') {
            b->num_lines = 1;
            eval(b, s, p);
        }
        had_error = false;
    }
}

int main(int argc, const char *argv[])
{
    Buffer b;
    buffer_init(&b, BUFFER_MAX_LEN, BUFFER_MAX_LINES);

    Token *tokens = malloc(sizeof(Token) * MAX_TOKENS);

    Scanner *s = malloc(sizeof(Scanner));
    s->tokens = tokens;

    Parser *p = malloc(sizeof(Parser));
    p->eof = false;
    p->cursor = p->tokens = s->tokens;
    // p->buffer = &b;

    switch (argc) {
        case 1: repl(&b, s, p); break;
        case 2: eval_file(&b, s, p, argv[1]); break;
        default: fputs("Usage: loxy [path]\n", stderr); return ERR_USAGE;
    }
}
