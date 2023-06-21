#include <stdlib.h>
#include <unistd.h>
#include "canvas.h"
#include "err.h"

u32 point_dist_sqrd(point_t p1, point_t p2) {
    i32 dx = (i32) (p1.x - p2.x), dy = (i32) (p1.y - p2.y);
    return dx*dx + dy*dy;
}

point_t point_sum(point_t p1, point_t p2) {
    return (point_t) {.x = p1.x + p2.x, .y = p1.y + p2.y};
}

canvas_t *canvas_dup(canvas_t *canvas) {
    canvas_t *new_canvas = canvas_create(canvas->width, canvas->height); 

    for (usize j = 0; j < canvas->height; j++) {
        for (usize i = 0; i < canvas->width; i++) {
            point_t p = MAKEPOINT(i, j);
            canvas_draw_point(new_canvas, p, canvas_get_point(canvas, p));
        }
    }

    return new_canvas;
}

// Copies the contents of a canvas to another (equivalent to canvas_paste(dest, src, (0, 0)))
inline void canvas_cpy(canvas_t *dst, canvas_t *src) {
    canvas_paste(dst, src, MAKEPOINT(0,0));
}

canvas_t *canvas_create(u32 width, u32 height) {
    u32 *buffer = (u32*) malloc(sizeof(u32) * width * height); 
    canvas_t *canvas = canvas_from_buffer(buffer, width, height);
    canvas->is_static = FALSE;
    return canvas;
}

canvas_t *canvas_from_buffer(u32 *buffer, u32 width, u32 height) {
    canvas_t *canvas = (canvas_t *) malloc(sizeof(canvas_t));
    canvas->width    = width;
    canvas->height   = height;
    canvas->buffer   = buffer;
    canvas->is_static = TRUE;
    return canvas;
}

void canvas_destroy(canvas_t **canvas) {
    if ((*canvas)->is_static == FALSE)
        free((*canvas)->buffer);

    free(*canvas);
    *canvas = NULL;
}

void canvas_fill(canvas_t *canvas, color_t color) {
    for (usize i = 0; i < canvas->width * canvas->height; i++) {
        canvas->buffer[i] = color;
    }
}

void canvas_dump(canvas_t *canvas, int fd) {
    write(fd, canvas->buffer, sizeof(color_t)*canvas->width*canvas->height);
}

color_t canvas_get_point(canvas_t *canvas, point_t point) {
    return canvas->buffer[point.x + point.y*canvas->width];
}

void canvas_draw_point(canvas_t *canvas, point_t p, color_t color) {
    ERR_ASSERT(p.x >= 0 && p.x < canvas->width && p.y >= 0 && p.y < canvas->height, "Invalid coordinates");
    canvas->buffer[p.x + p.y*canvas->width] = color;
}

void canvas_draw_line(canvas_t *canvas, point_t p1, point_t p2, u32 thickness, color_t color) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    float xIncrement = (float)dx / (float)steps;
    float yIncrement = (float)dy / (float)steps;
    float x = p1.x;
    float y = p1.y;

    for (int i = 0; i <= steps; i++) {
        canvas_draw_circle(canvas, (point_t){.x = x, .y = y}, thickness, color);
        x += xIncrement;
        y += yIncrement;
    }
}

void canvas_draw_circle(canvas_t *canvas, point_t center, u32 radius, color_t color) {
    for (u32 x = center.x - radius; x < center.x + radius; x++){
        for (u32 y = center.y - radius; y < center.y + radius; y++){
            point_t p = (point_t) {.x = x, .y = y};
            if (p.x >= 0 && p.x < canvas->width && p.y >= 0 && p.y < canvas->height && point_dist_sqrd(p, center) <= radius*radius) 
                canvas_draw_point(canvas, p, color);
        }
    }
}

void canvas_draw_circle_outline(canvas_t *canvas, point_t center, u32 radius, u32 border_size, color_t color) {
    u32 dist_min = radius - border_size;
    dist_min *= dist_min;
    for (u32 x = center.x - radius; x < center.x + radius; x++){
        for (u32 y = center.y - radius; y < center.y + radius; y++){
            point_t p = (point_t) {.x = x, .y = y};
            u32 dist = point_dist_sqrd(p, center);
            if (p.x >= 0 && p.x < canvas->width && p.y >= 0 && p.y < canvas->height && dist <= radius*radius && dist >= dist_min) 
                canvas_draw_point(canvas, p, color);
        }
    }
}

/*
 * Pastes the src canvas on top of the dest canvas
 * p is the coordinate on dest of the top right corner of for the src canvas
 */
void canvas_paste(canvas_t *dest, canvas_t *src, point_t p) {
    for (u32 w = 0; w < src->width; w++) {
        for (u32 h = 0; h < src->height; h++) {
            point_t p2 = MAKEPOINT(w, h);
            canvas_draw_point(dest, point_sum(p, p2), canvas_get_point(src, p2));
        }
    }
}
