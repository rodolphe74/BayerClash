#ifndef USEFUL_C
#define USEFUL_C

#include "useful.h"
#include "comparator.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include <stb_image_resize2.h>

float clamp(float v)
{
	if (v < 0.0f) return 0.0f;
	if (v > 1.0f) return 1.0f;
	return v;
}

float clamp_deviation(float v)
{
	if (v < 0.0f) {
		return -v;
	} else if (v > 1.0f) {
		return v - 1.0f;
	} else {
		return 0.0f;
	}
}

// Addition de deux couleurs
Color color_add(Color a, Color b)
{
	Color result = {a.r + b.r, a.g + b.g, a.b + b.b};
	return result;
}

// Soustraction de deux couleurs
Color color_sub(Color a, Color b)
{
	Color result = {a.r - b.r, a.g - b.g, a.b - b.b};
	return result;
}

// Multiplication par un coefficient
Color color_mul(Color a, float coef)
{
	Color result = {a.r * coef, a.g * coef, a.b * coef};
	return result;
}

// Clamp d'une couleur complète
Color color_clamp(Color c)
{
	Color result = {clamp(c.r), clamp(c.g), clamp(c.b)};
	return result;
}

Color fromColorPalette(ColorPalette c)
{
    Color result;
    result.r = c.r / 255.0f;
    result.g = c.g / 255.0f;
    result.b = c.b / 255.0f;
    return result;
}


void convert_rgba_to_float(const unsigned char *input, const int image_width, const int image_height, float *output)
{
	// Iterate over pixels in the input image
	for (int y = 0; y < image_height; y++) {
		for (int x = 0; x < image_width; x++) {
			// Get the current pixel from the input buffer
			const int image_index = x + y * image_width;
			const unsigned char *pixel = &input[image_index * COLOR_COMP];
			float *out_pixel = &output[image_index * 3];

			// Convert to float and normalize to [0, 1]
			out_pixel[0] = pixel[0] / 255.0f; // R
			out_pixel[1] = pixel[1] / 255.0f; // G
			out_pixel[2] = pixel[2] / 255.0f; // B
		}
	}
}

void convert_float_to_rgba(const float *input, const int image_width, const int image_height, unsigned char *output)
{
	// Iterate over pixels in the input image
	for (int y = 0; y < image_height; y++) {
		for (int x = 0; x < image_width; x++) {
			// Get the current pixel from the input buffer
			const int image_index = x + y * image_width;
			const float *pixel = &input[image_index * 3];
			unsigned char *out_pixel = &output[image_index * COLOR_COMP];

			// Convert to unsigned char and denormalize from [0, 1]
			out_pixel[0] = (unsigned char)(fmin(fmax(pixel[0], 0.0f), 1.0f) * 255.0f); // R
			out_pixel[1] = (unsigned char)(fmin(fmax(pixel[1], 0.0f), 1.0f) * 255.0f); // G
			out_pixel[2] = (unsigned char)(fmin(fmax(pixel[2], 0.0f), 1.0f) * 255.0f); // B
			out_pixel[3] = 255;														 // A
		}
	}
}

void convert_palette_to_float(const unsigned char *input, const int palette_size, float *output)
{
	// Iterate over colors in the palette
	for (int i = 0; i < palette_size; i++) {
		const unsigned char *color = &input[i * COLOR_COMP];
		float *out_color = &output[i * 3];

		// Convert to float and normalize to [0, 1]
		out_color[0] = color[0] / 255.0f; // R
		out_color[1] = color[1] / 255.0f; // G
		out_color[2] = color[2] / 255.0f; // B
	}
}

