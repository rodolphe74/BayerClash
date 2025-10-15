#include "color.h"
#include "comparator.h"
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
// #define STB_IMAGE_RESIZE2_IMPLEMENTATION
// #include <stb_image_resize2.h>

Color fromColorPalette(ColorPalette palette_color)
{
	Color normalized_color;
	normalized_color.r = (double)palette_color.r / 255.0;
	normalized_color.g = (double)palette_color.g / 255.0;
	normalized_color.b = (double)palette_color.b / 255.0;
	return normalized_color;
}

ColorPalette toColorPalette(Color normalized_color)
{
	ColorPalette byte_color;
	double val_r = round(normalized_color.r * 255.0);
	byte_color.r = (unsigned char)fmax(0.0, fmin(255.0, val_r));
	double val_g = round(normalized_color.g * 255.0);
	byte_color.g = (unsigned char)fmax(0.0, fmin(255.0, val_g));
	double val_b = round(normalized_color.b * 255.0);
	byte_color.b = (unsigned char)fmax(0.0, fmin(255.0, val_b));
	byte_color.thomson_idx = 0;
	return byte_color;
}


Color srgb_to_linear(Color pixel)
{
	Color c;
	c.r = pixel.r > 0.04045f ? powf((pixel.r + 0.055f) / 1.055f, 2.4f) : pixel.r / 12.92f;
	c.g = pixel.g > 0.04045f ? powf((pixel.g + 0.055f) / 1.055f, 2.4f) : pixel.g / 12.92f;
	c.b = pixel.b > 0.04045f ? powf((pixel.b + 0.055f) / 1.055f, 2.4f) : pixel.b / 12.92f;
	return c;
}

Color linear_to_srgb(Color pixel)
{
	Color c;
	c.r = pixel.r > 0.0031308f ? 1.055f * powf(pixel.r, 1.0f / 2.4f) - 0.055f : 12.92f * pixel.r;
	c.g = pixel.g > 0.0031308f ? 1.055f * powf(pixel.g, 1.0f / 2.4f) - 0.055f : 12.92f * pixel.g;
	c.b = pixel.b > 0.0031308f ? 1.055f * powf(pixel.b, 1.0f / 2.4f) - 0.055f : 12.92f * pixel.b;
	return c;
}

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

Color color_add_att(Color a, Color d, float factor)
{
	a.r += d.r * factor;
	a.g += d.g * factor;
	a.b += d.b * factor;
	return a;
}

// Clamp d'une couleur complète
Color color_clamp(Color c)
{
	Color result = {clamp(c.r), clamp(c.g), clamp(c.b)};
	return result;
}

void quantize(const unsigned char *input_image, int width, int height, int comps, unsigned char *out_palette,
			  int palette_size)
{
	if (comps != 4) return;

	liq_attr *attr = liq_attr_create();
	if (!attr) return;

	// 1. Réglages clés : Vitesse et Qualité

	// Vitesse 1 : La plus lente, mais maximise la qualité de la palette trouvée
	liq_set_speed(attr, 1);

	// Qualité 0 : Assure que la quantification réussit toujours (LIQ_OK)
	liq_set_quality(attr, 0, 100);

	// Définir le nombre de couleurs désiré (e.g., 16)
	liq_set_max_colors(attr, palette_size);

	// [ Optionnel : Si vous devez garantir une couleur transparente ]
	// liq_set_last_index_transparent(attr, 1);

	// 2. Création de l'image
	liq_image *liq_input = liq_image_create_rgba(attr, input_image, width, height, 0);
	if (!liq_input) {
		liq_attr_destroy(attr);
		return;
	}

	// 3. Quantification
	liq_result *quant_result;
	liq_error err = liq_image_quantize(liq_input, attr, &quant_result);

	if (err != LIQ_OK) {
		fprintf(stderr, "Erreur de quantification (Code: %d). \n", err);
		liq_image_destroy(liq_input);
		liq_attr_destroy(attr);
		return;
	}

	// 4. Extraction de la Palette
	const liq_palette *palette = liq_get_palette(quant_result);
	int actual_colors = palette->count;

	for (int i = 0; i < actual_colors; i++) {
		const liq_color color = palette->entries[i];

		out_palette[i * 4 + 0] = color.r;
		out_palette[i * 4 + 1] = color.g;
		out_palette[i * 4 + 2] = color.b;
		out_palette[i * 4 + 3] = color.a;
	}

	// 5. Nettoyage
	liq_result_destroy(quant_result);
	liq_image_destroy(liq_input);
	liq_attr_destroy(attr);
}

