#ifndef IMAGEQUANT_H
#define IMAGEQUANT_H

#include <stdio.h>
#include <stdlib.h>
#include "libimagequant.h"

void quantize(unsigned char *input_image, int width, int height, int comps, unsigned char *out_palette,
			  int palette_size);


#endif
