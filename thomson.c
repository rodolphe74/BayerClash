#include "thomson.h"
#include <stdio.h>
#include <list.h>
#include <math.h>
#include <string.h>
#include <float.h>

ColorPalette red_255[16] = {{0, 0, 0, 0},	 {96, 0, 0, 1},	  {122, 0, 0, 2},  {142, 0, 0, 3},
							{158, 0, 0, 4},	 {170, 0, 0, 5},  {183, 0, 0, 6},  {193, 0, 0, 7},
							{204, 0, 0, 8},	 {211, 0, 0, 9},  {219, 0, 0, 10}, {226, 0, 0, 11},
							{234, 0, 0, 12}, {242, 0, 0, 13}, {249, 0, 0, 14}, {255, 0, 0, 15}};

// Green : 0 16 32 48 64 80 96 112 128 144 160 176 192 208 224 240
ColorPalette green_255[16] = {{0, 0, 0, 0},		{0, 96, 0, 16},	  {0, 122, 0, 32},	{0, 142, 0, 48},
							  {0, 158, 0, 64},	{0, 170, 0, 80},  {0, 183, 0, 96},	{0, 193, 0, 112},
							  {0, 204, 0, 128}, {0, 211, 0, 144}, {0, 219, 0, 160}, {0, 226, 0, 176},
							  {0, 234, 0, 192}, {0, 242, 0, 208}, {0, 249, 0, 224}, {0, 255, 0, 240}};

// Blue : 0 256 512 768 1024 1280 1536 1792 2048 2304 2560 2816 3072 3328 3584 3840
ColorPalette blue_255[16] = {{0, 0, 0, 0},		{0, 0, 96, 256},   {0, 0, 122, 512},  {0, 0, 142, 768},
							 {0, 0, 158, 1024}, {0, 0, 170, 1280}, {0, 0, 183, 1536}, {0, 0, 193, 1792},
							 {0, 0, 204, 2048}, {0, 0, 211, 2304}, {0, 0, 219, 2560}, {0, 0, 226, 2816},
							 {0, 0, 234, 3072}, {0, 0, 242, 3328}, {0, 0, 249, 3584}, {0, 0, 255, 3840}};

ColorPalette thomson_full_palette[4096];

// int mo5_palette_indexes[16] = {0, 15, 240, 255, 51, 204, 187, 255, 85, 170, 102, 153, 204, 51, 153, 255};
int mo5_palette_indexes[16] = {0, 15, 240, 255, 3840, 3855, 4080, 4095, 1911, 826, 931, 938, 2611, 2618, 3815, 123};
ColorPalette mo5_palette[16] = {};

void init_thomson_palette()
{
	int index = 0;
	for (int b = 0; b < 16; b++) {
		for (int g = 0; g < 16; g++) {
			for (int r = 0; r < 16; r++) {
				uint8_t r8 = red_255[r].r;
				uint8_t g8 = green_255[g].g;
				uint8_t b8 = blue_255[b].b;
				index = red_255[r].thomson_idx + green_255[g].thomson_idx + blue_255[b].thomson_idx;
				thomson_full_palette[index].r = r8;
				thomson_full_palette[index].g = g8;
				thomson_full_palette[index].b = b8;
				thomson_full_palette[index].thomson_idx = index;
				// printf("Index %4d : (r=%3d,g=%3d,b=%3d) (r8=%3d,g8=%3d,b8=%3d)\n", index, r, g, b, r8, g8, b8);
			}
		}
	}
}

void init_mo5_palette()
{
    for (int i = 0; i < 16; i++) {
        mo5_palette[i] = thomson_full_palette[mo5_palette_indexes[i]];
		mo5_palette[i].thomson_idx = mo5_palette_indexes[i];
    }
}



void reduce_palette_to_mo6_color_space(const ColorPalette *input_palette, ColorPalette *output_palette, int input_size)
{
	for (int i = 0; i < input_size; i++) {
		// ColorPalette mo6_color = output_palette[i];
		int best_index = 0;
		double best_distance = DBL_MAX;
		unsigned char r = input_palette[i].r;
		unsigned char g = input_palette[i].g;
		unsigned char b = input_palette[i].b;
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
		output_palette[i].thomson_idx = best_index;
	}
}





void init_vector(IntVector *vec)
{
	vec->size = 0;
	vec->capacity = 4;
	vec->data = (uint8_t *)malloc(vec->capacity * sizeof(uint8_t));
}

