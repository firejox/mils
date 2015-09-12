#include "image.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <jpeglib.h>

#define MAX_DIMENSION 10000

static cairo_surface_t* load_png_surface (const char *fn) {
    cairo_surface_t *png;
    cairo_status_t st;

    png = cairo_image_surface_create_from_png (fn);
    st = cairo_surface_status (png);

    if (st == CAIRO_STATUS_SUCCESS) 
        return png;
    cairo_surface_destroy (png);

    return NULL;
}

static cairo_surface_t* load_jpeg_surface (const char *fn) {
    cairo_surface_t *jpeg;
    cairo_status_t st;
    size_t width, height, comps;
    int scan_width;
    int stride;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    uint8_t *data, *ptr;
    int i;
    FILE *fp = fopen (fn, "rb");

    if (fp == NULL) {
        fprintf (stderr, "open file failed: %s", 
                fn);
        return NULL;
    }

    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_decompress (&cinfo);
    jpeg_stdio_src (&cinfo, fp);
    jpeg_read_header (&cinfo, TRUE);
    cinfo.out_color_space = JCS_EXT_BGRA;

    jpeg_calc_output_dimensions (&cinfo);

    if (cinfo.output_width >= MAX_DIMENSION || 
            cinfo.output_height >= MAX_DIMENSION) {
        fprintf (stderr, "Dimension of image is too large.\n");
        fclose (fp);
        return NULL;
    }


    jpeg_start_decompress (&cinfo);

    width = cinfo.output_width;
    height = cinfo.output_height;
    comps = cinfo.out_color_components;
    stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);



    data = xmalloc (stride*height);
    
    ptr = data;
    while (cinfo.output_scanline < height) {
        jpeg_read_scanlines (&cinfo, &ptr, 1);
        ptr += stride;
    }

    jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);
    fclose (fp);

    jpeg = cairo_image_surface_create_for_data (
            data, CAIRO_FORMAT_ARGB32, width, height, stride);

    st = cairo_surface_status (jpeg);

    if (st == CAIRO_STATUS_SUCCESS) 
        return jpeg;
    cairo_surface_destroy (jpeg);

    return NULL;

}


static const char* get_ext (const char *fn) {
    char *ext = NULL;
    ext = strrchr (fn, '.');
    if (ext == NULL)
        return NULL;
    return (ext + 1);
}

cairo_surface_t* get_image_surface (const char *fn) {
    const char *ext = get_ext (fn);
    if (ext == NULL) 
        return NULL;

    if (!strcmp (ext, "png")) 
        return load_png_surface (fn);
    else if (!strcmp (ext, "jpeg")
            || !strcmp (ext, "jpg"))
        return load_jpeg_surface (fn);
    else
        return NULL;
}
