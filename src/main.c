#include "config.h"
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

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#define WIDTH 1200
#define HEIGHT 800
#define FPS 30

#define MAX_FILENAME 100

#define USAGE "vis <output_name>"
#define DEFAULT_NAME "out.mp4"
#define CHUNK (512 * 4)
#define OFFSET 1
#define SMOOTHING 0.4
#define NORMALIZE_PARAMETER 0.04
#define BASE_SCALE 0.6
#define DEFAULT_MAPPING_ENERGY 200.0

#ifndef DURATION
#define DURATION duration
#endif

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
    wav_to_mono(wav);
    i32 *buffer = wav_to_32(wav);

    wav_normalize(wav, 150);
    return buffer;
}

void fft_cpy(i32 *dst, i32*src, usize len) {
    for (usize i = 0; i < len; i++)
        dst[i] = src[i];
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

double prev_circle_energy1[CHUNK];

void clear_circle_energy(double *ce, i32 *fft) {
    for (usize i = 0; i < CHUNK; i++){
        ce[i] = mapf(fft[i], 0 , 1400, 100, 300 * 4);
    }
}

void draw_spectrum_circle(canvas_t *canvas, i32 *fft, color_t color1, color_t color2, double scale, double *tmp) {
    const double e_min = canvas->height/9.3, e_max = scale * canvas->height/3;
    const usize spec   = CHUNK * 0.20;
    const usize offset = CHUNK/40;
    const double softness = 0.2;

    u32 width = canvas->width, height = canvas->height;

    for (usize i = 0; i < spec;  i ++) {
        usize next = i + 1;

        double energy1 = mapf(fft[offset + i], 0 , 400, e_min, (i+1)*12 + e_max * 5);
        double energy2 = mapf(fft[offset + next], 0 , 400, e_min, (i+1)*12 + e_max * 5);

        energy1 = tmp[i] + (energy1 - tmp[i]) * softness;
        energy2 = tmp[next] + (energy2 - tmp[next]) * softness;
        tmp[i] = energy1;
        tmp[next] = energy2;

        double step = M_PI  / log10(spec);
        //double step = M_PI  / spec;
        double start = -0.5 * M_PI;

        double x1 = cos(start + log10(i + 1)    * step) * energy1;
        double x2 = cos(start + log10(next + 1) * step) * energy2; 

        double y1 = sin(start + log10(i + 1)    * step) * energy1;
        double y2 = sin(start + log10(next + 1) * step) * energy2;
        
        point_t p1, p2, p3, p4;

        p1 = (point_t){.x = width/2 + x1*1.05, .y = height/2 + y1*1.05};
        p2 = (point_t){.x = width/2 + x2*1.05, .y = height/2 + y2*1.05};

        p3 = (point_t){.x = width/2 - x1*1.05, .y = height/2 + y1*1.05};
        p4 = (point_t){.x = width/2 - x2*1.05, .y = height/2 + y2*1.05};

        canvas_draw_line(canvas, p1, p2, 7, color1 & RGBA(0xFF,0xFF,0xFF, 0x0F));
        canvas_draw_line(canvas, p3, p4, 7, color1 & RGBA(0xFF,0xFF,0xFF, 0x0F));

        p1 = (point_t){.x = width/2 + x1, .y = height/2 + y1};
        p2 = (point_t){.x = width/2 + x2, .y = height/2 + y2};
                      
        p3 = (point_t){.x = width/2 - x1, .y = height/2 + y1};
        p4 = (point_t){.x = width/2 - x2, .y = height/2 + y2};

        canvas_draw_line(canvas, p1, p2, 5, color1);
        canvas_draw_line(canvas, p3, p4, 5, color1);

    }

    for (usize i = 0; i < spec;  i ++) {
        usize next = i + 1;
        double energy1 = tmp[i];
        double energy2 = tmp[next];

        double step = M_PI  / log10(spec);
        //double step = M_PI  / spec;
        double start = -0.5 * M_PI;

        double x1 = cos(start + log10(i + 1)    * step) * energy1;
        double x2 = cos(start + log10(next + 1) * step) * energy2; 

        double y1 = sin(start + log10(i + 1)    * step) * energy1;
        double y2 = sin(start + log10(next + 1) * step) * energy2;
        
        point_t p1, p2, p3, p4;
        const usize iters = 50;
        const double iter_step = 1.0 / iters;
        for (usize i = 0; i < iters; i++) {
            p1 = (point_t){.x = width/2 + x1 * iter_step * i, .y = height/2 + y1 * iter_step * i};
            p2 = (point_t){.x = width/2 + x2 * iter_step * i, .y = height/2 + y2 * iter_step * i};
                          
            p3 = (point_t){.x = width/2 - x1 * iter_step * i, .y = height/2 + y1 * iter_step * i};
            p4 = (point_t){.x = width/2 - x2 * iter_step * i, .y = height/2 + y2 * iter_step * i};

            canvas_draw_line(canvas, p1, p2, 4, color2);
            canvas_draw_line(canvas, p3, p4, 4, color2);
        }
    }
}

int main(ARGS) {
    double max_energy = DEFAULT_MAPPING_ENERGY;

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

    canvas = canvas_dup(canvas_back);
    usize width = canvas->width, height = canvas->height;

    canvas_t *canvas_center = NULL, *canvas_center_backup = NULL;

    if (center_pic_file) {
        canvas_center = canvas_from_img(center_pic_file);
        u32 min_pic = canvas_center->width < canvas_center->height ? canvas_center->width : canvas_center->height;
        u32 min_canvas = width < height ? width : height;

        double max_radius = min_canvas * 0.5 * 0.35;

        // Limit center pic to a size
        if (min_pic * 0.5 > max_radius) {
            canvas_scale(canvas_center, (max_radius * 2) / min_pic );
        }

        canvas_cut_circle(canvas_center);
        canvas_center_backup = canvas_dup(canvas_center);

        clean_register(&canvas_center, clean_canvas);
        clean_register(&canvas_center_backup, clean_canvas);
    }
    


    int outfd = open_ffmpeg(output_file, input_file, width, height, FPS);
    clean_register(&outfd, clean_fd);

    usize start = width / 10, end = width * 9/10;
    //double step = (double) (end - start) / (CHUNK * 0.5);
    usize d = (end - start);
    usize N = wav_n_samples(wav);
    double log_step = d / log10(CHUNK/2.0);
    
    // *** Prologue
    i32 fft_tmp1[CHUNK], fft_tmp2[CHUNK], fft_back[CHUNK];
    fft_init(CHUNK);

    fft(buffer, fft_tmp2); // Fill second tmp buffer with initial fft
    normalize_fft(fft_tmp2, fft_tmp2, CHUNK, height/ 5);
    clear_circle_energy(prev_circle_energy1, fft_tmp2);

    fft_lowpass(fft_tmp2, CHUNK/2, 0.04, 1);
    i32 *fft_tmp[2] = {fft_tmp1, fft_tmp2};
    // *** 

    double prev_energy = 0;
    double circle_coords[CHUNK/2][2];
    double circle_step = M_PI / log10((double) CHUNK / 2);

    for (usize i = 0; i < CHUNK/2; i++) {
        circle_coords[i][0] = cos(M_PI/2 + log10(circle_step*i));
        circle_coords[i][1] = sin(-M_PI/2 + log10(circle_step*i));
    }

    for (usize i = 0; i < FPS * DURATION; i++) {

        fft(&buffer[map(i, 0, FPS * duration, CHUNK*2, N - CHUNK)], fft_tmp[i % 2]);

        // Alternating pointers
        i32 *fft_out = &(fft_tmp[i % 2])[OFFSET]; // Output FFT
        i32 *fft_prev = &(fft_tmp[(i + 1) % 2])[OFFSET]; // Previous FFT (for smoothing)

        //// *** Process FFT
        normalize_fft(fft_out, fft_prev, CHUNK - OFFSET, height / 2.2);

        canvas_cpy(canvas, canvas_back);
        draw_spectrum_circle(canvas, fft_out, COLOR_WHITE, RGBA(0x0A, 0xD0, 0x0A, 0xEE), 1, prev_circle_energy1);

        fft_lowpass(fft_out, CHUNK/2 - OFFSET, 0.0005, 2);

        // *** 
        
        double energy = fft_energy(fft_out, CHUNK/3);

        // If, for some reason, it exceeds the initial prediction of max energy, adjust accordingly
        if (energy > max_energy)
            max_energy = energy;

        energy = prev_energy + (energy - prev_energy) * 0.6;
        prev_energy = energy;

        double scale = BASE_SCALE + mapf(energy, 0, max_energy, 0, 1.0 - BASE_SCALE);

        if (canvas_center) {
            canvas_center->width = canvas_center_backup->width;
            canvas_center->height = canvas_center_backup->height;
            canvas_cpy(canvas_center, canvas_center_backup);
            canvas_scale(canvas_center, scale);
            canvas_paste(canvas, canvas_center, MAKEPOINT(width/2 - canvas_center->width/2, height/2 - canvas_center->height/2));
        } else
            canvas_draw_circle_outline(canvas, MAKEPOINT(width/2, height/2), (double)height/3 + energy, 4, COLOR_WHITE);

        canvas_dump(canvas, outfd);
    }

    close(outfd);
    int status;

    wait(&status);
    end_msg(status, input_file, output_file) ;

    clean_exit(0);
}
