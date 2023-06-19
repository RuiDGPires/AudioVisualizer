#include "err.h"

static struct _cleanup _cleanup_buffer[MAX_CLEANUPS];
static int _cleanup_n = 0;

void clean_register(void *var, void (*f)(void*)) {
    _cleanup_buffer[_cleanup_n++] = (struct _cleanup){.var = var, .f = f};
}

void clean_exit(int err_n) {
    for (int i = 0; i < _cleanup_n; i++) {
        struct _cleanup c = _cleanup_buffer[i];
        c.f(c.var);
    }

    exit(err_n);
}

