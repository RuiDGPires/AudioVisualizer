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

#define WIDTH  600 
#define HEIGHT 400
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

    for (usize i = 100; i < 1000; i++) {
        printf("%d\n", wav_get_val32(wav, i)); 
    }

    color_t pixels[WIDTH*HEIGHT];
    canvas_t *canvas = canvas_from_buffer(pixels, WIDTH, HEIGHT);
    clean_register(&canvas, clean_canvas);

    int outfd = open_ffmpeg(output_file, WIDTH, HEIGHT, FPS);
    clean_register(&outfd, clean_fd);
     
    for (usize i = 0; i < FPS * DURATION; i++) {
        canvas_fill(canvas, COLOR_BLUE);
        canvas_draw_circle(canvas, (point_t){.x = WIDTH/2, .y = i}, HEIGHT/6, COLOR_RED);

        canvas_dump(canvas, outfd);
    }
    close(outfd);
    wait(NULL);
    printf("Operation completed\n");
    clean_exit(0);
}
