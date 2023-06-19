#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "defs.h"

typedef u32 color_t;

#define RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))

#define COLOR_RED   RGBA(0xFF, 0x00, 0x00, 0xFF)
#define COLOR_GREEN RGBA(0x00, 0xFF, 0x00, 0xFF)
#define COLOR_BLUE  RGBA(0x00, 0x00, 0xFF, 0xFF)
#define COLOR_BLACK RGBA(0x00, 0x00, 0x00, 0xFF)
#define COLOR_WHITE RGBA(0xFF, 0xFF, 0xFF, 0xFF)

typedef struct {
    u32 width, height;
    color_t *buffer;
    u8 is_static;
} canvas_t;

typedef struct {
    usize x, y;
} point_t;

u32 point_dist_sqrd(point_t, point_t);

canvas_t *canvas_create(u32, u32);
canvas_t *canvas_from_buffer(u32 *, u32, u32);
void canvas_destroy(canvas_t **);

void canvas_fill(canvas_t *, color_t); 
void canvas_dump(canvas_t *, int); 
void canvas_draw_point(canvas_t *, point_t, color_t); 
void canvas_draw_circle(canvas_t *, point_t, u32, color_t);
void canvas_draw_line(canvas_t *, point_t, point_t, u32, color_t);

#endif
