#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include "fft.h"

void _fft(double complex *buffer, usize size) {
    // Check if size is a power of 2
    if (size <= 1 || (size & (size - 1)) != 0) {
        printf("Error: Size of the buffer must be a power of 2.\n");
        return;
    }
    
    // Bit-reversal permutation
    int i, j, k;
    for (i = 0, j = 0; i < size; i++) {
        if (j > i) {
            double complex temp = buffer[j];
            buffer[j] = buffer[i];
            buffer[i] = temp;
        }
        k = size >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    
    // Cooley-Tukey decimation-in-time radix-2 FFT
    double complex w, wp, temp;
    int m, mh, i_step;
    for (m = 2; m <= size; m <<= 1) {
        mh = m >> 1;
        wp = cexp(-I * (2.0 * M_PI / m));
        w = 1.0 + 0.0 * I;
        for (i = 0; i < mh; i++) {
            for (k = i; k < size; k += m) {
                i_step = k + mh;
                temp = w * buffer[i_step];
                buffer[i_step] = buffer[k] - temp;
                buffer[k] += temp;
            }
            w *= wp;
        }
    }
}

void fft(int32_t *buffer, double complex *output, int size) {
    // Check if size is a power of 2
    if (size <= 1 || (size & (size - 1)) != 0) {
        printf("Error: Size of the buffer must be a power of 2.\n");
        return;
    }

    // Convert the integer samples to complex values
    for (int i = 0; i < size; i++) {
        output[i] = CMPLX(buffer[i], 0);
    }

    // Perform the FFT
    _fft(output, size);
}
