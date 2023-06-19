#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include "../CCLArgs/cclargs.h"
#include "ffmpeg.h"
#include "defs.h"
#include "err.h"
#include "wav.h"
#include "canvas.h"
#include "fft.h"
#include "util.h"

#define WIDTH  1200
#define HEIGHT 800
#define FPS 30
#define DURATION 5

#define MAX_FILENAME 100

#define USAGE "vis <output_name>"
#define DEFAULT_NAME "a.mp4"
#define CHUNK 1024 * 2

DEC_VOID(canvas_destroy, clean_canvas)
DEC_VOID(wav_destroy, clean_wav)
DEC_VOID_DEREF(close, clean_fd, int)

void check_input_name(const char *name) {
    usize len = strlen(name);
    ERR_ASSERT(len > 4, "Invalid input name"); 
    ERR_ASSERT(strcmp(&name[len-4], ".wav") == 0, "Invalid input file type"); 
}

void normalize_i32(i32 *buffer, usize len, usize new_max) {
    i32 max = 0;

    for (usize i = 0; i < len; i++) {
        i32 val = buffer[i];
        if (abs(val) > max) 
            max = val;
    }

    for (usize i = 0; i < len; i++) {
        i32 val = buffer[i];
        i32 s = sign(val);
        buffer[i] = s * map(abs(val), 0, max, 0, new_max);
    }
}

int main(ARGS) {
    char *output_file = DEFAULT_NAME, input_file[MAX_FILENAME] = "\0";

    BEGIN_PARSE_ARGS("%s", input_file)
        ARG_STRING(output_file, "-o")
    END_PARSE_ARGS

    check_input_name(input_file);
    wav_t *wav = wav_from_file(input_file);
    clean_register(&wav, clean_wav);

    wav_to_mono(wav);
    i32 *buffer = wav_to_32(wav);
    wav_normalize(wav, HEIGHT/3);

    color_t pixels[WIDTH*HEIGHT];
    canvas_t *canvas = canvas_from_buffer(pixels, WIDTH, HEIGHT);
    clean_register(&canvas, clean_canvas);

    int outfd = open_ffmpeg(output_file, WIDTH, HEIGHT, FPS);
    clean_register(&outfd, clean_fd);
     
    canvas_fill(canvas, COLOR_BLACK);

    usize start = WIDTH / 10, end = WIDTH * 9/10;
    double step = (double) (end - start) / wav_n_samples(wav);

    usize d = (end - start);
    double log_step = d / log10(CHUNK);

    i32 fft_out[CHUNK];
    fft(buffer, fft_out, CHUNK);

    normalize_i32(fft_out, CHUNK, HEIGHT / 3);

    for (usize i = 0; i < CHUNK - 1; i ++) {
        double x1 = start + log10(i + 1) * log_step;
        double x2 = start + log10(i + 2) * log_step;

        point_t p1 = {.x = x1, .y = HEIGHT/2 + fft_out[i]};
        point_t p2 = {.x = x2, .y = HEIGHT/2 + fft_out[i+1]};

        canvas_draw_line(canvas, p1, p2, 1, COLOR_WHITE);
    }

    for (usize i = 0; i < FPS * DURATION; i++) {
        canvas_dump(canvas, outfd);
    }

    close(outfd);
    wait(NULL);
    printf("Operation completed\n");
    clean_exit(0);
}
