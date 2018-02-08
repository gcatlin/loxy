//
// Immutable, not necessarily null-terminated strings
//
typedef struct {
    const char *head;
    const int len;
} str;

static str str_new(const char *s) {
    return (str) { .head = s, .len = strlen(s) };
}

static str str_new_s(const char *s, const int len) {
    return (str) { .head = s, .len = len };
}

static int str_ncmp(const str lhs, const str rhs, const int count) {
    // TODO bounds check `count`
    return strncmp(lhs.head, rhs.head, count);
}

static str str_slice(const str s, const int from, const int to) {
    // TODO bounds check `from` and `to`
    return (str) { .head = s.head+from, .len = to-from };
}

static char *str_to_char(const str s) {
    // FIXME @leak this will leak memory!!!!
    static char *buf = {0};
    buf = arr_concat(buf, s.head, s.len);
    arr_push(buf, '\0');
    return buf;
}

void str_pp(const str s) {
    printf("[str %p:%d] \"%.*s\"\n", (void *) s.head, s.len, s.len, s.head);
}
// struct {
//     char **head;
// } string
//
// string string_new(const char s*) {
//     return (string) {
//
// }

//
//
// Adapt arr.h for strings.
//  - handle null-terminator
//    - e.g. str_concat(a, "abc") does something like: strcpy(arr_add(a, strlen(s)), s)
//  - maybe combine with `struct str` and have `head` be a pointer to a pointer?
//  - other stuff?
//
// #define arr_add(a, n)     (_arr_maybe_grow(a,n), _arr_len(a)+=(n), &(a)[_arr_len(a)-(n)])
// #define arr_append(a, v)  (_arr_maybe_grow((a),1), (a)[_arr_len(a)++]=(v))
// #define arr_capacity(a)   ((a) ? _arr_cap(a) : 0)
// #define arr_count(a)      ((a) ? _arr_len(a) : 0)
// #define arr_free(a)       ((a) ? free(_arr_raw(a)), 0 : 0)
// #define arr_last(a)       ((a)[_arr_len(a)-1])
// #define arr_reserve(a, n) (_arr_grow((a,n), _arr_cap(a)+=(n), &(a)[_arr_cap(a)-(n)])

// #define _arr_raw(a) ((int *)(a)-2)
// #define _arr_cap(a) _arr_raw(a)[0]
// #define _arr_len(a) _arr_raw(a)[1]

// #define _arr_need_grow(a, n)  ((a)==0 || _arr_len(a)+(n) >= _arr_cap(a))
// #define _arr_maybe_grow(a, n) (_arr_need_grow(a,(n)) ? _arr_grow(a,n) : 0)
// #define _arr_grow(a, n)       (*((void **)&(a)) = _arr_growf((a), (n), sizeof(*(a))))

// static void *_arr_growf(void *arr, int increment, int item_size)
// {
//    int dbl_cap = arr ? 2 * _arr_cap(arr) : 0;
//    int min_cap = arr_count(arr) + increment;
//    int new_cap = dbl_cap > min_cap ? dbl_cap : min_cap;
//    int *p = (int *) realloc(arr ? _arr_raw(arr) : 0, item_size * new_cap + 2 * sizeof(int));
//    if (p) {
//       if (!arr) { p[1] = 0; }
//       p[0] = new_cap;
//       return p+2;
//    } else {
//       return (void *) (2 * sizeof(int)); // try to force a NULL pointer exception later
//    }
// }