void push_back(IntVector *vec, uint8_t value)
{
	if (vec->size >= vec->capacity) {
		vec->capacity *= 2;
		void *tmp = realloc(vec->data, vec->capacity * sizeof(int));
		if (tmp == NULL) {
			vec->capacity /= 2;
			return;
		}
		vec->data = tmp;
	}
	vec->data[vec->size++] = value;
}

void free_vector(IntVector *vec)
{
	free(vec->data);
	vec->data = NULL;
	vec->size = vec->capacity = 0;
}

void transpose_data_map_40(int columns, int lines, IntVector *src, IntVector *target)
{
	uint8_t current;
	uint8_t zero = 0;

	// Le nombre de lignes doit être un multiple de 8
	// La hauteur est inscrite dans l'entêtte du map
	// sous la forme (map_40->lines - 1) / 8

	int padding = lines % 8;
	int add_line = 8 - padding;

	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < lines; y++) {
			// current = src.at(y * columns + x);
			current = src->data[y * columns + x];
			push_back(target, current);
		}

		if (padding)
			for (int y = 0; y < add_line; y++) push_back(target, zero);

		add_line = 8 - padding;
	}
}

int read_ahead(const IntVector *buffer_list, int idx)
{
	uint8_t current;
	uint8_t compare_to;

	// compare_to = buffer_list.at(idx);
	compare_to = buffer_list->data[idx];
	int repeat = 0;

	for (int i = idx + 1; i < buffer_list->size; i++) {
		// current = buffer_list.at(i);
		current = buffer_list->data[i];
		if (compare_to != current || repeat > 253) break;
		repeat++;
	}
	return repeat;
}

void write_segment(IntVector *target, const IntVector *buffer_list, int i, uint8_t seg_size)
{
	uint8_t current;
	uint8_t header[2];

	header[0] = 0;
	header[1] = seg_size;

	push_back(target, header[0]);
	push_back(target, header[1]);

	for (int j = i - seg_size; j < i; j++) {
		// current = buffer_list.at(j);
		current = buffer_list->data[j];
		push_back(target, current);
	}
}

void compress(IntVector *target, IntVector *buffer_list, int enclose)
{
	// Traitement du buffer;
	int i = 0;
	int seg = 0;
	uint8_t current;

	while (i < buffer_list->size) {
		int repeat = read_ahead(buffer_list, i);

		if (repeat == 0) {
			i += 1;
			seg++;

			if (seg > 254) {
				write_segment(target, buffer_list, i, seg);
				seg = 0;
			}
		} else {
			if (seg > 0) write_segment(target, buffer_list, i, (uint8_t)seg);

			i += (repeat + 1);
			seg = 0;

			uint8_t rep_count;
			rep_count = repeat + 1;
			push_back(target, rep_count);
			// current = buffer_list.at(i - repeat - 1);
			current = buffer_list->data[i - repeat - 1];
			push_back(target, current);
		}
	}

	// flush
	if (seg > 0) write_segment(target, buffer_list, i, seg);

	// cloture ?
	if (enclose) {
		uint8_t cloture[2] = {0, 0};
		push_back(target, cloture[0]);
		push_back(target, cloture[1]);
	}
}

int find_palette_index(int r, int g, int b, ColorPalette palette[PALETTE_SIZE])
{
	for (int i = 0; i < PALETTE_SIZE; i++)
		if (r == palette[i].r && g == palette[i].g && b == palette[i].b) return i;

	return 0; // ?
}

void clash_fragment_to_palette_indexed_bloc(const uint8_t *fragment, uint8_t *bloc, int blocSize, ColorPalette palette[PALETTE_SIZE])
{
	for (int i = 0; i < blocSize; i++) {
		ColorPalette c;
		c.r = fragment[i * COLOR_COMP];
		c.g = fragment[i * COLOR_COMP + 1];
		c.b = fragment[i * COLOR_COMP + 2];
		int idx = find_palette_index(c.r, c.g, c.b, palette);
		bloc[8 - 1 - i] = idx;
	}
}

