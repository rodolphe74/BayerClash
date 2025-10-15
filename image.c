#include "image.h"
#include "thomson.h"

Point thom2screen(int x, int y, int screenW, int screenH, int centered)
{
	double i, j;

	if ((double)screenW / screenH < 1.6) {
		double o = centered ? (screenW - screenH * 1.6) / 2.0 : 0.0;
		i = x * (double)screenH / THOMSON_SCREEN_H_FLOAT + o;
		j = y * (double)screenH / THOMSON_SCREEN_H_FLOAT;
	} else {
		double o = centered ? (screenH - screenW / 1.6) / 2.0 : 0.0;
		i = x * (double)screenW / THOMSON_SCREEN_W_FLOAT;
		j = y * (double)screenW / THOMSON_SCREEN_W_FLOAT + o;
	}

	Point p;
	p.x = (int)floor(i);
	p.y = (int)floor(j);
	return p;
}

Color get_picture_color_linear(const unsigned char *image, int width, int height, int i, int j)
{
	if (i < 0 || j < 0 || i >= width || j >= height) {
		Color black = {0.0f, 0.0f, 0.0f};
		return black;
	}

	int idx = (j * width + i) * COLOR_COMP; // RGBA
	Color c;
	c.r = image[idx + 0] / 255.0f;
	c.g = image[idx + 1] / 255.0f;
	c.b = image[idx + 2] / 255.0f;

	return  srgb_to_linear(c);
}


Color get_average_pixel_linear(const unsigned char *image, int width, int height, int x, int y)
{
	Point p1 = thom2screen(x, y, width, height, 1);
	Point p2 = thom2screen(x + 1, y + 1, width, height, 1);

	int x1 = p1.x;
	int y1 = p1.y;
	int x2 = p2.x;
	int y2 = p2.y;

	if (x2 == x1) x2 = x1 + 1;
	if (y2 == y1) y2 = y1 + 1;

	float r = 0.0f, g = 0.0f, b = 0.0f;
	int count = 0;

	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++) {
			Color c = get_picture_color_linear(image, width, height, i, j);
			r += c.r;
			g += c.g;
			b += c.b;
			count++;
		}
	}

	if (count == 0) {
		Color black = {0.0f, 0.0f, 0.0f};
		return black;
	}

	Color out;
	out.r = r / count;
	out.g = g / count;
	out.b = b / count;
	return out;
}

void pset(unsigned char *image, int x, int y, ColorPalette *palette, int color_index, int width, int height)
{
	if (x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}
	int index = (y * width + x) * COLOR_COMP;
	ColorPalette color = palette[color_index];

	image[index] = color.r;		// R
	image[index + 1] = color.g; // G
	image[index + 2] = color.b; // B
	image[index + 3] = 255;		// A

}
