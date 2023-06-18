#ifndef __ERR_H__
#define __ERR_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define ERR(...) {fprintf(stderr, "ERR: " __VA_ARGS__); fprintf(stderr, "\n"); exit(1);}

#define ERR_ASSERT(cond, ...) {if (!(cond)) ERR(__VA_ARGS__)}
#define ERR_ASSERT_CLN(cond, cln, ...) {if (!(cond)) {{cln;} ERR(__VA_ARGS__)}}

#endif
