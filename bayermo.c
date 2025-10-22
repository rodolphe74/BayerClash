// Choix de la machine
// #define MO5
#define MO6

// Choix du dithering
// #define BLUE_NOISE
// #define BAYER_4
// #define BAYER_4_LOW
// #define BAYER_8
#define R_SEQUENCE

// Choix de l'algorithme
// #define TETRAPAL
// #define N_CANDIDATES 4
#define KNOLL
#define N_CANDIDATES 32


#define ERROR_FACTOR 0.8
#define COEF 0.6f

#include "color.h"
#include "image.h"
#include "thomson.h"
#include "k7.h"
#include "gx2reduce.h"
#include "comparator.h"
#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map.h>
#include <list.h>
#include <float.h>

#ifdef BLUE_NOISE
#define BLUE_NOISE_GENERATOR_IMPLEMENTATION
#include <blue_noise_generator.h>
#endif

#ifdef TETRAPAL
#include <tetrapal.h>
#endif

#include <stb_image.h>
#include <stb_image_write.h>

// resize avec imagemagick
// magick ../samples/owl2.png -resize 320x200 -background black -gravity center -extent 320x200 crop.png

typedef struct Candidate {
	Color c;
	float w;
	int idx;
} Candidate;

int cmp_luminosity(const void *a, const void *b)
{
	const Candidate *color_a = (const Candidate *)a;
	const Candidate *color_b = (const Candidate *)b;

	const double luminance_a = 0.2126f * color_a->c.r + 0.7152f * color_a->c.g + 0.0722f * color_a->c.b;
	const double luminance_b = 0.2126f * color_b->c.r + 0.7152f * color_b->c.g + 0.0722f * color_b->c.b;

	if (luminance_a < luminance_b) {
		return -1;
	} else if (luminance_a > luminance_b) {
		return 1;
	}
	return 0;
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



int get_or_default(map histo, int key, int default_value)
{
	int value;
	if (map_get(&value, histo, &key)) {
		return value;
	} else {
		return default_value;
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

	// cherche une palette optimale
	init_thomson_palette();
	init_mo5_palette();

	// les palettes sur lesquelles l'algo de dithering va travailler
	ColorPalette palette_mo[PALETTE_SIZE];
	ColorPalette palette_mo_linear[PALETTE_SIZE];

#ifdef MO6
	ColorPalette palette[PALETTE_SIZE];
	ColorPalette palette_mo6[PALETTE_SIZE];
	ColorPalette palette_mo6_linear[PALETTE_SIZE];
	find_palette(original_image, width, height, palette, PALETTE_SIZE);
	output_palette(palette, PALETTE_SIZE, "palette.png");
	reduce_palette_to_mo6_color_space(palette, palette_mo6, PALETTE_SIZE);
	output_palette(palette_mo6, PALETTE_SIZE, "palette_mo6.png");

	// conversion de la palette dans l'espace lineaire
	linearize_palette(palette_mo6, palette_mo6_linear, PALETTE_SIZE);
	output_palette(palette_mo6_linear, PALETTE_SIZE, "palette_mo6_linear.png");
	copy_palette(palette_mo, palette_mo6, PALETTE_SIZE);
	copy_palette(palette_mo_linear, palette_mo6_linear, PALETTE_SIZE);
#endif

#ifdef MO5
	ColorPalette palette_mo5_linear[PALETTE_SIZE];
	output_palette(mo5_palette, PALETTE_SIZE, "palette_mo5.png");
	linearize_palette(mo5_palette, palette_mo5_linear, PALETTE_SIZE);
	output_palette(palette_mo5_linear, PALETTE_SIZE, "palette_mo5_linear.png");
	copy_palette(palette_mo, mo5_palette, PALETTE_SIZE);
	copy_palette(palette_mo_linear, palette_mo5_linear, PALETTE_SIZE);
#endif

#ifdef TETRAPAL
	float tetra_weights[N_CANDIDATES];
	int tetra_candidates[N_CANDIDATES];
	memset(tetra_weights, 0, sizeof(tetra_weights));
	memset(tetra_candidates, 0, sizeof(tetra_candidates));
	float palette_tetra[PALETTE_SIZE * 3];
	palette_to_floats(palette_mo_linear, palette_tetra, PALETTE_SIZE);
	Tetrapal *tetrapal = tetrapal_new(palette_tetra, PALETTE_SIZE);
#endif

#ifdef BLUE_NOISE
	unsigned int buffer[4096];
	double blue_noise_matrix[64][64];
	blue_noise_generator_create_void_and_cluster(buffer, 64, 64);
	for (int y = 0; y < 64; y++) {
		for (int x = 0; x < 64; x++) {
			blue_noise_matrix[y][x] = buffer[y * 64 + x] / 4096.0;
		}
	}
#endif

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
	Candidate candidates[N_CANDIDATES];

	for (int y = 0; y < THOMSON_SCREEN_H; y++) {

		// Décalage des lignes d'erreurs
		memcpy(temp, err1, sizeof(err1));
		memcpy(err1, err2, sizeof(err1));
		memcpy(err2, temp, sizeof(err2));
		memset(err2, 0, sizeof(err2));

		for (int x = 0; x < THOMSON_SCREEN_W; x += 8) {
			memset(q, 0, sizeof(q));
			map_clear(histo);
			int i = 0;
			for (int z = x; z <= x + 7; z++) {
				Color p = color_add(get_average_pixel_linear(original_image, width, height, z, y), err1[z + 1]);

#ifdef KNOLL
				Color goal = p;
				for (int i = 0; i < N_CANDIDATES; i++) {
					Color c;
					int idx = find_closest_color_from_palette(color_mul(goal, ERROR_FACTOR), palette_mo_linear,
															  PALETTE_SIZE, &c);
					candidates[i].idx = idx;
					candidates[i].c = c;
					goal = color_add(goal, color_sub(p, c));
				}
				qsort(candidates, N_CANDIDATES, sizeof(Candidate), cmp_luminosity);
#endif

#ifdef TETRAPAL
				float pixel[3] = {p.r, p.g, p.b};
				tetrapal_interpolate(tetrapal, pixel, tetra_candidates, tetra_weights);

				sort_by_luminance(tetra_candidates, tetra_weights, palette_tetra);
				for (int i = 0; i < N_CANDIDATES; i++) {
					int candidate_index = tetra_candidates[i];
					candidates[i].c =
						(Color){palette_tetra[candidate_index * 3], palette_tetra[candidate_index * 3 + 1],
								palette_tetra[candidate_index * 3 + 2]};
					candidates[i].w = tetra_weights[i];
					candidates[i].idx = candidate_index;
				}
#endif

#ifdef BLUE_NOISE
				const double threshold = blue_noise_matrix[y % 64][z % 64];
#endif

#ifdef BAYER_4
				const double threshold = bayer_matrix_4x4[y % 4][z % 4];
#endif

#ifdef BAYER_4_LOW
				const double threshold = bayer_matrix_4x4_low[y % 4][z % 4];
#endif

#ifdef BAYER_8
				const double threshold = bayer_matrix_8x8[y % 8][z % 8];
#endif

#ifdef R_SEQUENCE
				const double threshold = r_sequence(z, y);
#endif

				// int image_index = y * width + z;

#ifdef KNOLL
				int k = (int)round((threshold * (N_CANDIDATES - 1)));
				Candidate c = candidates[k];
#endif

#ifdef TETRAPAL
				double sum = 0.0;
				int c_idx = 0;
				Candidate c;
				for (int i = 0; i < N_CANDIDATES; i++) {
					sum += candidates[i].w;
					if (threshold < sum) {
						c = candidates[i];
						break;
					}
				}
#endif

				int val = get_or_default(histo, c.idx, 0);
				val++;
				map_put(histo, &c.idx, &val);
				q[i++] = c.idx;
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

			if (c1 != -1) {
				// 2 couleurs ou moins
				c2 = (c2 != -1) ? c2 : c1;
			} else {
				// Plus de 2 couleurs
				// recherche des meilleurs couples
				list best_couples;
				best_couples = list_init(sizeof(Couple));
				find_best_couple(&histo, palette_mo_linear, PALETTE_SIZE, &best_couples);

				if (list_size(best_couples) == 1) {
					// 1 seul meilleur couple
					Couple best;
					list_get_at(&best, best_couples, 0);
					c1 = best.c1;
					c2 = best.c2;
				} else {
					// plusieurs meilleurs couples - recherche du meilleur meuilleur couple
					double dm = DBL_MAX;
					for (int i = 0; i < list_size(best_couples); i++) {
						Couple couple;
						list_get_at(&couple, best_couples, i);
						double d = 0;
						for (int k = 0; k < 8; k++) {
							int qk = q[k];
							int p = distance_between_colors_index(qk, couple.c1, palette_mo_linear) <
											distance_between_colors_index(qk, couple.c2, palette_mo_linear)
										? couple.c1
										: couple.c2;
							Color e = color_mul(color_sub(fromColorPalette(palette_mo_linear[qk]),
														  fromColorPalette(palette_mo_linear[p])),
												COEF);
							// Color z = color_add(get_average_pixel(linearized_image, width, height, x + k, y + 1), e);
							Color z = color_add(get_average_pixel_linear(original_image, width, height, x + k, y + 1), e);
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
				int chosen = distance_between_colors_index(qk, c1, palette_mo_linear) <
									 distance_between_colors_index(qk, c2, palette_mo_linear)
								 ? c1
								 : c2;
				// différence entre couleur exacte et couleur color-clash affichée
				Color d =
					color_sub(fromColorPalette(palette_mo_linear[qk]), fromColorPalette(palette_mo_linear[chosen]));
				// repousse l'erreur sur le pixel du dessous
				err2[z + 1] = color_add(err2[z + 1], color_mul(d, COEF));
				pset(output_image, z, y, palette_mo, (chosen == c1) ? (c1) : (c2), THOMSON_SCREEN_W, THOMSON_SCREEN_H);
			}
		}
	}
	map_destroy(histo);
	free_best_couples_map();

#ifdef TETRAPAL
	tetrapal_free(tetrapal);
#endif


#ifdef MO6
	stbi_write_png("output_mo6.png", THOMSON_SCREEN_W, THOMSON_SCREEN_H, COLOR_COMP, output_image,
				   THOMSON_SCREEN_W * COLOR_COMP);
#endif

#ifdef MO5
	stbi_write_png("output_mo5.png", THOMSON_SCREEN_W, THOMSON_SCREEN_H, COLOR_COMP, output_image,
				   THOMSON_SCREEN_W * COLOR_COMP);
#endif

	// 	// to-snap
	IntVector pixels, colors;
	init_vector(&pixels);
	init_vector(&colors);
	save_as_to_snap("OUTMO6", output_image, palette_mo, &pixels, &colors);
	free_vector(&pixels);
	free_vector(&colors);

	// 	// Ajout dans une k7
	FILE *fick7 = fopen("clash.k7", "wb");
	ajouter_fichier(fick7, "OUTMO6.MAP");
	fclose(fick7);
	printf("clash.k7 créé\n");

	free(original_image);
	free(output_image);
	// 	// free(output_tetra_indexed);

	return 0;
}