int get_index_color_thomson_to(int back_index, int fore_index)
{
	// Palette thomson TO xyBVRBVR | x = 0 : fd pastel | y = 0 fo pastel
	// N,R,V,J,B,M,C,BL (fonce)
	// x,x,x,x,x,x,x,OR (pastel)

	// couleur > 7 = pastel
	int subst_back = (back_index > 7 ? 8 : 0);
	int subst_fore = (fore_index > 7 ? 8 : 0);
	uint8_t idx = (back_index > 7 ? 0 : 1) << 7 | (fore_index > 7 ? 0 : 1) << 6 | (fore_index - subst_fore) << 3 |
						(back_index - subst_back);

	return idx;
}

int get_index_color_thomson_mo(int back_index, int fore_index)
{
	// Palette thomson MO5/6 xBVRyBVR | x = 1 : fd pastel | y = 1 fo pastel
	// N,R,V,J,B,M,C,BL (fonce)
	// x,x,x,x,x,x,x,OR (pastel)

	// couleur > 7 = pastel
	uint8_t idx =
		(fore_index > 7 ? 1 : 0) << 7 | (fore_index) << 4 | (back_index > 7 ? 1 : 0) << 3 | (back_index);

	return idx;
}

void thomson_encode_bloc(uint8_t bloc[8], uint8_t thomson_bloc[3])
{
	// Conversion du bloc en valeur thomson to/mo
	// en sortie :
	// thomson_bloc[0] = forme
	// thomson_bloc[1] = couleurs format TO
	// thomson_bloc[2] = couleurs format MO
	// En basic, le format de la couleur est spécifié en fonction de la config TO/MO
	// En SNAP-TO, le format de la couleur est toujours TO

	// recherche des couleurs
	int fd = bloc[0];
	int fo = -1;
	int val = 0 /*, coul = 0*/;

	for (int i = 0; i < 8; i++)
		if (bloc[i] != fd) fo = bloc[i];

	// Calcul forme
	for (int i = 7; i >= 0; i--)
		if (bloc[i] == fo) val += pow(2, i);

	// Couleur MO / TO
	thomson_bloc[1] = get_index_color_thomson_to(fd, fo <= 0 ? 0 : fo);
	thomson_bloc[2] = get_index_color_thomson_mo(fd, fo <= 0 ? 0 : fo);

	thomson_bloc[0] = val;
}


void find_back_and_front(uint8_t bloc[8], uint8_t *back, uint8_t *front)
{
	uint8_t b = 0;
	uint8_t f = 0;
	uint8_t bc = 0, fc = 0;
	uint8_t count[255] = {0};
	for (int i = 0; i < 8; i++) {
		count[bloc[i]]++;
	}
	for (int i = 0; i < 255; i++) {
		if (count[i] > 0) {
			b = i;
			bc = count[i];
		}
	}

	for (int i = 0; i < 255; i++) {
		if (count[i] > 0 && i != b) {
			f = i;
			fc = count[i];
		}
	}
	// printf("b=%d  f=%d\n", b, f);
	if (bc > fc) {
		*back = b;
		*front = f;
	} else {
		*back = f;
		*front = b;
	}
}


