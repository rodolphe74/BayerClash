#ifndef USEFUL_H
#define USEFUL_H

#include <map.h>
#include <list.h>
#include "thomson.h"

#define COLOR_COMP 4
#define PALETTE_SIZE 16
#define EXIT_FAILURE 1
#define THOMSON_SCREEN_W 320
#define THOMSON_SCREEN_H 200
#define THOMSON_SCREEN_W_FLOAT 320.0
#define THOMSON_SCREEN_H_FLOAT 200.0
#define NORMALIZE 0.01

struct Color {
	float r, g, b;
};
typedef struct Color Color;

float clamp(float v);
float clamp_deviation(float v);
Color color_add(Color a, Color b);
Color color_sub(Color a, Color b);
Color color_mul(Color a, float coef);
Color srgb_to_linear(Color pixel);
Color linear_to_srgb(Color pixel);

Color fromColorPalette(ColorPalette c);

struct Point {
	int x, y;
};
typedef struct Point Point;

struct Couple {
	int c1;
	int c2;
};
typedef struct Couple Couple;

void convert_rgba_to_float(const unsigned char *input, const int image_width, const int image_height, float *output);
void convert_float_to_rgba(const float *input, const int image_width, const int image_height, unsigned char *output);
void convert_palette_to_float(const unsigned char *input, const int palette_size, float *output);
void convert_indexed_to_rgba(const unsigned char *input, const int image_width, const int image_height,
							 const unsigned char *palette, const int palette_size, unsigned char *output);

void convert_rgba_to_lineare(const unsigned char *input, const int image_width, const int image_height, unsigned char *output);

void output_palette(const float *palette, const int palette_size, const char *filename);
Color get_pixel(unsigned char *image, int width, int x, int y, int comps);
Color get_picture_color(const unsigned char *image, int width, int height, int i, int j);
Color get_average_pixel(const unsigned char *image, int width, int height, int x, int y);
float linearize(float val);
unsigned char *linearizeImage(unsigned char *image, int width, int height);
double distance_between_colors(int index1, int index2, const ColorPalette *palette);
void display_histo(const map *histo);
void display_couples(const list *l);
void find_best_couple(const map *histo, const ColorPalette *palette, int palette_size, list *best_couples);
void free_best_couples_map();
Couple find_two_most_frequent(const map *histo);
void pset(unsigned char *image, int x, int y, ColorPalette *palette, int color_index, int width, int height);
void reduce_exoquant_palette_to_mo6(const unsigned char *input_palette, ColorPalette *output_palette, int input_size);
unsigned char *resize_if_necessary(const unsigned char *inputImage, const int ix, const int iy,
								   unsigned char *resizedImage, int *ox, int *oy);
unsigned char *frame_into_thomson_res(const unsigned char *inputData, int ix, int iy, unsigned char *outputData, int *ox,
								 int *oy);
int check_color_clash(const unsigned char *image, int width, int height, int block_size);
#endif