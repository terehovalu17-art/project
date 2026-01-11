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
Image *filter_edge(const Image *in, double threshold){
    Image *gs = filter_gs(in);
    double k[9] = {-1,-1,-1, -1,8,-1, -1,-1,-1};
    Image *conv = filter_convolve(gs, k, 3);
    image_free(gs);
    Image *out = image_create(conv->width, conv->height);
    for(int y=0;y<conv->height;++y){
        for(int x=0;x<conv->width;++x){
            unsigned char r,g,b;
            get_pixel(conv,x,y,&r,&g,&b);
            
            double v = (r/255.0);
            unsigned char vv = (v > threshold) ? 255 : 0;
            set_pixel(out,x,y,vv,vv,vv);
        }
    }
    image_free(conv);
    return out;
}

int compare_uc(const void *a, const void *b){
    return (*(unsigned char*)a) - (*(unsigned char*)b);
}
Image *filter_med(const Image *in, int window){
    if(window <= 1) return image_create(in->width, in->height);
    if(window % 2 == 0) window++;
    int w = in->width, h = in->height;
    Image *out = image_create(w,h);
    int r = window/2;
    int bufsize = window*window;
    unsigned char *rb = malloc(bufsize);
    unsigned char *gb = malloc(bufsize);
    unsigned char *bb = malloc(bufsize);
    if(!rb || !gb || !bb){
        free(rb); free(gb); free(bb);
        return NULL;
    }
    for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
            int idx=0;
            for(int dy=-r; dy<=r; ++dy){
                for(int dx=-r; dx<=r; ++dx){
                    unsigned char r0,g0,b0;
                    get_pixel(in, x+dx, y+dy, &r0,&g0,&b0);
                    rb[idx]=r0; gb[idx]=g0; bb[idx]=b0; idx++;
                }
            }
            qsort(rb, bufsize, 1, compare_uc);
            qsort(gb, bufsize, 1, compare_uc);
            qsort(bb, bufsize, 1, compare_uc);
            unsigned char mr = rb[bufsize/2];
            unsigned char mg = gb[bufsize/2];
            unsigned char mb = bb[bufsize/2];
            set_pixel(out,x,y,mr,mg,mb);
        }
    }
    free(rb); free(gb); free(bb);
    return out;
}

double *gaussian_kernel_1d(double sigma, int *out_k){
    if(sigma <= 0){
        *out_k = 1;
        double *k = malloc(sizeof(double));
        k[0] = 1.0;
        return k;
    }
    int radius = (int)ceil(3.0 * sigma);
    int size = radius*2 + 1;
    double *k = malloc(sizeof(double) * size);
    double sum = 0.0;
    for(int i=-radius;i<=radius;++i){
        double val = exp(-(i*i)/(2.0*sigma*sigma));
        k[i+radius] = val;
        sum += val;
    }
    for(int i=0;i<size;++i) k[i] /= sum;
    *out_k = size;
    return k;
}
Image *filter_blur(const Image *in, double sigma){
    int w = in->width, h = in->height;
    int ksize;
    double *k = gaussian_kernel_1d(sigma, &ksize);
    int kr = ksize/2;
    
    Image *tmp = image_create(w,h);
    for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
            double rr=0, gg=0, bb=0;
            for(int i=0;i<ksize;++i){
                int sx = x + (i - kr);
                unsigned char r,g,b;
                get_pixel(in, sx, y, &r,&g,&b);
                rr += k[i] * (r/255.0);
                gg += k[i] * (g/255.0);
                bb += k[i] * (b/255.0);
            }
            set_pixel(tmp, x, y, clamp8((int)floor(clampd(rr)*255.0+0.5)), clamp8((int)floor(clampd(gg)*255.0+0.5)), clamp8((int)floor(clampd(bb)*255.0+0.5)));
        }
    }

    Image *out = image_create(w,h);
    for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
            double rr=0, gg=0, bb=0;
            for(int i=0;i<ksize;++i){
                int sy = y + (i - kr);
                unsigned char r,g,b;
                get_pixel(tmp, x, sy, &r,&g,&b);
                rr += k[i] * (r/255.0);
                gg += k[i] * (g/255.0);
                bb += k[i] * (b/255.0);
            }
            set_pixel(out, x, y, clamp8((int)floor(clampd(rr)*255.0+0.5)), clamp8((int)floor(clampd(gg)*255.0+0.5)), clamp8((int)floor(clampd(bb)*255.0+0.5)));
        }
    }
    free(k);
    image_free(tmp);
    return out;
}


Image *filter_crys(const Image *in, int size){
    if(size <= 1) return filter_med(in, 3); 
    int w = in->width, h = in->height;
    Image *out = image_create(w,h);
    for(int by=0; by<h; by += size){
        for(int bx=0; bx<w; bx += size){
            
            long sr=0, sg=0, sb=0;
            int cnt=0;
            for(int y=by; y<by+size && y<h; ++y){
                for(int x=bx; x<bx+size && x<w; ++x){
                    unsigned char r,g,b;
                    get_pixel(in,x,y,&r,&g,&b);
                    sr += r; sg += g; sb += b; cnt++;
                }
            }
            if(cnt==0) continue;
            unsigned char ar = sr / cnt;
            unsigned char ag = sg / cnt;
            unsigned char ab = sb / cnt;
            for(int y=by; y<by+size && y<h; ++y){
                for(int x=bx; x<bx+size && x<w; ++x){
                    set_pixel(out,x,y,ar,ag,ab);
                }
            }
        }
    }
    return out;
}


Image *filter_glass(const Image *in, int radius){
    if(radius <= 0) return image_create(in->width, in->height);
    srand((unsigned)time(NULL));
    int w = in->width, h = in->height;
    Image *out = image_create(w,h);
    for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
            int dx = (rand() % (2*radius+1)) - radius;
            int dy = (rand() % (2*radius+1)) - radius;
            unsigned char r,g,b;
            get_pixel(in, x+dx, y+dy, &r,&g,&b);
            set_pixel(out,x,y,r,g,b);
        }
    }
    return out;
}
 