void save_map_40_col(const char *filename, MAP_SEG *map_40, ColorPalette thomson_palette[PALETTE_SIZE])
{
	IntVector buffer_list, target_buffer_list;
	uint8_t current;

	FILE *fout;
	char map_filename[256];

	init_vector(&buffer_list);
	init_vector(&target_buffer_list);

	sprintf(map_filename, "%s.MAP", filename);
	if ((fout = fopen(map_filename, "wb")) == NULL) {
		fprintf(stderr, "Impossible d'ouvrir le fichier données en écriture\n");
		return;
	}

	transpose_data_map_40(map_40->columns, map_40->lines, &map_40->rama, &buffer_list);
	compress(&target_buffer_list, &buffer_list, 1);

	init_vector(&buffer_list);

	transpose_data_map_40(map_40->columns, map_40->lines, &map_40->ramb, &buffer_list);
	compress(&target_buffer_list, &buffer_list, 1);

	// Ecriture de l'entete
	uint16_t size = (uint16_t)target_buffer_list.size + 3 + 39;

	if (size % 2 == 1) {
		// Apparement, la taille doit être paire
		uint8_t zero = 0;
		push_back(&target_buffer_list, zero);
		size++;
	}

	uint8_t header[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	header[2] = size & 255;
	header[1] = (size >> 8) & 255;

	header[6] = map_40->columns - 1;
	header[7] = (map_40->lines - 1) / 8; // Le fichier map ne fonctionne que sur multiple de 8

	fwrite(header, sizeof(uint8_t), 8, fout);

	// Ecriture du buffer map compressé dans le fichier de sortie
	// cout << "ToSnap buffer size:" << target_buffer_list.size() << endl;
	for (int i = 0; i < target_buffer_list.size; i++) {
		// current = target_buffer_list.at(i);
		current = target_buffer_list.data[i];
		fwrite(&current, sizeof(uint8_t), 1, fout);
	}

	// Ecriture footer TO-SNAP
	uint8_t to_snap[40];

	memset(to_snap, 0, 39);
	to_snap[0] = 0; // 16 couleurs 40 colonnes
	to_snap[2] = 0; // tour de l'écran
	to_snap[4] = 0; // mode 3 console

	for (int i = 0; i < 16; i++) {
		// uint16_t thomson_palette_value = find_thomson_palette_index(palette[i].r, palette[i].g, palette[i].b, thomson_palette);
		uint16_t thomson_palette_value = thomson_palette[i].thomson_idx;
		printf(" rgb(%d,%d,%d) -> Thomson[%d]=%d\n", thomson_palette[i].r, thomson_palette[i].g, thomson_palette[i].b, i,
			   thomson_palette_value);
		to_snap[5 + i * 2] = (thomson_palette_value >> 8) & 255;
		to_snap[5 + i * 2 + 1] = thomson_palette_value & 255;
	}

	to_snap[37] = 0xA5;
	to_snap[38] = 0x5A;
	fwrite(to_snap, sizeof(uint8_t), 39, fout);

	// Ecriture du footer
	uint8_t footer[] = {0, 0, 0, 0, 0};

	footer[0] = 255;
	fwrite(footer, sizeof(uint8_t), 5, fout);

	fflush(fout);
	fclose(fout);

	//printf("TO-SNAP créé\n");

	free_vector(&buffer_list);
	free_vector(&target_buffer_list);
}

void save_as_to_snap(const char *name, const uint8_t *output_image_data, ColorPalette thomson_palette[PALETTE_SIZE], IntVector *pixels, IntVector *colors)
{
	MAP_SEG map_40;
	uint8_t b, f;
	init_vector(&map_40.rama);
	init_vector(&map_40.ramb);
	uint8_t *clash_fragment = malloc(8 * COLOR_COMP);
	if (!clash_fragment) return;
	uint8_t current_bloc[8];
	for (int y = 0; y < THOMSON_SCREEN_H; y++) {
		for (int x = 0; x < THOMSON_SCREEN_W; x += 8) {
			int length = x + 8 > THOMSON_SCREEN_W ? THOMSON_SCREEN_W - x : 8;
			memset(clash_fragment, 0, 8 * COLOR_COMP);
			for (int i = 0; i < length; i++) {
				int output_pixel_idx = (y * THOMSON_SCREEN_W + x + i) * COLOR_COMP;
				uint8_t r = output_image_data[output_pixel_idx];
				uint8_t g = output_image_data[output_pixel_idx + 1];
				uint8_t b = output_image_data[output_pixel_idx + 2];
				clash_fragment[i * COLOR_COMP] = r;
				clash_fragment[i * COLOR_COMP + 1] = g;
				clash_fragment[i * COLOR_COMP + 2] = b;
			}
			clash_fragment_to_palette_indexed_bloc(clash_fragment, current_bloc, 8, thomson_palette);
			uint8_t ret[3];
			thomson_encode_bloc(current_bloc, ret);
			push_back(&map_40.rama, ret[0]);
			push_back(&map_40.ramb, ret[1]);

			// MO5 pixels and colors
			find_back_and_front(current_bloc, &b, &f);
			uint8_t result = 0;
			for (int i = 0; i < 8; i++) {
				if (current_bloc[7 - i] == f) {
					result |= 1 << (7 - i);
				}
			}
			// en sortie les données pixels et forme (utils pour la sauvegarde MO5)
			push_back(colors, 16 * f + b);
			push_back(pixels, result);
		}
	}
	map_40.lines = THOMSON_SCREEN_H;
	map_40.columns = THOMSON_SCREEN_W / 8 + (THOMSON_SCREEN_W % 8 == 0 ? 0 : 1);
	save_map_40_col(name, &map_40, thomson_palette);

	free_vector(&map_40.rama);
	free_vector(&map_40.ramb);
}