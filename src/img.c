#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>
#include "canvas.h"
#include "err.h"

canvas_t *canvas_from_png_rgba(png_structp png_ptr, png_infop info_ptr, u32 width, u32 height) {
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }
    png_read_image(png_ptr, row_pointers);
    canvas_t *canvas = canvas_create(width, height);

    // Access individual pixels
    for (int y = 0; y < height; ++y) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width; ++x) {
            png_bytep px = &(row[x * 4]); // Each pixel has 4 bytes (RGBA)

            // Access individual color channels (R, G, B, A)
            png_byte red = px[0];
            png_byte green = px[1];
            png_byte blue = px[2];
            png_byte alpha = px[3];
            
            color_t c = RGBA(red, green, blue, alpha);
            canvas_draw_point(canvas, MAKEPOINT(x, y), c);
        }
    }

    free(row_pointers);
    return canvas;
}

canvas_t *canvas_from_png_palette(png_structp png_ptr, png_infop info_ptr, u32 width, u32 height) {

    png_colorp palette;
    int num_palette;
    png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    png_read_image(png_ptr, row_pointers);
    canvas_t *canvas = canvas_create(width, height);

    // Access individual pixels
    for (int y = 0; y < height; ++y) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width; ++x) {
            png_byte index = row[x];

            // Retrieve color information from the palette
            png_color color = palette[index];
            
            color_t c = RGBA(color.red, color.green, color.blue, 0xFF);
            canvas_draw_point(canvas, MAKEPOINT(x, y), c);
        }
    }

    free(row_pointers);
    return canvas;
}

canvas_t *canvas_from_png(const char *filename) {
    FILE* fp = fopen(filename, "rb");
    ERR_ASSERT(fp, "Error opening file: %s", filename);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    ERR_ASSERT_CLN(png_ptr, fclose(fp), "Error initializing libpng");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    ERR_ASSERT_CLN(info_ptr, {png_destroy_read_struct(&png_ptr, NULL, NULL);fclose(fp);}, "Error initializing libpng");

    ERR_ASSERT_CLN(!setjmp(png_jmpbuf(png_ptr)), { png_destroy_read_struct(&png_ptr, &info_ptr, NULL); fclose(fp);}, "Error during PNG file reading\n");

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);

    if (color_type == PNG_COLOR_TYPE_RGB) {
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER); // Convert RGB to RGBA
        png_read_update_info(png_ptr, info_ptr); // Update info after conversion
        color_type = png_get_color_type(png_ptr, info_ptr);
    }

    // You can further process or manipulate the image data here
    //
    canvas_t *canvas;

    if (color_type == PNG_COLOR_TYPE_RGBA) {
        canvas = canvas_from_png_rgba(png_ptr, info_ptr, width, height);
    } else if (color_type == PNG_COLOR_TYPE_PALETTE) {
        canvas = canvas_from_png_palette(png_ptr, info_ptr, width, height);
    } else {
        ERR_ASSERT_CLN(FALSE, png_destroy_read_struct(&png_ptr, &info_ptr, NULL), "Invalid png color type");
        exit(1); // Unreachable
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return canvas;
}

canvas_t *canvas_from_jpeg(const char *filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE* fp = fopen(filename, "rb");
    ERR_ASSERT(fp, "Error opening file: %s\n", filename);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int num_channels = cinfo.num_components;

    JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
    for (int y = 0; y < height; ++y) {
        buffer[y] = (JSAMPROW)malloc(sizeof(JSAMPLE) * width * num_channels);
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        int num_scanlines = jpeg_read_scanlines(&cinfo, buffer + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
        ERR_ASSERT(num_scanlines > 0, "Error reading JPEG scanlines");
    }

    canvas_t *canvas = canvas_create(width, height);
    // Access individual pixels
    for (int y = 0; y < height; ++y) {
        JSAMPROW row = buffer[y];
        for (int x = 0; x < width; ++x) {
            JSAMPLE* pixel = &(row[x * num_channels]);

            // Access individual color channels (depending on num_channels)
            JSAMPLE red = pixel[0];
            JSAMPLE green = pixel[1];
            JSAMPLE blue = pixel[2];

            // Do something with the pixel values
            canvas_draw_point(canvas, MAKEPOINT(x, y), RGBA(red, green, blue, 0xFF));
        }
    }

    // Cleanup
    for (int y = 0; y < height; ++y) {
        free(buffer[y]);
    }
    free(buffer);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    return canvas;
}