void convert_indexed_to_rgba(const unsigned char *input, const int image_width, const int image_height,
							 const unsigned char *palette, const int palette_size, unsigned char *output)
{
	// Iterate over pixels in the input image
	for (int y = 0; y < image_height; y++) {
		for (int x = 0; x < image_width; x++) {
			// Get the current pixel from the input buffer
			const int image_index = x + y * image_width;
			const unsigned char index = input[image_index];
			const unsigned char *color = &palette[index * COLOR_COMP];
			unsigned char *out_pixel = &output[image_index * COLOR_COMP];

			// Copy the color from the palette to the output buffer
			out_pixel[0] = color[0]; // R
			out_pixel[1] = color[1]; // G
			out_pixel[2] = color[2]; // B
			out_pixel[3] = 255;		 // A
		}
	}
}

void output_palette(const float *palette, const int palette_size, const char *filename)
{
	// On force une grille carrée : par ex. 4x4 si palette_size=16
	int grid_size = (int)(sqrt((double)palette_size) + 0.5); // arrondi
	if (grid_size * grid_size < palette_size) grid_size++;	 // sécurité si palette_size pas un carré parfait

	int cell_size = 32; // taille d’un carré en pixels
	int width = grid_size * cell_size;
	int height = grid_size * cell_size;

	unsigned char *img = (unsigned char *)malloc(width * height * COLOR_COMP);
	memset(img, 255, width * height * COLOR_COMP); // fond blanc

	for (int i = 0; i < palette_size; i++) {
		int gx = i % grid_size;
		int gy = i / grid_size;

		// couleur de la palette (palette est float 0..1)
		unsigned char r = (unsigned char)(palette[i * 3 + 0] * 255.0f);
		unsigned char g = (unsigned char)(palette[i * 3 + 1] * 255.0f);
		unsigned char b = (unsigned char)(palette[i * 3 + 2] * 255.0f);

		for (int y = 0; y < cell_size; y++) {
			for (int x = 0; x < cell_size; x++) {
				int px = gx * cell_size + x;
				int py = gy * cell_size + y;
				int idx = (py * width + px) * COLOR_COMP;
				img[idx + 0] = r;
				img[idx + 1] = g;
				img[idx + 2] = b;
				img[idx + 3] = 255;
			}
		}
	}

	stbi_write_png(filename, width, height, COLOR_COMP, img, width * COLOR_COMP);
	free(img);
}

Color get_pixel(unsigned char* image, int width, int x, int y, int comps) {
    int index = (y * width + x) * comps;
    Color color;
    color.r = image[index] / 255.0f;
    color.g = image[index + 1] / 255.0f;
    color.b = image[index + 2] / 255.0f;
    return color;
};


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


// Récupère un pixel RGBA normalisé en LinearColor
Color getPictureColor(const unsigned char *image, int width, int height, int i, int j)
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
    return c;
}

// Traduction de getLinearPixel
Color get_average_pixel(const unsigned char *image, int width, int height, int x, int y)
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
            Color c = getPictureColor(image, width, height, i, j);
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

float linearize(float val) {
    if (val <= 0.081f) {
        return val / 4.5f;
    } else {
        return powf((val + 0.099f) / 1.099f, 2.2f);
    }
}

