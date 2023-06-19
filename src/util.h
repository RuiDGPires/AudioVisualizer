#ifndef __UTIL_H__
#define __UTIL_H__

#include "defs.h"
#include <stdio.h>

void file_read(void *, usize, FILE *);

i32 sign(i32);
i32 abs(i32);
i32 map(i32, i32, i32, i32, i32);

#endif
