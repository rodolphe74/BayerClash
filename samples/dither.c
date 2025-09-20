#include <stdio.h>
#include <exoquant.h>
#include "useful.h"


ColorPalette palette[16];

void quantize(unsigned char *image, int w, int h, ColorPalette color_palette[PALETTE_SIZE], int size)
{
    unsigned char palette[PALETTE_SIZE * 4];
	exq_data *pExqPalette;
	pExqPalette = exq_init();
	exq_feed(pExqPalette, image, w * h);
	exq_quantize_hq(pExqPalette, PALETTE_SIZE);
	exq_get_palette(pExqPalette, palette, PALETTE_SIZE);
	exq_free(pExqPalette);
	for (int i = 0; i < PALETTE_SIZE; i++) {
		color_palette[i].r = palette[i * 4];
		color_palette[i].g = palette[i * 4 + 1];
		color_palette[i].b = palette[i * 4 + 2];
		color_palette[i].idx = i;
	}
}


int main() {
    return 0;
}