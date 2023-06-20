#ifndef __FFT_H__
#define __FFT_H__
#include "defs.h"

void fft_init(usize);
void fft(i32 *buffer, i32 *output);

void lowpass(i32 *, usize, double, usize);

#endif
