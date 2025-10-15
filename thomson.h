#ifndef THOMSON_H
#define THOMSON_H

#define PALETTE_SIZE 16
#define HALF_PALETTE_SIZE 8
#define COLOR_COMP 4
#define THOMSON_SCREEN_W 320
#define THOMSON_SCREEN_H 200
#define THOMSON_SCREEN_W_FLOAT 320.0
#define THOMSON_SCREEN_H_FLOAT 200.0

#include "color.h"
#include <stdint.h>
#include <list.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
	uint8_t *data;
	size_t size;
	size_t capacity;
} IntVector;

typedef struct {
	uint8_t columns;
	uint8_t lines;
	IntVector rama;
	IntVector ramb;
} MAP_SEG;

extern ColorPalette thomson_full_palette[4096];
extern int mo5_palette_indexes[16];
extern ColorPalette mo5_palette[16];

void init_thomson_palette();
void init_mo5_palette();
void reduce_palette_to_mo6_color_space(const ColorPalette *input_palette, ColorPalette *output_palette, int input_size);
void init_vector(IntVector *vec);
void push_back(IntVector *vec, uint8_t value);
void free_vector(IntVector *vec);
void save_as_to_snap(const char *name, const uint8_t *output_image_data, ColorPalette thomson_palette[PALETTE_SIZE], IntVector *pixels, IntVector *colors);



#endif