unsigned char *linearizeImage(unsigned char *image, int width, int height) {

	unsigned char *result = malloc(width * height * COLOR_COMP * sizeof(unsigned char));

	// 1. Linéarisation initiale
	int numPixels = width * height;
	for (int i = 0; i < numPixels; i++) {
		result[i * COLOR_COMP + 0] = roundf(linearize(image[i * COLOR_COMP + 0] / 255.0f) * 255.0f); // R
		result[i * COLOR_COMP + 1] = roundf(linearize(image[i * COLOR_COMP + 1] / 255.0f) * 255.0f); // G
		result[i * COLOR_COMP + 2] = roundf(linearize(image[i * COLOR_COMP + 2] / 255.0f) * 255.0f); // B
		result[i * COLOR_COMP + 3] = 1.0f; // A	
	}

	// 2. Histogramme des composantes R, G, B
	int histo[256] = {0};
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = (y * width + x) * COLOR_COMP; // index dans le buffer RGB (3 canaux)
			unsigned char r = image[idx + 0];
			unsigned char g = image[idx + 1];
			unsigned char b = image[idx + 2];
			histo[r]++;
			histo[g]++;
			histo[b]++;
		}
	}

	// 3. Calcul des seuils min/max
	double threshold = NORMALIZE * width * height * 3;
	double acc = 0.0;
	double min_val = -1.0, max_val = -1.0;

	// recherche du max (descendant)
	for (int i = 255; i >= 0; i--) {
		acc += histo[i];
		if (max_val < 0.0 && acc >= threshold) {
			max_val = linearize(i / 255.0f);
		}
	}

	// recherche du min (ascendant)
	acc = 0.0;
	for (int i = 0; i <= 255; i++) {
		acc += histo[i];
		if (min_val < 0.0 && acc >= threshold) {
			min_val = linearize(i / 255.0f);
		}
	}

	// sécurité : éviter min == max
	if (min_val >= 0.0 && max_val >= 0.0 && fabs(max_val - min_val) < 1e-9) {
		max_val = min_val + 1.0;
	}

	// 4. Remappage + clamp + écriture dans linearImage
	// linImg : tableau float* (taille w*h*3) contenant les valeurs linéarisées [0,1]
	// outImg : tableau float* (taille w*h*3) qui recevra le résultat remappé/clampé

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = (y * width + x) * COLOR_COMP;

			float r = (result[idx + 0] / 255.0f - (float)min_val) / (float)(max_val - min_val);
			float g = (result[idx + 1] / 255.0f - (float)min_val) / (float)(max_val - min_val);
			float b = (result[idx + 2] / 255.0f - (float)min_val) / (float)(max_val - min_val);

			// clamp entre 0 et 1
			if (r < 0.0f)
				r = 0.0f;
			else if (r > 1.0f)
				r = 1.0f;
			if (g < 0.0f)
				g = 0.0f;
			else if (g > 1.0f)
				g = 1.0f;
			if (b < 0.0f)
				b = 0.0f;
			else if (b > 1.0f)
				b = 1.0f;

			result[idx + 0] = roundf(r * 255.0f);
			result[idx + 1] = roundf(g * 255.0f);
			result[idx + 2] = roundf(b * 255.0f);
			result[idx + 3] = 255;
		}
	}
	return result;
}

void generateKey(const map *h, char key[64])
{
	int left = 0;
	int val;
	for (int i = 1; i <= 4; i++) {
		bk_bool here = map_get(&val, *h, &i);
		left = left * 8 + (here ? val : 0);
	}

	int right = 0;
	for (int i = 5; i <= 8; i++) {
		bk_bool here = map_get(&val, *h, &i);
		right = right * 8 + (here ? val : 0);
	}

	// allouer une chaîne "left,right"
	snprintf(key, 64, "%d,%d", left, right);
}

double distance_between_colors(int index1, int index2, const ColorPalette *palette)
{
	ColorPalette c1 = palette[index1];
	ColorPalette c2 = palette[index2];
	float dr = (c1.r - c2.r) / 255.0f;
	float dg = (c1.g - c2.g) / 255.0f;
	float db = (c1.b - c2.b) / 255.0f;
	// Pondération
	double c = 1.8;
	double d = pow(fabs(dr) * 8, c) + pow(fabs(dg) * 11, c) + pow(fabs(db) * 8, c);
	return d;
}

void display_histo(const map *histo) {
	unsigned int *first_key, *last_key, *current_key;
	first_key = map_first(*histo);
	last_key = map_last(*histo);
	current_key = first_key;
	int n = 0;
	while (current_key != NULL) {
		map_get(&n, *histo, current_key);
		printf("Color %d: %d pixels\n", *current_key, n);
		current_key = map_higher(*histo, current_key);
	}
}

