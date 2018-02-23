#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdarg.h> // va_*
#include <stddef.h> // ptrdiff_t
#include <stdlib.h> // exit
#include <stdio.h>  // printf, fprintf
#include <string.h> // memcmp, memcpy, memcpy_s, strlen

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#include "arr.h"
#include "str.h"

const char *bool_str(bool b)
{
    return b ? "true" : "false";
}

char *unescaped(const char *s)
{
    // TODO Use strpbrk and memcpy instead?
    static char *buf = NULL;
    arr_reset(buf);
    char c;
    while ((c = *(s++))) {
        switch (c) {
            case '\a': arr_push(buf, '\\'); arr_push(buf, 'a');  break;
            case '\b': arr_push(buf, '\\'); arr_push(buf, 'b');  break;
            case '\t': arr_push(buf, '\\'); arr_push(buf, 't');  break;
            case '\n': arr_push(buf, '\\'); arr_push(buf, 'n');  break;
            case '\v': arr_push(buf, '\\'); arr_push(buf, 'v');  break;
            case '\f': arr_push(buf, '\\'); arr_push(buf, 'f');  break;
            case '\r': arr_push(buf, '\\'); arr_push(buf, 'r');  break;
            case '\\': arr_push(buf, '\\'); arr_push(buf, '\\'); break;
            case '\"': arr_push(buf, '\\'); arr_push(buf, '\"'); break;
            default: arr_push(buf, c);
        }
    }
    arr_push(buf, '\0');
    return buf;
}

// typedef struct {
//     char *head;
//     int cap;
//     int len;
// } mem; // buf? span?

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

// TODO store line index elsewhere
typedef struct {
    const char *restrict name;
    char *head;    // beginning of buffer
    char **lines;  // array of beginning-of-lines
    int len;       // length of null-terminated string
    int cap;       // capacity of allocated region
    int num_lines; // do not modify directly
} Buffer;

// typedef struct {
//     int num_lines;
//     str lines[];
// } LineIndex;

void buffer_reset(Buffer *b)
{
    b->len = 0;
    b->num_lines = 1;
    b->lines[0] = b->head;
}

void buffer_init(Buffer *b, int cap, int num_lines)
{
    b->head = malloc(sizeof(char) * cap);
    b->lines = malloc(sizeof(char *) * num_lines);
    b->cap = cap;
    buffer_reset(b);
}

static int buffer_add_line(Buffer *restrict b, char *restrict line)
{
    // TODO ensure b->num_lines does not overflow?
    // TODO update b->len (scan for newline or NUL?) ???
    b->lines[b->num_lines++] = line;
    return b->num_lines;
}

static int buffer_find_line(Buffer *restrict b, const char *restrict c)
{
    // TODO bounds check `c`
    int mid;
    int low = 0;
    int high = b->num_lines - 1;
    while (low <= high) {
        mid = (low + high) / 2;
        if (c < b->lines[mid]) high = mid - 1;
        else if (c > b->lines[mid]) low = mid + 1;
        else return mid;
    }
    return mid;
}

static str buffer_get_line(Buffer *b, int line_index)
{
    // TODO bounds check
    char *line = b->lines[line_index];
    // does not include tailing \n or \0
    char *end = strchr(line, '\n');
    return str_new_s(line, end - line);
}

static str buffer_last_line(Buffer *b)
{
    return buffer_get_line(b, b->num_lines-1);
}

// static str buffer_last_line_str(Buffer *b)
// {
//     return b->lines[b->num_lines-1];
// }

size_t fsize(FILE *stream)
{
    fseek(stream, 0L, SEEK_END);
    long size = ftell(stream);
    rewind(stream);
    return size;
}

#endif
