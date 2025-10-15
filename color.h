#ifndef COLOR_H
#define COLOR_H

#include "gx2reduce.h"
#include "libimagequant.h"
#include <math.h>
#include <map.h>
#include <list.h>

#define COLOR_COMP 4

struct Color {
	double r, g, b;
};
typedef struct Color Color;

typedef struct {
	unsigned char r, g, b;
	uint16_t thomson_idx;
} ColorPalette;

typedef struct Couple {
	int c1;
	int c2;
} Couple;


Color fromColorPalette(ColorPalette palette_color);
ColorPalette toColorPalette(Color normalized_color);
Color srgb_to_linear(Color pixel);
Color linear_to_srgb(Color pixel);
float clamp(float v);
float clamp_deviation(float v);
Color color_add(Color a, Color b);
Color color_sub(Color a, Color b);
Color color_mul(Color a, float coef);
Color color_add_att(Color a, Color d, float factor);
Color color_clamp(Color c);

double distance_between_colors(Color c1, Color c2);
double distance_between_colors_index(int index1, int index2, const ColorPalette *palette);
void quantize(const unsigned char *input_image, int width, int height, int comps, unsigned char *out_palette,
			  int palette_size);
void copy_palette(ColorPalette *target, const ColorPalette *source, int palette_size);
void find_palette(const unsigned char *image, int width, int height, ColorPalette *palette, unsigned char palette_size);
int find_closest_color_from_palette(Color c, ColorPalette *palette, int palette_size, Color *d);
void output_palette(const ColorPalette *palette, const int palette_size, const char *filename);
void linearize_palette(const ColorPalette *palette, ColorPalette *output_palette, int palette_size);
void palette_to_floats(const ColorPalette *palette, float *output_palette, int palette_size);
void find_best_couple(const map *histo, const ColorPalette *palette, int palette_size, list *best_couples);
void free_best_couples_map();
#endif