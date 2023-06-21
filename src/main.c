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
#include "pcolors.h"

//#define FREQ_GRAPH

#define WIDTH  1200
#define HEIGHT 800
#define FPS 20

#define MAX_FILENAME 100

#define USAGE "vis <output_name>"
#define DEFAULT_NAME "out.mp4"
#define CHUNK (2048 * 2)
#define OFFSET 250
#define SMOOTHING 0.3
#define NORMALIZE_PARAMETER 0.8
#define ENERGY_SCALE 10

DEC_VOID(canvas_destroy, clean_canvas)
DEC_VOID(wav_destroy, clean_wav)
DEC_VOID_DEREF(close, clean_fd, int)

void check_input_name(const char *name) {
    usize len = strlen(name);
    ERR_ASSERT(len > 4, "Invalid input name"); 
    ERR_ASSERT(strcmp(&name[len-4], ".wav") == 0, "Invalid input file type"); 
}

void normalize_fft(i32 *buffer, i32 *tmp, usize len, usize new_max) {
    for (usize i = 0; i < len; i++) {
        buffer[i] = new_max * NORMALIZE_PARAMETER *  buffer[i] / (double) CHUNK;
        buffer[i] = tmp[i] + (buffer[i] - tmp[i]) * SMOOTHING;
    }
}

// Checks filename, opens wav and registers it for cleaning
// NO NEED TO FREE (is freed on clean_exit) 
wav_t *open_wav(const char *filename) {
    check_input_name(filename);
    wav_t *wav = wav_from_file(filename);
    return wav;
}

i32 *process_wav(wav_t *wav) {
    wav_to_mono_left(wav);
    i32 *buffer = wav_to_32(wav);
    return buffer;
}

void end_msg(int status, const char *input_file, const char *output_file) {
    printf("\n\n");
    if (WIFEXITED(status)) {
        printf("************************\n");
        printf("%sOperation successful%s\n", PGREEN, PNC);
        printf("************************\n");
        printf("Used audio: \n\t'%s'\n", input_file);
        printf("Generated file:\n\t'%s'\n", output_file);
    } else {
        printf("************************\n");
        printf("%sOperation failed%s\n", PRED, PNC);
        printf("************************\n");
    }
}

int main(ARGS) {
    char *output_file = DEFAULT_NAME, input_file[MAX_FILENAME] = "\0";
    char *center_pic_file = NULL, *background_pic_file = NULL;

    BEGIN_PARSE_ARGS("%s", input_file)
        ARG_STRING(output_file, "-o")
        ARG_STRING(center_pic_file, "-c")
        ARG_STRING(background_pic_file, "-b")
    END_PARSE_ARGS

    wav_t *wav = open_wav(input_file);
    clean_register(&wav, clean_wav);

    i32 *buffer = process_wav(wav);
    double duration = wav_duration(wav);

    canvas_t *canvas_back, *canvas;
    clean_register(&canvas_back, clean_canvas);
    clean_register(&canvas, clean_canvas);

    if (background_pic_file) {
        canvas_back = canvas_from_img(background_pic_file);
    } else {
        canvas_back = canvas_create(WIDTH, HEIGHT);
        canvas_fill(canvas_back, COLOR_BLACK);
    }

    canvas_t *canvas_center = NULL, *canvas_center_backup = NULL;

    if (center_pic_file) {
        canvas_center = canvas_from_img(center_pic_file);
        canvas_center_backup = canvas_dup(canvas_center);

        clean_register(&canvas_center, clean_canvas);
        clean_register(&canvas_center_backup, clean_canvas);
    }
    
    canvas = canvas_dup(canvas_back);

    usize width = canvas->width, height = canvas->height;

    wav_normalize(wav, height/3);
    int outfd = open_ffmpeg(output_file, input_file, width, height, FPS);
    clean_register(&outfd, clean_fd);

    usize start = width / 10, end = width * 9/10;
    //double step = (double) (end - start) / (CHUNK * 0.5);
    usize d = (end - start);
    usize N = wav_n_samples(wav);
    double log_step = d / log10(CHUNK/2.0);
    
    // *** Prologue
    i32 fft_tmp1[CHUNK], fft_tmp2[CHUNK];
    fft_init(CHUNK);

    fft(buffer, fft_tmp2); // Fill second tmp buffer with initial fft
    normalize_fft(fft_tmp2, fft_tmp2, CHUNK, height/ 2.2);
    fft_lowpass(fft_tmp2, CHUNK/2, 0.04, 0);
    i32 *fft_tmp[2] = {fft_tmp1, fft_tmp2};
    // *** 

    i32 prev_energy = map(fft_energy(fft_tmp2, CHUNK/2), 0, 100, 0, height/200);
    double prev_scale = 1 - (1. / ( 5+ prev_energy));

    for (usize i = 0; i < FPS * duration; i++) {
        canvas_cpy(canvas, canvas_back);

        fft(&buffer[map(i, 0, FPS * duration, CHUNK*2, N - CHUNK)], fft_tmp[i % 2]);

        // Alternating pointers
        i32 *fft_out = &(fft_tmp[i % 2])[OFFSET]; // Output FFT
        i32 *fft_prev = &(fft_tmp[(i + 1) % 2])[OFFSET]; // Previous FFT (for smoothing)

        // *** Process FFT
        normalize_fft(fft_out, fft_prev, CHUNK/2 - OFFSET, height / 2.2);
        fft_lowpass(fft_out, CHUNK/2 - OFFSET, 0.04, 0);
        // *** 

#ifdef FREQ_GRAPH
        // *** Draw Frequency graph
        for (usize i = 0; i < CHUNK/2 - 1 - OFFSET; i ++) {
            double x1 = start + log10(i + 1) * log_step;
            double x2 = start + log10(i + 2) * log_step;
            
            point_t p1 = {.x = x1, .y = height*3/4 - fft_out[i]};
            point_t p2 = {.x = x2, .y = height*3/4 - fft_out[i+1]};

            canvas_draw_line(canvas, p1, p2, 1, COLOR_WHITE);
        }
        canvas_dump(canvas, outfd);
        // *** 
#else
        i32 energy = map(fft_energy(fft_out, CHUNK/2 - OFFSET), 0, 100, 0, height/200);

        energy = prev_energy + (energy - prev_energy) * 0.4;
        prev_energy = energy;

        double scale = 1.0 - (1.0 / (5 + energy ));
        scale = prev_scale + (scale - prev_scale) * 0.6;

        if (canvas_center) {
            canvas_center->width = canvas_center_backup->width;
            canvas_center->height = canvas_center_backup->height;
            canvas_cpy(canvas_center, canvas_center_backup);
            canvas_scale(canvas_center, scale);
            canvas_paste(canvas, canvas_center, MAKEPOINT(width/2 - canvas_center->width/2, height/2 - canvas_center->height/2));
        } else
            canvas_draw_circle_outline(canvas, MAKEPOINT(width/2, height/2), height/3 + energy, 4, COLOR_WHITE);
        canvas_dump(canvas, outfd);
#endif

    }

    close(outfd);
    int status;

    wait(&status);
    end_msg(status, input_file, output_file) ;

    clean_exit(0);
}
