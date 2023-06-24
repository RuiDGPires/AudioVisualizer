#ifndef __FFT_H__
#define __FFT_H__
#include "defs.h"

void fft_init(usize);
void fft(i32 *buffer, i32 *output);

void fft_lowpass(i32 *, usize, double, usize);
double fft_energy(i32 *, usize);
void fft_soften(i32 *, usize, double);

#endif