void copy_palette(ColorPalette *target, const ColorPalette *source, int palette_size)
{
    for (int i = 0; i < palette_size; i++) {
        target[i].r = source[i].r;
        target[i].g = source[i].g;
        target[i].b = source[i].b;
        target[i].thomson_idx = source[i].thomson_idx;
    }
}

double distance_between_colors(Color c1, Color c2)
{
	float dr = (c1.r - c2.r);
	float dg = (c1.g - c2.g);
	float db = (c1.b - c2.b);
	// Pondération
	double c = 1.8;
	double d = pow(fabs(dr) * 8, c) + pow(fabs(dg) * 11, c) + pow(fabs(db) * 8, c);
	return d;
}

double distance_between_colors_index(int index1, int index2, const ColorPalette *palette)
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

int find_closest_from_palette(Color c, unsigned char *palette, int palette_size, Color *d)
{
	float max_distance = FLT_MAX;
	int idx = 0;
	Color r;
	for (int i = 0; i < palette_size; i++) {
		Color d = {palette[i * 4] / 255.0, palette[i * 4 + 1] / 255.0, palette[i * 4 + 2] / 255.0};
		// d = srgb_to_linear(d);
		float distance = distance_between_colors(d, c);
		if (distance < max_distance) {
			idx = i;
			r = d;
			max_distance = distance;
		}
	}
	*d = r;
	return idx;
}

void find_palette(const unsigned char *image, int width, int height, ColorPalette *palette, unsigned char palette_size)
{
// recherche d'abord une palette de 256 couleurs
#define P256 256
#define P256_4 1024
	unsigned char palette_256[P256_4];
	quantize(image, width, height, COLOR_COMP, palette_256, P256);
	float f256[P256 * 3];
	memset(f256, 0, P256 * 3 * sizeof(float));

	dword color_usage[256];
	memset(color_usage, 0, sizeof(color_usage));
	for (int i = 0; i < width * height; i++) {
		Color d;
		Color c = {image[i * COLOR_COMP] / 255.0, image[i * COLOR_COMP + 1] / 255.0, image[i * COLOR_COMP + 2] / 255.0};
		int idx = find_closest_from_palette(c, palette_256, P256, &d);
		color_usage[idx]++;
	}

	short used_colors = 256;
	T_Palette p;

	for (int i = 0; i < 256; i++) {
		p[i].R = palette_256[i * 4];
		p[i].G = palette_256[i * 4 + 1];
		p[i].B = palette_256[i * 4 + 2];
	}

	// puis ressert la palette a 16 coulerur avec l'algo de Grafx2
	Reduce_palette(&used_colors, palette_size, p, color_usage);

	for (int i = 0; i < palette_size; i++) {
		palette[i].r = p[i].R;
		palette[i].g = p[i].G;
		palette[i].b = p[i].B;
		palette[i].thomson_idx = 0;
	}
}

int find_closest_color_from_palette(Color c, ColorPalette *palette, int palette_size, Color *d)
{
	float max_distance = FLT_MAX;
	int idx = 0;
	Color r;
	for (int i = 0; i < palette_size; i++) { // 8 premieres couleurs de la palette
		Color d = {palette[i].r / 255.0f, palette[i].g / 255.0f, palette[i].b / 255.0f};
		float distance = distance_between_colors(d, c);
		if (distance < max_distance) {
			idx = i;
			r = d;
			max_distance = distance;
		}
	}
	*d = r;
	return idx;
}

