#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#pragma pack(push,1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)


Image *image_create(int w, int h){
    Image *img = malloc(sizeof(Image));
    if (!img) return NULL;
    img->width = w;
    img->height = h;
    size_t n = (size_t)w * h * 3;
    img->data = malloc(n);
    if (!img) return NULL;
    memset(img->data, 0, n);
    return img;
}
void image_free(Image *img){
    if(!img) return;
    free(img->data);
    free(img);
}

Image *image_load_bmp(const char *path){
    FILE *f = fopen(path, "rb");
    if(!f) return NULL;
    BITMAPFILEHEADER fh;
    if(fread(&fh, sizeof(fh), 1, f) != 1){ fclose(f); return NULL; }
    if(fh.bfType != 0x4D42) { fclose(f); return NULL; }
    BITMAPINFOHEADER ih;
    if(fread(&ih, sizeof(ih), 1, f) != 1){ fclose(f); return NULL; }
    if(ih.biBitCount != 24 || ih.biCompression != 0 || ih.biSize != 40){
        fclose(f);
        return NULL;
    }
    int w = ih.biWidth;
    int h = abs(ih.biHeight);
    Image *img = image_create(w, h);
    int row_bytes = w * 3;
    int pad = (4 - (row_bytes % 4)) % 4;
    fseek(f, fh.bfOffBits, SEEK_SET);
    unsigned char *rowbuf = malloc((size_t)row_bytes + pad);
    if (!img) return NULL;
    int top_down = (ih.biHeight < 0);
    for(int row = 0; row < h; ++row){
        if(fread(rowbuf, 1, (size_t)row_bytes + pad, f) != (size_t)row_bytes + pad){
            free(rowbuf); image_free(img); fclose(f); return NULL;
        }
        int dst_row = top_down ? row : (h - 1 - row);
        unsigned char *dst = img->data + (size_t)dst_row * w * 3;
        for(int x=0;x<w;++x){
            unsigned char b = rowbuf[3*x + 0];
            unsigned char g = rowbuf[3*x + 1];
            unsigned char r = rowbuf[3*x + 2];
            dst[3*x + 0] = r;
            dst[3*x + 1] = g;
            dst[3*x + 2] = b;
        }
    }
    free(rowbuf);
    fclose(f);
    return img;
}

int image_save_bmp(const char *path, const Image *img){
    if(!img) return 0;
    FILE *f = fopen(path, "wb");
    if(!f) return 0;
    int w = img->width, h = img->height;
    int row_bytes = w * 3;
    int pad = (4 - (row_bytes % 4)) % 4;
    uint32_t pixels_size = (row_bytes + pad) * h;
    BITMAPFILEHEADER fh;
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fh.bfSize = fh.bfOffBits + pixels_size;
    fh.bfReserved1 = fh.bfReserved2 = 0;
    BITMAPINFOHEADER ih;
    ih.biSize = 40;
    ih.biWidth = w;
    ih.biHeight = h;
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    ih.biCompression = 0;
    ih.biSizeImage = pixels_size;
    ih.biXPelsPerMeter = 3780;
    ih.biYPelsPerMeter = 3780;
    ih.biClrUsed = 0;
    ih.biClrImportant = 0;
    if(fwrite(&fh, sizeof(fh), 1, f) != 1) { fclose(f); return 0; }
    if(fwrite(&ih, sizeof(ih), 1, f) != 1) { fclose(f); return 0; }
    unsigned char *rowbuf = malloc((size_t)row_bytes + pad);
    if(!rowbuf){ fclose(f); return 0; }
    for(int row = 0; row < h; ++row){
        int src_row = h - 1 - row; 
        const unsigned char *src = img->data + (size_t)src_row * w * 3;
        for(int x=0;x<w;++x){
            rowbuf[3*x + 0] = src[3*x + 2];
            rowbuf[3*x + 1] = src[3*x + 1];
            rowbuf[3*x + 2] = src[3*x + 0];
        }
        for(int p=0;p<pad;++p) rowbuf[row_bytes + p] = 0;
        if(fwrite(rowbuf, 1, (size_t)row_bytes + pad, f) != (size_t)row_bytes + pad){
            free(rowbuf); fclose(f); return 0;
        }
    }
    free(rowbuf);
    fclose(f);
    return 1;
}

void get_pixel(const Image *img, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b){
    if(x < 0) x = 0;
    if(x >= img->width) x = img->width - 1;
    if(y < 0) y = 0;
    if(y >= img->height) y = img->height - 1;
    const unsigned char *p = img->data + ((size_t)y * img->width + x) * 3;
    *r = p[0]; *g = p[1]; *b = p[2];
}
void set_pixel(Image *img, int x, int y, unsigned char r, unsigned char g, unsigned char b){
    if(x < 0 || x >= img->width || y < 0 || y >= img->height) return;
    unsigned char *p = img->data + ((size_t)y * img->width + x) * 3;
    p[0] = r; p[1] = g; p[2] = b;
}