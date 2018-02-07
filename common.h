#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdarg.h> // va_*
#include <stddef.h> // ptrdiff_t
#include <stdlib.h> // exit
#include <stdio.h>  // printf, fprintf
#include <string.h> // memcmp, memcpy, memcpy_s, strlen

#include "arr.h"
#include "str.h"

// typedef struct {
//     char *head;
//     int cap;
//     int len;
// } mem;

typedef struct {
    const char *restrict name;
    char *head;    // beginning of buffer
    char **lines;  // array of beginning-of-lines
    int len;       // length of null-terminated string
    int cap;       // capacity of allocated region
    int num_lines; // do not modify directly
} Buffer;

void buffer_init(Buffer *b, int cap, int num_lines) {
    b->len = 0;
    b->cap = cap;
    b->head = malloc(sizeof(char) * cap);
    b->lines = malloc(sizeof(char *) * num_lines);
    b->lines[0] = b->head;
    b->num_lines = 1;
}

static int buffer_add_line(Buffer *restrict b, char *restrict line) {
    // TODO ensure b->num_lines does not overflow?
    // TODO update b->len (scan for newline or NUL?) ???
    b->lines[b->num_lines++] = line;
    return b->num_lines;
}

static char *buffer_last_line(Buffer *b) {
    return b->lines[b->num_lines-1];
}

// typedef struct {
//     const Source *src;
//     // const char *start; // is this better than offset?
//     int line; // only needed for logging
//     int col;  // only needed for logging
//     // const char *start; // is this better than offset?
//     int offset; // do we really need this??
// } Loc; // SourceLocation?
//
// Loc new_loc(*Source src) {
//
// }

size_t fsize(FILE *stream) {
    fseek(stream, 0L, SEEK_END);
    long size = ftell(stream);
    rewind(stream);
    return size;
}

#endif
