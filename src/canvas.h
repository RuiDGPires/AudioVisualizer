#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "defs.h"

typedef u32 color_t;

#define RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))

#define GET_RED(v)   ((v >> (8*0)) & 0xFF)
#define GET_GREEN(v) ((v >> (8*1)) & 0xFF)
#define GET_BLUE(v)  ((v >> (8*2)) & 0xFF)
#define GET_ALPHA(v) ((v >> (8*3)) & 0xFF)

#define COLOR_RED   RGBA(0xFF, 0x00, 0x00, 0xFF)
#define COLOR_GREEN RGBA(0x00, 0xFF, 0x00, 0xFF)
#define COLOR_BLUE  RGBA(0x00, 0x00, 0xFF, 0xFF)
#define COLOR_BLACK RGBA(0x00, 0x00, 0x00, 0xFF)
#define COLOR_WHITE RGBA(0xFF, 0xFF, 0xFF, 0xFF)
#define MAKEPOINT(xx,yy) ((point_t){.x = (xx), .y = (yy)})

typedef struct {
    u32 width, height;
    color_t *buffer;
    u8 is_static;
} canvas_t;

typedef struct {
    usize x, y;
} point_t;

u32 point_dist_sqrd(point_t, point_t);
point_t point_sum(point_t, point_t);

canvas_t *canvas_create(u32, u32);
canvas_t *canvas_dup(canvas_t *); // Allocates a new canvas
void canvas_cpy(canvas_t *, canvas_t *); // Copies the contents of a canvas to another (equivalent to canvas_paste(dest, src, (0, 0)))
canvas_t *canvas_from_buffer(u32 *, u32, u32);
void canvas_destroy(canvas_t **);

void canvas_fill(canvas_t *, color_t); 
void canvas_dump(canvas_t *, int); 
color_t canvas_get_point(canvas_t *, point_t); 
void canvas_draw_point(canvas_t *, point_t, color_t); 
void canvas_set_point(canvas_t *, point_t, color_t); 
void canvas_draw_circle(canvas_t *, point_t, u32, color_t);
void canvas_cut_circle(canvas_t *);
void canvas_draw_circle_outline(canvas_t *, point_t, u32, u32, color_t);
void canvas_draw_line(canvas_t *, point_t, point_t, u32, color_t);
void canvas_paste(canvas_t *, canvas_t *, point_t);

//canvas_t *canvas_from_png(const char *);
//canvas_t *canvas_from_jpeg(const char *);
canvas_t *canvas_from_img(const char *);
void canvas_scale(canvas_t *canvas, double s);
void canvas_draw_integer(canvas_t *canvas, point_t, i32 n, color_t color);
void canvas_draw_integer_center(canvas_t *canvas, point_t, i32 n, color_t color);

#endif
