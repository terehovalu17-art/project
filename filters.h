#ifndef FILTERS_H
#define FILTERS_H

#include "image.h"

Image *filter_crop(const Image *in, int w, int h);
Image *filter_gs(const Image *in);
Image *filter_neg(const Image *in);
Image *filter_sharp(const Image *in);



#endif