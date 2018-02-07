#define ERROR_C

#ifndef COMMON_H
#include "common.h"
#endif

#define ERR_USAGE    64
#define ERR_COMPILE  65
#define ERR_RUNTIME  70
#define ERR_FILE     74

#define ANSI_RESET     "\x1b[0m"
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_FG_RED    "\x1b[31m"
#define ANSI_FG_GREEN  "\x1b[32m"
#define ANSI_FG_YELLOW "\x1b[33m"
#define ANSI_FG_BLUE   "\x1b[34m"
#define ANSI_FG_CYAN   "\x1b[36m"

#define FILENAME_STYLE  ANSI_RESET
#define LINE_STYLE      ANSI_RESET
#define UNDERLINE_STYLE ANSI_RESET ANSI_FG_GREEN // maybe INFO_LINE_STYLE???
#define LINE_NUM_STYLE  ANSI_RESET ANSI_FG_BLUE
#define COL_NUM_STYLE   ANSI_RESET ANSI_FG_CYAN
#define MESSAGE_STYLE   ANSI_RESET ANSI_BOLD
#define INFO_STYLE      ANSI_RESET ANSI_FG_GREEN ANSI_BOLD
#define ERROR_STYLE     ANSI_RESET ANSI_FG_RED ANSI_BOLD

static bool had_error;

int digits(unsigned int v) {
    return (v < 10) ? 1 : (v < 100) ? 2 : (v < 1000) ? 3 : (v < 10000) ? 4 :
        (v < 100000) ? 5 : (v < 1000000) ? 6 : (v < 10000000) ? 7 :
        (v < 100000000) ? 8 : 9;
}

//
// https://blog.rust-lang.org/2016/08/10/Shape-of-errors-to-come.html
//
// error[E0499]: cannot borrow `foo.bar1` as mutable more than once at a time
//   --> src/test/compile-fail/borrowck/borrowck-borrow-for-owned-ptr.rs:29:22
//    |
// 28 |      let bar1 = &mut foo.bar1;
//    |                       -------- first mutable borrow occurs here
// 29 |      let _bar2 = &mut foo.bar1;
//    |                        ^^^^^^^^ second mutable borrow occurs here
// 30 |      *bar1;
// 31 |  }
//    |  - first borrow ends here
//
void report(const int line, const int col, const char *restrict where, const char *restrict message) {
    // fprintf(stderr, "error [%d:%d] %s\n", (where ? " " : ""), where, line, col, message);
    fprintf(stderr, ANSI_BOLD ANSI_FG_RED "error[E0000]" ANSI_RESET
            ANSI_BOLD ": %s" ANSI_RESET "\n", message);
    // fprintf(stderr, ANSI_FG_BLUE  "  --> " ANSI_RESET "%s:%d:%d\n", "unknown", line, col);
    fprintf(stderr, ANSI_FG_BLUE " %d | "  ANSI_RESET "%s\n", line, "source goes here");
    fprintf(stderr, ANSI_FG_BLUE  "   | "  ANSI_RESET "%s\n", "^");
    // fprintf(stderr, ANSI_FG_BLUE "   | "  ANSI_RESET "\n");
    // (where ? " " : ""), where, line, col, message);

    had_error = true;
}

// TODO larger buffer and fill using `memset`
static const char* carets = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
static const char* dashes = "------------------------------------------------------------";

void info(const int line_num, const str line, const str substr, const str message) {
    const int substr_offset = substr.head - line.head;
    const str before_substr = str_slice(line, 0, substr_offset);
    const str after_substr = str_slice(line, substr_offset + substr.len, line.len);
    const int col = substr_offset + 1;
    const int padding = digits(line_num);

    // Message
    fprintf(stderr, INFO_STYLE "info");
    fprintf(stderr, MESSAGE_STYLE": %.*s\n", message.len, message.head);
    fprintf(stderr, LINE_NUM_STYLE " %*s--> ", padding, "");
    fprintf(stderr, FILENAME_STYLE "%s" LINE_NUM_STYLE ":%d" COL_NUM_STYLE ":%d\n",
            "unknown", line_num, col);
    fprintf(stderr, LINE_NUM_STYLE " %*s | \n", padding, "");
    fprintf(stderr, LINE_NUM_STYLE " %d | ", line_num);

    // Code
    fprintf(stderr, LINE_STYLE "%.*s", before_substr.len, before_substr.head);
    fprintf(stderr, INFO_STYLE "%.*s", substr.len, substr.head);
    fprintf(stderr, LINE_STYLE "%.*s\n", after_substr.len, after_substr.head);

    // Annotation
    fprintf(stderr, LINE_NUM_STYLE " %*s | ", padding, "");
    fprintf(stderr, UNDERLINE_STYLE "%*s%.*s", substr_offset, "", substr.len, carets);
    fprintf(stderr, INFO_STYLE " %*s" ANSI_RESET "\n\n", message.len, message.head);
}

void error(const int line, const int col, const char *message) {
    report(line, col, "Error", message);
}