void output_palette(const ColorPalette *palette, const int palette_size, const char *filename)
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
		unsigned char r = (unsigned char)(palette[i].r);
		unsigned char g = (unsigned char)(palette[i].g);
		unsigned char b = (unsigned char)(palette[i].b);

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

void linearize_palette(const ColorPalette *palette, ColorPalette *output_palette, int palette_size)
{
	for (int i = 0; i < palette_size; i++) {
		Color c = {palette[i].r / 255.0f, palette[i].g / 255.0f, palette[i].b / 255.0f};
		c = srgb_to_linear(c);
		output_palette[i].r = c.r * 255.0f;
		output_palette[i].g = c.g * 255.0f;
		output_palette[i].b = c.b * 255.0f;
		output_palette[i].thomson_idx = palette[i].thomson_idx;
	}
}

void palette_to_floats(const ColorPalette *palette, float *output_palette, int palette_size)
{
	for (int i = 0; i < palette_size; i++) {
		output_palette[i * 3] = palette[i].r / 255.0f;
		output_palette[i * 3 + 1] = palette[i].g / 255.0f;
		output_palette[i * 3 + 2] = palette[i].b / 255.0f;
	}
}

void generateKey(const map *h, char key[64])
{
	int vals[16] = {0}; // valeurs par défaut si absentes
	int val;
	for (int i = 0; i < 16; i++) {
		if (map_get(&val, *h, &i)) {
			vals[i] = val;
		}
	}

	snprintf(key, 64,
			 "%d,%d,%d,%d,%d,%d,%d,%d,"
			 "%d,%d,%d,%d,%d,%d,%d,%d",
			 vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8], vals[9], vals[10],
			 vals[11], vals[12], vals[13], vals[14], vals[15]);
}

typedef struct Couple_array {
	Couple *couples;
	size_t size;
} Couple_array;
static map best_couples_map = NULL;
void find_best_couple(const map *histo, const ColorPalette *palette, int palette_size, list *best_couples)
{
	if (best_couples_map == NULL) {
		best_couples_map = map_init(sizeof(char[64]), sizeof(Couple_array), string_cmp);
	}

	char key[64];
	generateKey(histo, key);

	list_clear(*best_couples);

	// --- cache hit ---
	if (map_contains(best_couples_map, key)) {
		Couple_array best_couples_array;
		map_get(&best_couples_array, best_couples_map, key);
		list_add_all(*best_couples, best_couples_array.couples, best_couples_array.size);
		return;
	}

	// --- sinon, on calcule ---
	double dm = DBL_MAX;
	for (int i = 0; i < 15; i++) {
		for (int j = i + 1; j < 16; j++) {
			double d = 0;
			unsigned int *current_key = map_first(*histo);
			int n = 0;

			while (current_key != NULL) {
				map_get(&n, *histo, current_key);
				double d1 = distance_between_colors_index(*current_key, i, palette);
				double d2 = distance_between_colors_index(*current_key, j, palette);
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

	// --- mettre à jour le cache ---
	Couple_array best_couples_array;
	best_couples_array.size = list_size(*best_couples);
	best_couples_array.couples = malloc(best_couples_array.size * sizeof(Couple));
	list_copy_to_array(best_couples_array.couples, *best_couples);
	map_put(best_couples_map, key, &best_couples_array);
}

void free_best_couples_map()
{
	if (best_couples_map != NULL) {
		unsigned int *first_key = map_first(best_couples_map);
		unsigned int *current_key = first_key;
		while (current_key != NULL) {
			// printf("Freeing key %s\n", (char *)current_key);
			Couple_array couple_array;
			map_get(&couple_array, best_couples_map, current_key);
			free(couple_array.couples);
			current_key = map_higher(best_couples_map, current_key);
		}
		map_destroy(best_couples_map);
	}
}