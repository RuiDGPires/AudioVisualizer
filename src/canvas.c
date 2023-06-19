#include <stdlib.h>
#include <unistd.h>
#include "canvas.h"
#include "err.h"

u32 point_dist_sqrd(point_t p1, point_t p2) {
    i32 dx = (i32) (p1.x - p2.x), dy = (i32) (p1.y - p2.y);
    return dx*dx + dy*dy;
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

void canvas_draw_point(canvas_t *canvas, point_t point, color_t color) {
    canvas->buffer[point.x + point.y*canvas->width] = color;
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
