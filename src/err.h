#ifndef __ERR_H__
#define __ERR_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct _cleanup {
    void (*f)(void*);
    void *var;
};

#define MAX_CLEANUPS 50
static struct _cleanup _cleanup_buffer[MAX_CLEANUPS];
static int _cleanup_n = 0;

#define ERR(...) {fprintf(stderr, "ERR: " __VA_ARGS__); fprintf(stderr, "\n"); clean_exit(1);}
#define ERR_ASSERT(cond, ...) {if (!(cond)) ERR(__VA_ARGS__)}
#define ERR_ASSERT_CLN(cond, cln, ...) {if (!(cond)) {{cln;} ERR(__VA_ARGS__)}}

#define DEC_VOID(f, new_name) void new_name(void*v) {if (v != NULL) f(v);} 
#define DEC_VOID_DEREF(f, new_name, type) void new_name(void*v) {if (v != NULL) f(*((type *) v));} 

static void clean_register(void *var, void (*f)(void*)) {
    _cleanup_buffer[_cleanup_n++] = (struct _cleanup){.var = var, .f = f};
}

static void clean_exit(int err_n) {
    for (int i = 0; i < _cleanup_n; i++) {
        struct _cleanup c = _cleanup_buffer[i];
        c.f(c.var);
    }

    exit(err_n);
}


#endif
