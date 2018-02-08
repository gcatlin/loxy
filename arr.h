// #include <string.h> // memcpy
// #include <stdlib.h> // realloc

//
// Adapted from:
//   https://github.com/nothings/stb/blob/master/stretchy_buffer.h
//   Version 1.03
//
// Look for more ideas here:
//   https://github.com/golang/go/wiki/SliceTricks
//   https://github.com/golang/go/blob/master/src/strings/builder.go
//
// Ideas:
//   arr_popn(T *a, int n)  pop multiple items, return pointer to first dropped item (remove?)
//   arr_grow(T *a, int n)  instead of arr_reserve ?
//   arr_reset(T *a)        instead of arr_clear ?
//   arr_drop(T *a, int n)  remove n items from the beginning
//   arr_trunc(T *a, int n) instead of arr_drop?
//   arr_take(T *a, int n)  keep n items from the beginning, remove the rest
//   arr_resize(T *a, int len, int cap) // grow and set len and cap?
//
// arr_add(T *a, int n)          adds n uninitialized elements at end of array, returns pointer to first added element
// arr_concat(T *a, T *b, int n) appends a copy of b to the end of a, returns pointer to the first added element
// arr_copy(T *a, T *b, int n)   copies n elements from b into a
// arr_count(T *a)               returns the number of elements in the array
// arr_empty(T *a)               is the array empty?
// arr_end(T *a, int i)          is iterator at end of array?
// arr_free(T *a)                free the array
// arr_last(T *a)                returns an lvalue of the last item in the array
// arr_limit(T *a)               returns the number of elements allocated for the array
// arr_pop(T *a)                 removes the last element of the array and returns it
// arr_push(T *a, T v)           adds v on the end of the array
// arr_reserve(T *a, int n)      ensures at least n unintialized elements are allocated, returns pointer to first reserved element
// arr_reset(T *a)               set count to 0
// a[n]                          access the nth (counting from 0) element of the array
//
#define arr_add(a, n)       (_arr_maybe_grow(a,n), _arr_cnt(a)+=(n), &(a)[_arr_cnt(a)-(n)])
#define arr_concat(a, b, n) (_arr_maybe_grow(a,n), _arr_cnt(a)+=(n), memcpy(&(a)[_arr_cnt(a)-(n)], (b), (n)))
#define arr_copy(a, b, n)   (_arr_maybe_grow(a,n), _arr_cnt(a)=(n), memcpy(&(a)[0], (b), (n)))
#define arr_count(a)        ((a) ? _arr_cnt(a) : 0)
#define arr_empty(a)        (!(a) || (_arr_cnt(a) == 0))
// #define arr_end(a, i)       ((i) >= &(a)[_arr_cnt(a)])
#define arr_free(a)         ((a) ? free(_arr_raw(a)), 0 : 0)
#define arr_last(a)         ((a)[_arr_cnt(a)-1])
#define arr_limit(a)        ((a) ? _arr_lim(a) : 0)
#define arr_pop(a)          ((a)[--_arr_cnt(a)])
#define arr_push(a, v)      (_arr_maybe_grow(a,1), (a)[_arr_cnt(a)++]=(v))
#define arr_reserve(a, n)   (_arr_maybe_grow(a,n), (a)[_arr_cnt(a)])
#define arr_reset(a)        ((a) ? _arr_cnt(a)=0 : 0)
#define arr_pp(a)           (printf("[arr %p:%d:%d] \"%.*s\"\n",(void*)(a),arr_count(a),arr_limit(a),arr_count(a),(a)))

#define _arr_raw(a) ((int *)(a)-2)
#define _arr_lim(a) _arr_raw(a)[0]
#define _arr_cnt(a) _arr_raw(a)[1]

#define _arr_need_grow(a, n)  ((a)==0 || _arr_cnt(a)+(n) >= _arr_lim(a))
#define _arr_maybe_grow(a, n) (_arr_need_grow(a,n) ? _arr_grow(a,n) : 0)
#define _arr_grow(a, n)       (*((void **)&(a)) = _arr_growf((a), (n), sizeof(*(a))))

static void *_arr_growf(void *arr, int increment, int item_size)
{
    int dbl_lim = arr ? 2 * _arr_lim(arr) : 0;
    int min_lim = arr_count(arr) + increment;
    int new_lim = dbl_lim > min_lim ? dbl_lim : min_lim;
    int *p = (int *) realloc(arr ? _arr_raw(arr) : 0, item_size * new_lim + 2 * sizeof(int));
    if (p) {
        if (!arr) { p[1] = 0; }
        p[0] = new_lim;
        return p+2;
    } else {
        return (void *) (2 * sizeof(int)); // try to force a NULL pointer exception later
    }
}
