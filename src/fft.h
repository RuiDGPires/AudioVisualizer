#ifndef __FFT_H__
#define __FFT_H__
#include "defs.h"

void fft_init(usize);
void fft(int32_t *buffer, int32_t *output);

#endif