void display_couples(const list *l) {
	int size = list_size(*l);
	for (int i = 0; i < size; i++) {
		Couple c;
		list_get_at(&c, *l, i);
		printf("Couple %d: (%d,%d)\n", i, c.c1, c.c2);
	}
}

static map best_couples_map = NULL;
void find_best_couple(const map *histo, const ColorPalette *palette, int palette_size, list *best_couples)
{
	if (best_couples_map == NULL) {
		best_couples_map = map_init(sizeof(char[64]), sizeof(list), string_cmp);
	}

	char key[64];
	generateKey(histo, key);
	// printf("Key: %s\n", key);

	if (map_contains(best_couples_map, key)) {
		// copy le resultat du cache dans best_couples
		list_clear(*best_couples);
		list best_couples_copy = list_init(sizeof(Couple));
		map_get(&best_couples_copy, best_couples_map, key);
		Couple *best_couples_array = (Couple *)malloc(list_size(best_couples_copy) * sizeof(Couple));
		list_copy_to_array(best_couples_array, best_couples_copy);
		list_add_all(*best_couples, best_couples_array, list_size(best_couples_copy));
		free(best_couples_array);
		return;
	}

	list_clear(*best_couples);

	double dm = DBL_MAX;
	for (int i = 0; i < 15; i++) {
		for (int j = i + 1; j < 16; j++) {
			double d = 0;

			unsigned int *first_key, *last_key, *current_key;
			first_key = map_first(*histo);
			last_key = map_last(*histo);
			current_key = first_key;
			int n = 0;
			while (current_key != NULL) {
				map_get(&n, *histo, current_key);
				double d1 = distance_between_colors(*current_key, i, palette);
				double d2 = distance_between_colors(*current_key, j, palette);
				d += n * fmin(d1, d2);
				if (d > dm) break;
				current_key = map_higher(*histo, current_key);
			}

			if (d < dm) {
				dm = d;
				list_clear(*best_couples);
			}
			if (d <= dm) {
				list_add_last(*best_couples, &(Couple){i, j});
			}
		}
	}

	// met a jour le cache en copiant best_couples
	Couple *best_couples_array = (Couple *)malloc(list_size(*best_couples) * sizeof(Couple));
	list_copy_to_array(best_couples_array, *best_couples);
	list best_couples_copy = list_init(sizeof(Couple));
	list_add_all(best_couples_copy, best_couples_array, list_size(*best_couples));
	map_put(best_couples_map, key, &best_couples_copy);
	free(best_couples_array);
}

void free_best_couples_map() {
	if (best_couples_map != NULL) {
		unsigned int *first_key = map_first(best_couples_map);
		unsigned int *current_key = first_key;
		while (current_key != NULL) {
			list l;
			// printf("Freeing key %s\n", (char *)current_key);
			map_get(&l, best_couples_map, current_key);
			list_clear(l);
			list_destroy(l);
			current_key = map_higher(best_couples_map, current_key);
		}
		map_destroy(best_couples_map);
	}
}

Couple find_two_most_frequent(const map *histo)
{
	Couple result = {-1, -1};
	int max1 = -1, max2 = -1;
	int col1 = -1, col2 = -1;

	unsigned int *current_key = map_first(*histo);
	while (current_key != NULL) {
		int count = 0;
		map_get(&count, *histo, current_key);

		if (count > max1) {
			// Décale l'ancien max1 vers max2
			max2 = max1;
			col2 = col1;

			max1 = count;
			col1 = *current_key;
		} else if (count > max2) {
			max2 = count;
			col2 = *current_key;
		}

		current_key = map_higher(*histo, current_key);
	}

	if (col1 != -1 && col2 == -1) {
		// Une seule couleur présente → duplique
		col2 = col1;
	}

	result.c1 = col1;
	result.c2 = col2;
	return result;
}

void pset(unsigned char *image, int x, int y, ColorPalette *palette, int color_index, int width, int height) {
	if (x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}
	int index = (y * width + x) * COLOR_COMP;
	ColorPalette color = palette[color_index];
	image[index] = color.r;     // R
	image[index + 1] = color.g; // G
	image[index + 2] = color.b; // B
	image[index + 3] = 255;     // A
}


