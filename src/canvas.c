#include <stdlib.h>
#include <unistd.h>
#include <math.h>
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

// Copies the contents of a canvas to another (roughly equivalent to canvas_paste(dest, src, (0, 0)))
// Key difference: 
//  *sets* the colors, doesnt draw them
inline void canvas_cpy(canvas_t *dest, canvas_t *src) {
    for (u32 w = 0; w < src->width; w++) {
        for (u32 h = 0; h < src->height; h++) {
            point_t p2 = MAKEPOINT(w, h);
            canvas_set_point(dest, p2, canvas_get_point(src, p2));
        }
    }
}

canvas_t *canvas_create(u32 width, u32 height) {
    u32 *buffer = (u32*) malloc(sizeof(u32) * width * height); 
    canvas_t *canvas = canvas_from_buffer(buffer, width, height);
    canvas->is_static = FALSE;

    canvas_fill(canvas, RGBA(0, 0, 0, 0));
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

// Ignores alpha
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

color_t blend_pixel(color_t back, color_t fore) {
    // Perform alpha blending calculation
    float alpha = GET_ALPHA(fore) / 255.0;
    float invAlpha = 1.0 - alpha;

    u8 red = (uint8_t)(GET_RED(fore) * alpha + GET_RED(back) * invAlpha);
    u8 green = (uint8_t)(GET_GREEN(fore) * alpha + GET_GREEN(back) * invAlpha);
    u8 blue = (uint8_t)(GET_BLUE(fore) * alpha + GET_BLUE(back) * invAlpha);
    u8 res_alpha = (uint8_t)(GET_ALPHA(fore) * alpha + GET_ALPHA(back) * invAlpha);

    return RGBA(red, green, blue, res_alpha);
}

void canvas_set_point(canvas_t *canvas, point_t p, color_t color) {
    ERR_ASSERT(p.x >= 0 && p.x < canvas->width && p.y >= 0 && p.y < canvas->height, "Invalid coordinates: (%lu, %lu) on canvas (%u, %u)", p.x, p.y, canvas->width, canvas->height);

    canvas->buffer[p.x + p.y*canvas->width] = color;
}

// Blends the pixels
void canvas_draw_point(canvas_t *canvas, point_t p, color_t color) {
     canvas_set_point(canvas, p, blend_pixel(canvas_get_point(canvas, p), color));
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

void canvas_cut_circle(canvas_t *canvas) {
    const double thickness = 10;
    u32 radius = (canvas->width < canvas->height ? canvas->width : canvas->height) * 0.5;

    point_t center = MAKEPOINT(canvas->width/2, canvas->height/2);

    for (u32 y = 0; y < canvas->height; y++){
        for (u32 x = 0; x < canvas->width; x++){
            point_t p = (point_t) {.x = x, .y = y};
            u32 dist = point_dist_sqrd(p, center);
            if (dist >= (radius*radius))
                canvas->buffer[x + y*canvas->width] = RGBA(0xFF, 0xFF, 0xFF, 0xFF);
            if (dist >= (radius*radius + thickness*thickness) )
                canvas->buffer[x + y*canvas->width] = RGBA(0, 0, 0, 0);
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

color_t bilinear_interpolation(color_t tl, color_t tr, color_t bl, color_t br, double dx, double dy) {

    double w1 = (1 - dx) * (1 - dy);
    double w2 = dx * (1 - dy);
    double w3 = (1 - dx) * dy;
    double w4 = dx * dy;

    u8 result_r = (u8)(GET_RED(tl) * w1 + GET_RED(tr) * w2 + GET_RED(bl) * w3 + GET_RED(br) * w4);
    u8 result_g = (u8)(GET_GREEN(tl) * w1 + GET_GREEN(tr) * w2 + GET_GREEN(bl) * w3 + GET_GREEN(br) * w4);
    u8 result_b = (u8)(GET_BLUE(tl) * w1 + GET_BLUE(tr) * w2 + GET_BLUE(bl) * w3 + GET_BLUE(br) * w4);
    u8 result_a = (u8)(GET_ALPHA(tl) * w1 + GET_ALPHA(tr) * w2 + GET_ALPHA(bl) * w3 + GET_ALPHA(br) * w4);

    return RGBA(result_r, result_g, result_b, result_a);
}

void canvas_scale(canvas_t *canvas, double s) {
    //ERR_ASSERT(!canvas->is_static, "Cannot scale a static canvas");
    ERR_ASSERT(s <= 1, "Cannot upscale");

    u32 dst_w = canvas->width  * s;
    u32 dst_h = canvas->height * s;

    //color_t *dst = malloc(sizeof(color_t) * dst_w * dst_h);
    
    double s2 = 1 / s;

    for (usize y = 0; y < dst_h; y++) {
        for (usize x = 0; x < dst_w; x++) {
            double src_x = x * s2;
            double src_y = y * s2;
            int x1 = (int)src_x;
            int y1 = (int)src_y;
            int x2 = ceil(src_x);
            int y2 = ceil(src_y);

            if (x1 >= canvas->width || y1 >= canvas->height || x2 >= canvas->width || y2 >= canvas->height)
                continue;

            double dx = src_x - x1;
            double dy = src_y - y1;

            color_t tl = canvas_get_point(canvas, MAKEPOINT(x1, y1));
            color_t tr = canvas_get_point(canvas, MAKEPOINT(x2, y1));
            color_t bl = canvas_get_point(canvas, MAKEPOINT(x1, y2));
            color_t br = canvas_get_point(canvas, MAKEPOINT(x2, y2));

            color_t result = bilinear_interpolation(tl, tr, bl, br, dx, dy);

            canvas->buffer[y * dst_w + x] = result;
        }
    }

    canvas->width = dst_w;
    canvas->height = dst_h;
}
