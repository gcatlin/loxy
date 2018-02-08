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
    arr_reset(buf);
    buf = expr_sprint(buf, e);
    printf("%.*s\n", arr_count(buf), buf);
}

Expr *eval(Buffer *restrict b, Scanner *restrict s, Parser *restrict p, Token *restrict ts)
{
    return parse(p, scan(s, b, ts));
}

void eval_file(Buffer *restrict b, Scanner *restrict s, Parser *restrict p,
        Token *restrict ts, const char *restrict path)
{
    // TODO use arr instead?
    // b->name = path;
    b->len = read_file(b->head, BUFFER_MAX_LEN, path);

    print(eval(b, s, p, ts));

    if (had_error) {
        exit(ERR_COMPILE);
    }
}

void repl(Buffer *restrict b, Scanner *restrict s, Parser *restrict p, Token *restrict ts)
{
    b->name = "repl";
    for (;;) {
        fputs(ANSI_BOLD "loxy> " ANSI_RESET, stdout);
        if (!fgets(b->head, BUFFER_MAX_LEN, stdin)) {
            fputs(ANSI_RESET "\n", stdout);
            break;
        }
        if (b->head[0] != '\n') {
            print(eval(b, s, p, ts));
        }
        buffer_reset(b);
        had_error = false;
    }
}

int main(int argc, const char *argv[])
{
    Buffer b;
    buffer_init(&b, BUFFER_MAX_LEN, BUFFER_MAX_LINES);
    Scanner *s = malloc(sizeof(Scanner));
    Parser *p = malloc(sizeof(Parser));
    Token *ts = malloc(sizeof(Token) * MAX_TOKENS);

    switch (argc) {
        case 1: repl(&b, s, p, ts); break;
        case 2: eval_file(&b, s, p, ts, argv[1]); break;
        default: fputs("Usage: loxy [path]\n", stderr); return ERR_USAGE;
    }
}
