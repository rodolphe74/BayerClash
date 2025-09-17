#include "thomson.h"
#include <stdio.h>
#include <map.h>

ColorPalette red_255[16] = {{0, 0, 0, 0},	 {96, 0, 0, 1},	  {122, 0, 0, 2},  {142, 0, 0, 3},
							{158, 0, 0, 4},	 {170, 0, 0, 5},  {183, 0, 0, 6},  {193, 0, 0, 7},
							{204, 0, 0, 8},	 {211, 0, 0, 9},  {219, 0, 0, 10}, {226, 0, 0, 11},
							{234, 0, 0, 12}, {242, 0, 0, 13}, {249, 0, 0, 14}, {255, 0, 0, 15}};

// Green : 0 16 32 48 64 80 96 112 128 144 160 176 192 208 224 240
ColorPalette green_255[16] = {{0, 0, 0, 0},		{0, 96, 0, 16},	  {0, 122, 0, 32},	{0, 142, 0, 48},
							  {0, 158, 0, 64},	{0, 170, 0, 80},  {0, 183, 0, 96},	{0, 193, 0, 112},
							  {0, 204, 0, 128}, {0, 211, 0, 144}, {0, 219, 0, 160}, {0, 226, 0, 176},
							  {0, 234, 0, 192}, {0, 242, 0, 208}, {0, 249, 0, 224}, {0, 255, 0, 240}};

// Blue : 0 256 512 768 1024 1280 1536 1792 2048 2304 2560 2816 3072 3328 3584 3840
ColorPalette blue_255[16] = {{0, 0, 0, 0},		{0, 0, 96, 256},   {0, 0, 122, 512},  {0, 0, 142, 768},
							 {0, 0, 158, 1024}, {0, 0, 170, 1280}, {0, 0, 183, 1536}, {0, 0, 193, 1792},
							 {0, 0, 204, 2048}, {0, 0, 211, 2304}, {0, 0, 219, 2560}, {0, 0, 226, 2816},
							 {0, 0, 234, 3072}, {0, 0, 242, 3328}, {0, 0, 249, 3584}, {0, 0, 255, 3840}};

ColorPalette thomson_full_palette[4096];

// int mo5_palette_indexes[16] = {0, 15, 240, 255, 51, 204, 187, 255, 85, 170, 102, 153, 204, 51, 153, 255};
int mo5_palette_indexes[16] = {0, 15, 240, 255, 3840, 3855, 4080, 4095, 1911, 826, 931, 938, 2611, 2618, 3815, 123};
ColorPalette mo5_palette[16] = {};

void init_thomson_palette()
{
	int index = 0;
	for (int b = 0; b < 16; b++) {
		for (int g = 0; g < 16; g++) {
			for (int r = 0; r < 16; r++) {
				uint8_t r8 = red_255[r].r;
				uint8_t g8 = green_255[g].g;
				uint8_t b8 = blue_255[b].b;
				index = red_255[r].thomson_idx + green_255[g].thomson_idx + blue_255[b].thomson_idx;
				thomson_full_palette[index].r = r8;
				thomson_full_palette[index].g = g8;
				thomson_full_palette[index].b = b8;
				thomson_full_palette[index].thomson_idx = index;
				// printf("Index %4d : (r=%3d,g=%3d,b=%3d) (r8=%3d,g8=%3d,b8=%3d)\n", index, r, g, b, r8, g8, b8);
			}
		}
	}
}

void init_mo5_palette()
{
    for (int i = 0; i < 16; i++) {
        mo5_palette[i] = thomson_full_palette[mo5_palette_indexes[i]];
		mo5_palette[i].thomson_idx = i;
    }
}

