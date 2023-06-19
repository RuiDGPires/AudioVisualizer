#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../CCLArgs/cclargs.h"
#include "ffmpeg.h"
#include "defs.h"
#include "err.h"
#include "wav.h"
#include "canvas.h"
#include "fft.h"

#define WIDTH  1200
#define HEIGHT 800
#define FPS 30
#define DURATION 5

#define MAX_FILENAME 100

#define USAGE "vis <output_name>"
#define DEFAULT_NAME "a.mp4"

DEC_VOID(canvas_destroy, clean_canvas)
DEC_VOID(wav_destroy, clean_wav)
DEC_VOID_DEREF(close, clean_fd, int)

void check_input_name(const char *name) {
    usize len = strlen(name);
    ERR_ASSERT(len > 4, "Invalid input name"); 
    ERR_ASSERT(strcmp(&name[len-4], ".wav") == 0, "Invalid input file type"); 
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
     
    canvas_fill(canvas, COLOR_BLUE);

    usize start = WIDTH / 10, end = WIDTH * 9/10;
    double step = (double) (end - start) / wav_n_samples(wav);

    for (usize i = 0; i < wav_n_samples(wav); i ++) {
        point_t p1 = {.x = start + i * step,       .y = HEIGHT/2 + buffer[i]};
        point_t p2 = {.x = start + (i + 1) * step,  .y = HEIGHT/2 + buffer[i+1]};
        canvas_draw_line(canvas, p1, p2, 1, COLOR_RED);
    }


    for (usize i = 0; i < FPS * DURATION; i++) {
        canvas_dump(canvas, outfd);
    }

    close(outfd);
    wait(NULL);
    printf("Operation completed\n");
    clean_exit(0);
}
