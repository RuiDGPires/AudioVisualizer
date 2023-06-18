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

int main(ARGS) {
    char *output_file = DEFAULT_NAME;

    BEGIN_PARSE_ARGS("")
        ARG_STRING(output_file, "-o")
    END_PARSE_ARGS

    color_t pixels[WIDTH*HEIGHT];
    canvas_t *canvas = canvas_from_buffer(pixels, WIDTH, HEIGHT);

    int outfd = open_ffmpeg(output_file, WIDTH, HEIGHT, FPS);
     
    canvas_fill(canvas, COLOR_BLUE);
    canvas_draw_circle(canvas, (point_t){.x = WIDTH/2, .y = HEIGHT/2}, HEIGHT/3, COLOR_RED);

    for (usize i = 0; i < FPS * DURATION; i++) {
        canvas_dump(canvas, outfd);
    }

    close(outfd);
    canvas_destroy(&canvas);
    wait(NULL);
    printf("Operation completed\n");
    return 0;
}
