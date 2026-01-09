#include "filters.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

static unsigned char clamp8(int v){
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}
static double clampd(double v){
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}


Image *filter_crop(const Image *in, int new_w, int new_h){
    if(new_w <= 0) new_w = 0;
    if(new_h <= 0) new_h = 0;
    int w = (new_w > in->width) ? in->width : new_w;
    int h = (new_h > in->height) ? in->height : new_h;
    Image *out = image_create(w, h);
    for(int y=0;y<h;++y){
        memcpy(out->data + (size_t)y*w*3, in->data + (size_t)y*in->width*3, (size_t)w*3);
    }
    return out;
}


Image *filter_gs(const Image *in){
    Image *out = image_create(in->width, in->height);
    for(int y=0;y<in->height;++y){
        for(int x=0;x<in->width;++x){
            unsigned char r,g,b;
            get_pixel(in,x,y,&r,&g,&b);
            double rd = r/255.0, gd = g/255.0, bd = b/255.0;
            double lum = 0.299*rd + 0.587*gd + 0.114*bd;
            unsigned char v = clamp8((int)floor(lum*255.0 + 0.5));
            set_pixel(out,x,y,v,v,v);
        }
    }
    return out;
}


Image *filter_neg(const Image *in){
    Image *out = image_create(in->width, in->height);
    for(int i=0;i<in->width*in->height*3;++i){
        out->data[i] = 255 - in->data[i];
    }
    return out;
}


Image *filter_convolve(const Image *in, const double *kernel, int ksize){
    int w = in->width, h = in->height;
    Image *out = image_create(w,h);
    int khalf = ksize/2;
    double sum = 0.0;
    for(int i=0;i<ksize*ksize;++i) sum += kernel[i];
    for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
            double rr=0, gg=0, bb=0;
            for(int ky=0; ky<ksize; ++ky){
                for(int kx=0; kx<ksize; ++kx){
                    int sx = x + (kx - khalf);
                    int sy = y + (ky - khalf);
                    unsigned char r,g,b;
                    get_pixel(in, sx, sy, &r, &g, &b);
                    double k = kernel[ky*ksize + kx];
                    rr += k * (r/255.0);
                    gg += k * (g/255.0);
                    bb += k * (b/255.0);
                }
            }
            if (fabs(sum) > 1e-12) { rr /= sum; gg /= sum; bb /= sum; }
            
            rr = clampd(rr); gg = clampd(gg); bb = clampd(bb);
            set_pixel(out, x, y, clamp8((int)floor(rr*255.0+0.5)), clamp8((int)floor(gg*255.0+0.5)), clamp8((int)floor(bb*255.0+0.5)));
        }
    }
    return out;
}

Image *filter_sharp(const Image *in){
    double k[9] = {0,-1,0, -1,5,-1, 0,-1,0};
    return filter_convolve(in, k, 3);
}

 