void reduce_exoquant_palette_to_mo6(const unsigned char *input_palette, ColorPalette *output_palette, int input_size)
{
	for (int i = 0; i < input_size; i++) {
		// ColorPalette mo6_color = output_palette[i];
		int best_index = 0;
		double best_distance = DBL_MAX;
		unsigned char r = input_palette[i * 4 + 0];
		unsigned char g = input_palette[i * 4 + 1];
		unsigned char b = input_palette[i * 4 + 2];
		for (int j = 0; j < 4096; j++) {
			unsigned char dr = thomson_full_palette[j].r;
			unsigned char dg = thomson_full_palette[j].g;
			unsigned char db = thomson_full_palette[j].b;
			int drc = r - dr;
			int dgc = g - dg;
			int dbc = b - db;
			// distance perceptuelle
			double distance = 0.299 * drc * drc + 0.587 * dgc * dgc + 0.114 * dbc * dbc;
			if (distance < best_distance) {
				best_distance = distance;
				best_index = j;
			}
		}
		output_palette[i].r = thomson_full_palette[best_index].r;
		output_palette[i].g = thomson_full_palette[best_index].g;
		output_palette[i].b = thomson_full_palette[best_index].b;
		output_palette[i].thomson_idx = i;
	}
}

unsigned char *resize_if_necessary(const unsigned char *inputImage, const int ix, const int iy,
								   unsigned char *resizedImage, int *ox, int *oy)
{
	float ratioX = 0, ratioY = 0, ratio;
	int doResize = 0;

	ratioX = ix / 320.0;
	printf("ratio x -> %f\n", ratioX);
	doResize = 1;

	if (iy > 200) {
		ratioY = iy / 200.0;
		printf("ratio y -> %f\n", ratioY);
		doResize = 1;
	}

	if (doResize) {
		ratio = fmax(ratioX, ratioY);
		printf("ratio -> %f\n", ratio);

		int xx, yy;
		xx = ix / ratio;
		yy = iy / ratio;

		printf("Nouvelles dimensions %d*%d\n", xx, yy);

		resizedImage = malloc(xx * yy * COLOR_COMP);
		stbir_resize_uint8_linear(inputImage, ix, iy, COLOR_COMP * ix, resizedImage, xx, yy, xx * COLOR_COMP,
								  COLOR_COMP);
		*ox = xx;
		*oy = yy;
		return resizedImage;
	}
	return NULL;
}

unsigned char *frame_into_thomson_res(const unsigned char *inputData, int ix, int iy, unsigned char *outputData, int *ox,
								 int *oy)
{
	int targetw = THOMSON_SCREEN_W;
	int targeth = THOMSON_SCREEN_H;

	outputData = malloc(targetw * targeth * COLOR_COMP);
	if (outputData) {
		memset(outputData, 0, targetw * targeth * COLOR_COMP);
		int k = 0, l = 0;
		for (int j = 0; j < iy; j++) {
			for (int i = 0; i < ix; i++) {
				if (j < targeth && i < targetw) {
					outputData[(k * targetw + l) * COLOR_COMP] = inputData[(j * ix + i) * COLOR_COMP];
					outputData[(k * targetw + l) * COLOR_COMP + 1] = inputData[(j * ix + i) * COLOR_COMP + 1];
					outputData[(k * targetw + l) * COLOR_COMP + 2] = inputData[(j * ix + i) * COLOR_COMP + 2];
					outputData[(k * targetw + l) * COLOR_COMP + 3] = inputData[(j * ix + i) * COLOR_COMP + 3];
				}
				l++;
			}
			l = 0;
			k++;
		}
		*ox = targetw;
		*oy = targeth;
		return outputData;
	}
	return NULL;
}

#endif