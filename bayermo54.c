#include "useful.h"
#include "thomson.h"
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
#define BLUE_NOISE_GENERATOR_IMPLEMENTATION
#include <blue_noise_generator.h>

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
	Color err1[322];
	Color err2[322];
	Color temp[322];
	memset(err1, 0, sizeof(err1));
	memset(err2, 0, sizeof(err2));
	int q[8];
	map histo = map_init(sizeof(int), sizeof(int), int_cmp);
	int candidates[4];
	float weights[4];
	memset(candidates, 0, sizeof(candidates));
	memset(weights, 0, sizeof(weights));
	float pixel[3];
	float float_palette[PALETTE_SIZE * 3];
	float float_mo5_palette[PALETTE_SIZE * 3];

	// matrice blue noise
	unsigned int buffer[4096];
	double matrix[64][64];
	blue_noise_generator_create_void_and_cluster(buffer, 64, 64);
	for (int y = 0; y < 64; y++) {
		for (int x = 0; x < 64; x++) {
			matrix[y][x] = buffer[y * 64 + x] / 4096.0;
		}
	}

	// initialisation des palettes lineaires
	ColorPalette mo5_lineare_palette[16] = {};
	for (int i = 0; i < PALETTE_SIZE; i++) {
		Color c = {mo5_palette[i].r / 255.0f, mo5_palette[i].g / 255.0f, mo5_palette[i].b / 255.0f};
		c = srgb_to_linear(c);
		mo5_lineare_palette[i].r = c.r * 255.0f;
		mo5_lineare_palette[i].g = c.g * 255.0f;
		mo5_lineare_palette[i].b = c.b * 255.0f; 
		mo5_lineare_palette[i].thomson_idx = mo5_palette[i].thomson_idx;
		float_mo5_palette[i * 3] = mo5_lineare_palette[i].r / 255.0f;
		float_mo5_palette[i * 3 + 1] = mo5_lineare_palette[i].g / 255.0f;
		float_mo5_palette[i * 3 + 2] = mo5_lineare_palette[i].b / 255.0f;
	}

	// Triangulation de la palette
	Tetrapal *tetrapal = tetrapal_new(float_mo5_palette, PALETTE_SIZE);
	// unsigned char *output_tetra_indexed = malloc(THOMSON_SCREEN_W * THOMSON_SCREEN_H * COLOR_COMP * sizeof(char));

	for (int y = 0; y < THOMSON_SCREEN_H; y++) {

		// Décalage des lignes d'erreurs
		memcpy(temp, err1, sizeof(err1));
		memcpy(err1, err2, sizeof(err1));
		memcpy(err2, temp, sizeof(err2));
		memset(err2, 0, sizeof(err2));

		for (int x = 0; x < THOMSON_SCREEN_W; x+=8) {
			memset(q, 0, sizeof(q));
			map_clear(histo);
			int i = 0;
			for (int z = x; z <= x + 7; z++) {
				Color p = color_add(get_average_pixel(original_image, width, height, z, y), err1[z + 1]);
				pixel[0] = p.r;
				pixel[1] = p.g;
				pixel[2] = p.b;
				tetrapal_interpolate(tetrapal, pixel, candidates, weights);
				sort_by_luminance(candidates, weights, float_mo5_palette);
				const double threshold = matrix[y % 64][z % 64];
				int image_index = y * width + z;
				double sum = 0.0;
				int c = 0;
				for (int i = 0; i < 4; i++) {
					sum += weights[i];
					if (threshold < sum) {
						c  = candidates[i];
						break;
					}
				}
				int val = get_or_default(histo, c, 0);
				val++;
				map_put(histo, &c, &val);
				q[i++] = c;
			}

			unsigned int *first_key, *last_key, *current_key;
			first_key = map_first(histo);
			last_key = map_last(histo);
			current_key = first_key;
			int c1 = -1, c2 = -1;
			while (current_key != NULL) {
				if (c1 == -1)
					c1 = *current_key;
				else if (c2 == -1)
					c2 = *current_key;
				else {
					c1 = -1; // il y a plus de 2 couleurs
					break;
				}
				current_key = map_higher(histo, current_key);
			}

			// display_histo(&histo);
			if (c1 != -1) {
				// 2 couleurs ou moins
				c2 = (c2 != -1) ? c2 : c1;
			} else {
				// Plus de 2 couleurs

				// cas simple on prend les 2 couleurs les plus frequentes
				// Couple two_most = find_two_most_frequent(&histo);
				// c1 = two_most.c1;
				// c2 = two_most.c2;

				// cas meilleur couples
				list best_couples;
				best_couples = list_init(sizeof(Couple));
				find_best_couple(&histo, mo5_lineare_palette, PALETTE_SIZE, &best_couples);

				if (list_size(best_couples) == 1) {
					Couple best;
					list_get_at(&best, best_couples, 0);
					c1 = best.c1;
					c2 = best.c2;
				} else {
					double dm = DBL_MAX;
					for (int i = 0; i < list_size(best_couples); i++) {
						Couple couple;
						list_get_at(&couple, best_couples, i);
						double d = 0;
						for (int k = 0; k < 8; k++) {
							int qk = q[k];
							int p = distance_between_colors(qk, couple.c1, mo5_lineare_palette) <
											distance_between_colors(qk, couple.c2, mo5_lineare_palette)
										? couple.c1
										: couple.c2;
							Color e = color_mul(
								color_sub(fromColorPalette(mo5_lineare_palette[qk]), fromColorPalette(mo5_lineare_palette[p])), COEF);
							// Color z = color_add(get_average_pixel(linearized_image, width, height, x + k, y + 1), e);
							Color z = color_add(get_average_pixel(original_image, width, height, x + k, y + 1), e);
							d += clamp_deviation(z.r) + clamp_deviation(z.g) + clamp_deviation(z.b);
						}
						if (d <= dm) {
							dm = d;
							c1 = couple.c1;
							c2 = couple.c2;
						}
					}
				}
				list_destroy(best_couples);
			}
			for (int k = 0; k < 8; k++) {
				int z = x + k;
				// couleurs exacte après matrice
				int qk = q[k];
				// choix de la couleur du couple minimisant la distance
				int chosen = distance_between_colors(qk, c1, mo5_lineare_palette) < distance_between_colors(qk, c2, mo5_lineare_palette)
								 ? c1
								 : c2;
				// différence
				Color d = color_sub(fromColorPalette(mo5_lineare_palette[qk]), fromColorPalette(mo5_lineare_palette[chosen]));
				// repousse l'erreur sur le pixel du dessous
				err2[z + 1] = color_add(err2[z + 1], color_mul(d, COEF));
				pset(output_image, z, y, mo5_palette, (chosen == c1) ? (c1) : (c2), THOMSON_SCREEN_W, THOMSON_SCREEN_H);
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

	// version clash thomson
	stbi_write_png("output_mo5.png", THOMSON_SCREEN_W, THOMSON_SCREEN_H, COLOR_COMP, output_image,
				   THOMSON_SCREEN_W * COLOR_COMP);

	free(original_image);
	free(lineare_image);
	free(output_image);
	// free(output_tetra_indexed);

	return 0;
}