#include "canvas.h"
#include "err.h"

#define NUMBER_WIDTH 3
#define NUMBER_HEIGHT 5

u8 n_matrixes[10][NUMBER_HEIGHT][NUMBER_WIDTH] =  {

   {{ 0, 1, 0},
    { 1, 0, 1},
    { 1, 0, 1},
    { 1, 0, 1},
    { 0, 1, 0}},

    {{ 0, 0, 1},
     { 0, 1, 1},
     { 1, 0, 1},
     { 0, 0, 1},
     { 0, 0, 1}},

    {{ 1, 1, 0},
     { 0, 0, 1},
     { 0, 1, 0},
     { 1, 0, 0},
     { 1, 1, 1}},

    {{ 1, 1, 0},
     { 0, 0, 1},
     { 0, 1, 1},
     { 0, 0, 1},
     { 1, 1, 0}},

    {{ 1, 0, 1},
     { 1, 0, 1},
     { 1, 1, 1},
     { 0, 0, 1},
     { 0, 0, 1}},

    {{ 1, 1, 1},
     { 1, 0, 0},
     { 1, 1, 1},
     { 0, 0, 1},
     { 1, 1, 0}},

    {{ 1, 1, 1},
     { 1, 0, 0},
     { 1, 1, 1},
     { 1, 0, 1},
     { 1, 1, 1}},

    {{ 1, 1, 1},
     { 0, 0, 1},
     { 0, 1, 0},
     { 0, 1, 0},
     { 0, 1, 0}},

    {{ 1, 1, 1},
     { 1, 0, 1},
     { 0, 1, 0},
     { 1, 0, 1},
     { 1, 1, 1}},

   {{ 1, 1, 1},
    { 1, 0, 1},
    { 1, 1, 1},
    { 0, 0, 1},
    { 0, 1, 0}}
};

u32 get_int_len(i32 n) {
    u32 size = 0;

    do {
        n /= 10;
        size++;
    }
    while(n != 0);

    return size;
}

const u32 size = 8;
const double space = 0.75;
const u32 pad = (NUMBER_WIDTH + space) * size;

void canvas_draw_integer(canvas_t *canvas, point_t p, i32 n, color_t color){
    u32 len = get_int_len(n);
    u32 offset = (len - 1)*pad;

    do {
        u8 c = n % 10;
        n /= 10;

        for (u32 i = 0; i < NUMBER_HEIGHT*size; i++)
            for (u32 j = 0; j < NUMBER_WIDTH*size; j++) 
                if (n_matrixes[c][i/size][j/size])
                    canvas_draw_point(canvas, point_sum(p, MAKEPOINT(j + offset, i)), color);
        offset -= pad;
    } while (n != 0);
}

void canvas_draw_integer_center(canvas_t *canvas, point_t p, i32 n, color_t color){
    u32 len = get_int_len(n);
    p.x -= (len * pad) / 2;
    canvas_draw_integer(canvas, p, n, color);
}
