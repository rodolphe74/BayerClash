#include "useful.h"
#include "matrix.h"
#include "thomson.h"
#include "imquantize.h"
#include "k7.h"
#include "comparator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <map.h>
#include <list.h>
#include <float.h>
#include <tetrapal.h>
#include <exoquant.h>

#define COEF 0.6f

// resize avec imagemagick
// magick ../samples/owl2.png -resize 320x200 -background black -gravity center -extent 320x200 crop.png


int get_or_default(map histo, int key, int default_value)
{
	int value;
	if (map_get(&value, histo, &key)) {
		return value;
	} else {
		return default_value;
	}
}

void sort_by_luminance(int *candidates, float *weights, const float *palette)
{
	// Simple bubble sort
	for (int i = 0; i < 4 - 1; i++) {
		for (int j = 0; j < 4 - i - 1; j++) {
			// Calculate luminance for candidate j
			const float *color_j = &palette[candidates[j] * 3];
			const float luminance_j = 0.2126f * color_j[0] + 0.7152f * color_j[1] + 0.0722f * color_j[2];

			// Calculate luminance for candidate j+1
			const float *color_j1 = &palette[candidates[j + 1] * 3];
			const float luminance_j1 = 0.2126f * color_j1[0] + 0.7152f * color_j1[1] + 0.0722f * color_j1[2];

			// Swap if out of order
			if (luminance_j > luminance_j1) {
				// Swap candidates
				int temp_candidate = candidates[j];
				candidates[j] = candidates[j + 1];
				candidates[j + 1] = temp_candidate;

				// Swap weights
				float temp_weight = weights[j];
				weights[j] = weights[j + 1];
				weights[j + 1] = temp_weight;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	// char filename[] = "/Users/rodoc/develop/BayerClash/samples/bw.png";
	char *filename = argv[1];
	int width, height, channels;
	unsigned char *original_image = stbi_load(filename, &width, &height, &channels, COLOR_COMP);
	if (!original_image) {
		printf("Erreur: Impossible de charger l'image d'entrée '%s'. Vérifiez le chemin ou le format.\n", filename);
		return EXIT_FAILURE;
	}

	unsigned char *lineare_image = malloc(width * height * COLOR_COMP);
	convert_rgba_to_lineare(original_image, width, height, lineare_image);
	// stbi_write_png("output_lineare.png", width, height, COLOR_COMP, lineare_image, width * COLOR_COMP);

	unsigned char *output_image = malloc(THOMSON_SCREEN_W * THOMSON_SCREEN_H * COLOR_COMP * sizeof(unsigned char));

	init_thomson_palette();
	init_mo5_palette();
	// Color err1[322];
	// Color err2[322];
	Color temp[322];
	// memset(err1, 0, sizeof(err1));
	// memset(err2, 0, sizeof(err2));
	int q[8];
	map histo = map_init(sizeof(int), sizeof(int), int_cmp);
	int candidates[4];
	float weights[4];
	memset(candidates, 0, sizeof(candidates));
	memset(weights, 0, sizeof(weights));
	float pixel[3];
	float float_palette[PALETTE_SIZE * 3];
	float float_mo6_palette[PALETTE_SIZE * 3];

	// trouve une palette
	unsigned char palette[PALETTE_SIZE * 4];
	// exq_data *pExqPalette;
	// pExqPalette = exq_init();
	// exq_feed(pExqPalette, original_image, width * height);
	// exq_quantize_hq(pExqPalette, PALETTE_SIZE);
	// exq_get_palette(pExqPalette, palette, PALETTE_SIZE);
	// exq_free(pExqPalette);

	// libimagequant quantizer
	quantize(original_image, width, height, COLOR_COMP, palette, PALETTE_SIZE);

	memset(float_palette, 0, PALETTE_SIZE * 3 * sizeof(float));
	convert_palette_to_float(palette, PALETTE_SIZE, float_palette);
	output_palette(float_palette, PALETTE_SIZE, "palette_exoquant.png");

	// Pour la recherche sur les 4096 couleurs du mo6
	ColorPalette mo6_palette[16] = {};
	// il faut reduire la palette exoquant aux nuances disponibles sur mo6
	reduce_exoquant_palette_to_mo6(palette, mo6_palette, PALETTE_SIZE);

	// initialisation des palettes lineaires
	ColorPalette mo6_lineare_palette[16] = {};
	for (int i = 0; i < PALETTE_SIZE; i++) {
		Color c = {mo6_palette[i].r / 255.0f, mo6_palette[i].g / 255.0f, mo6_palette[i].b / 255.0f};
		c = srgb_to_linear(c);
		mo6_lineare_palette[i].r = c.r * 255.0f;
		mo6_lineare_palette[i].g = c.g * 255.0f;
		mo6_lineare_palette[i].b = c.b * 255.0f; 
		mo6_lineare_palette[i].thomson_idx = mo6_palette[i].thomson_idx;
		float_mo6_palette[i * 3] = mo6_lineare_palette[i].r / 255.0f;
		float_mo6_palette[i * 3 + 1] = mo6_lineare_palette[i].g / 255.0f;
		float_mo6_palette[i * 3 + 2] = mo6_lineare_palette[i].b / 255.0f;
	}
	output_palette(float_mo6_palette, PALETTE_SIZE, "palette_mo6.png");

	// Triangulation de la palette
	Tetrapal *tetrapal = tetrapal_new(float_mo6_palette, PALETTE_SIZE);
	unsigned char *output_tetra_indexed = malloc(THOMSON_SCREEN_W * THOMSON_SCREEN_H * sizeof(char));

	Color err[322];
	for (int y = 0; y < THOMSON_SCREEN_H; y++) {
		memset(err, 0, sizeof(err));
		for (int x = 0; x < THOMSON_SCREEN_W; x+=8) {
			// creation de l'histogramme
			memset(q, 0, sizeof(q));
			map_clear(histo);
			int i = 0;
			for (int z = x; z <= x + 7; z++) {
				Color p = color_add(get_average_pixel(original_image, width, height, z, y), err[z]);
				pixel[0] = p.r;
				pixel[1] = p.g;
				pixel[2] = p.b;
				tetrapal_interpolate(tetrapal, pixel, candidates, weights);
				sort_by_luminance(candidates, weights, float_mo6_palette);
				const double threshold = blue_noise_8x8[y % 8][z % 8];
				int image_index = y * width + z;
				double sum = 0.0;
				int c = 0;
				for (int i = 0; i < 4; i++) {
					sum += weights[i];
					if (threshold < sum) {
						c = candidates[i];
						output_tetra_indexed[y * THOMSON_SCREEN_W + z] = c;
						break;
					}
				}
				int val = get_or_default(histo, c, 0);
				val++;
				map_put(histo, &c, &val);
				q[i++] = c;
			}

			// recherche du meilleur couple par rapport a l'histo
			list best_couples;
			best_couples = list_init(sizeof(Couple));
			find_best_couple(&histo, mo6_lineare_palette, PALETTE_SIZE, &best_couples);
			int c1 = -1, c2 = -1;
			Couple best;
			list_get_at(&best, best_couples, 0);
			c1 = best.c1;
			c2 = best.c2;

			// printf("couple %d,%d=%d,%d\n", x,y,c1,c2);

			// dithering avec le meilleur couple
			for (int k = 0; k < 8; k++) {
				int z = x + k;
				int r = q[k];
				int p = distance_between_colors(r, c1, mo6_lineare_palette) < distance_between_colors(r, c2, mo6_lineare_palette) ? c1 : c2;
				Color color_r = fromColorPalette(mo6_lineare_palette[r]);
				Color color_p = fromColorPalette(mo6_lineare_palette[p]);
				Color color_d = color_sub(color_r, color_p);
				double ATT = 0.9;
				err[z] = color_add_att(err[z], color_d, ATT * .5);
				err[z - 1] = color_add_att(err[z - 1], color_d, ATT * .2);
				err[z + 1] = color_add_att(err[z + 1], color_d, ATT * .2);
				pset(output_image, z, y, mo6_palette, (p == c1) ? (c1) : (c2), THOMSON_SCREEN_W, THOMSON_SCREEN_H);
			}
		}
	}

	map_destroy(histo);
	free_best_couples_map();
	tetrapal_free(tetrapal);

	int ok = check_color_clash(output_image, THOMSON_SCREEN_W, THOMSON_SCREEN_H, 8);
	if (!ok) {
		printf("Contraintes color_clash non respectées !\n");
	}

	// version rgb
	unsigned char tetra_palette[PALETTE_SIZE * 4];\
	for (int i = 0; i < 16; i++) {
		tetra_palette[i * 4] = mo6_palette[i].r;
		tetra_palette[i * 4 + 1] = mo6_palette[i].g;
		tetra_palette[i * 4 + 2] = mo6_palette[i].b; 
		tetra_palette[i * 4 + 3] = 255;
	}
	unsigned char *output_tetra = malloc(THOMSON_SCREEN_W * THOMSON_SCREEN_H * COLOR_COMP);
	convert_indexed_to_rgba(output_tetra_indexed, THOMSON_SCREEN_W, THOMSON_SCREEN_H, tetra_palette, PALETTE_SIZE, output_tetra);
	stbi_write_png("output_tetra.png", THOMSON_SCREEN_W, THOMSON_SCREEN_H, COLOR_COMP, output_tetra, THOMSON_SCREEN_W * COLOR_COMP);
	free(output_tetra);

	// version clash thomson
	stbi_write_png("output_mo6.png", THOMSON_SCREEN_W, THOMSON_SCREEN_H, COLOR_COMP, output_image,
				   THOMSON_SCREEN_W * COLOR_COMP);

	// to-snap
	IntVector pixels, colors;
	init_vector(&pixels);
	init_vector(&colors);
	save_as_to_snap("OUTMO6", output_image, mo6_palette, &pixels, &colors);
	free_vector(&pixels);
	free_vector(&colors);

	// Ajout dans une k7
	FILE *fick7 = fopen("clash.k7", "wb");
	ajouterFichier(fick7, "OUTMO6.MAP");
	fclose(fick7);
	printf("clash.k7 créé\n");

	free(original_image);
	free(lineare_image);
	free(output_image);
	free(output_tetra_indexed);

	return 0;
}