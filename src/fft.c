#include "fft.h"
#include "../kiss_fft/kiss_fft.h"
#include "err.h"
#include "util.h"

static kiss_fft_cfg cfg;
static kiss_fft_cpx *fft_buff1, *fft_buff2;
static usize fft_size;

void clean_fft(void *) {
  if (fft_buff1 != NULL)
    free(fft_buff1);
  if (fft_buff2 != NULL)
    free(fft_buff2);
  if (cfg != NULL)
    free(cfg);

  fft_buff1 = NULL;
  fft_buff2 = NULL;
  cfg = NULL;
}

void fft_init(usize size) {
  fft_buff1 = malloc(sizeof(kiss_fft_cpx) * size);
  fft_buff2 = malloc(sizeof(kiss_fft_cpx) * size);
  cfg = kiss_fft_alloc(size, 1, NULL, NULL);


  clean_register(NULL, clean_fft);
  fft_size = size;
}

void fft(int32_t *buffer, int32_t *output) {
  for (usize i = 0; i < fft_size; i++) {
    fft_buff1[i].r = buffer[i];
    fft_buff1[i].i = 0;
  }

  kiss_fft(cfg, fft_buff1, fft_buff2);

  for (usize i = 0; i < fft_size; i++) {
    output[i] = sqrt(fft_buff2[i].r*fft_buff2[i].r + fft_buff2[i].i*fft_buff2[i].i);
  }
}

void lowpass(i32 *buffer, usize len, double db, usize freq) {
  usize d = (len - freq) * db;
  for (usize i = freq; i < len; i++) {
    double a = (double) (i - freq) / d;
    buffer[i] = a > 1.0 ? 0  : buffer[i] * (1 - a) * (1 - a);
  }
}
