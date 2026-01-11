#ifndef FILTERS_H
#define FILTERS_H

#include "image.h"

Image *filter_crop(const Image *in, int w, int h);
Image *filter_gs(const Image *in);
Image *filter_neg(const Image *in);
Image *filter_sharp(const Image *in);
Image *filter_edge(const Image *in, double threshold);
Image *filter_med(const Image *in, int window);
Image *filter_blur(const Image *in, double sigma);

Image *filter_crys(const Image *in, int size);
Image *filter_glass(const Image *in, int radius);



#endif
