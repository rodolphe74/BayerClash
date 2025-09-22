#include "useful.h"
#include "matrix.h"
#include "thomson.h"
#include "comparator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <map.h>
#include <list.h>
#include <float.h>

#define COEF 0.6f

// int string_cmp(const void *a, const void *b)
// {
// 	return strcmp(*(const char **)a, *(const char **)b);
// }

// int int_cmp(const void *a, const void *b)
// {
// 	int ia = *(const int *)a;
// 	int ib = *(const int *)b;
// 	return (ia > ib) - (ia < ib); // Retourne 1, 0 ou -1
// }

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
	// char filename[] = "/Users/rodoc/develop/BayerClash/samples/demo.jpg";
	char *filename = argv[1];
	int width, height, channels;
	unsigned char *original_image = stbi_load(filename, &width, &height, &channels, COLOR_COMP);
	if (!original_image) {
		printf("Erreur: Impossible de charger l'image d'entrée '%s'. Vérifiez le chemin ou le format.\n", filename);
		return EXIT_FAILURE;
	}

	unsigned char *linearized_image = linearizeImage(original_image, width, height);
	stbi_write_png("linearized.png", width, height, COLOR_COMP,
				   linearized_image, width * COLOR_COMP);

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

	for (int y = 0; y < 200; y++) {

		// Décalage des lignes d'erreurs
		memcpy(temp, err1, sizeof(err1));
		memcpy(err1, err2, sizeof(err1));
		memcpy(err2, temp, sizeof(err2));
		memset(err2, 0, sizeof(err2));

		for (int x = 0; x < 320; x+=8) {
			memset(q, 0, sizeof(q));
			map_clear(histo);
			int i = 0;
			for (int z = x; z <= x + 7; z++) {
				float d = bayer_matrix_8x8[y % 8][z % 8];
				Color p = color_add(get_average_pixel(linearized_image, width, height, z, y), err1[z + 1]);
				int c = ((p.r > d) ? 1 : 0) + ((p.g > d) ? 2 : 0) + ((p.b > d) ? 4 : 0);
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
				list best_couples;
				best_couples = list_init(sizeof(Couple));
				find_best_couple(&histo, mo5_palette, PALETTE_SIZE, &best_couples);
				// display_couples(&best_couples);

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
							int p = distance_between_colors(qk, couple.c1, mo5_palette) <
											distance_between_colors(qk, couple.c2, mo5_palette)
										? couple.c1
										: couple.c2;
							Color e = color_mul(
								color_sub(fromColorPalette(mo5_palette[qk]), fromColorPalette(mo5_palette[p])), COEF);
							Color z = color_add(get_average_pixel(linearized_image, width, height, x + k, y + 1), e);
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
				// int chosen = palette.distanceBetween(qk, c1) < palette.distanceBetween(qk, c2) ? c1 : c2;
				int chosen = distance_between_colors(qk, c1, mo5_palette) < distance_between_colors(qk, c2, mo5_palette)
								 ? c1
								 : c2;
				// différence
				Color d = color_sub(fromColorPalette(mo5_palette[qk]), fromColorPalette(mo5_palette[chosen]));
				// repousse l'erreur sur le pixel du dessous
				err2[z + 1] = color_add(err2[z + 1], color_mul(d, COEF));
				pset_lineare(output_image, z, y, mo5_palette, (chosen == c1) ? (c1) : (c2), THOMSON_SCREEN_W, THOMSON_SCREEN_H);
			}
		}
	}
	map_destroy(histo);
	free_best_couples_map();

	stbi_write_png("output_mo5.png", THOMSON_SCREEN_W, THOMSON_SCREEN_H, COLOR_COMP, output_image,
				   THOMSON_SCREEN_W * COLOR_COMP);

	free(original_image);
	free(linearized_image);
	free(output_image);

	return 0;
}