#ifndef IMAGE_H
#define IMAGE_H

#include "color.h"

struct Point {
	int x, y;
};
typedef struct Point Point;

Point thom2screen(int x, int y, int screenW, int screenH, int centered);
Color get_picture_color_linear(const unsigned char *image, int width, int height, int i, int j);
Color get_average_pixel_linear(const unsigned char *image, int width, int height, int x, int y);
void pset(unsigned char *image, int x, int y, ColorPalette *palette, int color_index, int width, int height);
#endif