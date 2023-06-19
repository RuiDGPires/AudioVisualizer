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

#define ERR(...) {fprintf(stderr, "ERR: " __VA_ARGS__); fprintf(stderr, "\n"); clean_exit(1);}
#define ERR_ASSERT(cond, ...) {if (!(cond)) ERR(__VA_ARGS__)}
#define ERR_ASSERT_CLN(cond, cln, ...) {if (!(cond)) {{cln;} ERR(__VA_ARGS__)}}

#define DEC_VOID(f, new_name) void new_name(void*v) {if (v != NULL) f(v);} 
#define DEC_VOID_DEREF(f, new_name, type) void new_name(void*v) {if (v != NULL) f(*((type *) v));} 

void clean_register(void *var, void (*f)(void*));
void clean_exit(int err_n);

#endif
