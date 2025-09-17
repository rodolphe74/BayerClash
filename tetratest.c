#include "useful.h"
#include "matrix.h"
#include <stdio.h>
#include <string.h>
#include <tetrapal.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <exoquant.h>



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

void dither_image(const float *input, const int image_width, const int image_height, unsigned char *output,
				  const float *palette, const int palette_size)
{
	// Candidate arrays
	int candidates[4];
	float weights[4];

	memset(candidates, 0, sizeof(candidates));
	memset(weights, 0, sizeof(weights));

	// Triangulate the palette
	Tetrapal *tetrapal = tetrapal_new(palette, palette_size);

	// Iterate over pixels in the input image
	for (int y = 0; y < image_height; y++) {
		for (int x = 0; x < image_width; x++) {
			// Get the current pixel from the input buffer
			const int image_index = x + y * image_width;
			const float *pixel = &input[image_index * 3];

			// Interpolate within the triangulation to get the candidates for the current pixel and their weights
			tetrapal_interpolate(tetrapal, pixel, candidates, weights);

			// Sort the candidates by luminance
			sort_by_luminance(candidates, weights, palette);

			// Use the value in the threshold matrix to select a candidate
			const double threshold = bayer_matrix_4x4[y % 4][x % 4];
			// const double threshold = bayer_matrix_8x8[y % 8][x % 8];
			double sum = 0.0;

			// Accumulate the sum of weights until we pass the threshold
			for (int i = 0; i < 4; i++) {
				sum += weights[i];

				if (threshold < sum) {
					output[image_index] = candidates[i];
					break;
				}
			}
		}
	}

	// Remember to free the triangulation data
	tetrapal_free(tetrapal);
}


int main(int argc, char *argv[])
{
	char filename[] = "/Users/rodoc/develop/BayerClash/samples/original.png";
	int width, height, channels;
	unsigned char *original_image = stbi_load(filename, &width, &height, &channels, COLOR_COMP);
	if (!original_image) {
		printf("Erreur: Impossible de charger l'image d'entrée '%s'. Vérifiez le chemin ou le format.\n", filename);
		return EXIT_FAILURE;
	}

	printf("Image chargée: %s (%dx%d pixels, %d canaux d'origine)\n", filename, width, height, channels);

	// find palette
	unsigned char palette[PALETTE_SIZE * 4];
	exq_data *pExqPalette;
	pExqPalette = exq_init();
	exq_feed(pExqPalette, original_image, width * height);
	exq_quantize_hq(pExqPalette, PALETTE_SIZE);
	exq_get_palette(pExqPalette, palette, PALETTE_SIZE);
	exq_free(pExqPalette);

	float *float_image = malloc(width * height * 3 * sizeof(float));
	convert_rgba_to_float(original_image, width, height, float_image);

	float *float_palette = malloc(PALETTE_SIZE * 3 * sizeof(float));
	memset(float_palette, 0, PALETTE_SIZE * 3 * sizeof(float));
	convert_palette_to_float(palette, PALETTE_SIZE, float_palette);

	output_palette(float_palette, PALETTE_SIZE, "palette_exoquant.png");

	unsigned char *output_image_indexed = malloc(width * height * sizeof(char));
	dither_image(float_image, width, height, output_image_indexed, float_palette, PALETTE_SIZE);

	char *output_image = malloc(width * height * COLOR_COMP * sizeof(char));
	convert_indexed_to_rgba((unsigned char *)output_image_indexed, width, height, palette, PALETTE_SIZE,
							(unsigned char *)output_image);

	stbi_write_png("output_indexed.png", width, height, COLOR_COMP,
				   output_image, width * COLOR_COMP);

	free(original_image);
	free(float_palette);
	free(float_image);
	free(output_image_indexed);
	free(output_image);

	return 0;
}
