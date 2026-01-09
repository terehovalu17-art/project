#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

Image *image_create(int w, int h);
void image_free(Image *img);

Image *image_load_bmp(const char *path);
int image_save_bmp(const char *path, const Image *img);

void get_pixel(const Image *img, int x, int y,
               unsigned char *r, unsigned char *g, unsigned char *b);
void set_pixel(Image *img, int x, int y,
               unsigned char r, unsigned char g, unsigned char b);

#endif
