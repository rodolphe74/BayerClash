#ifndef GX2_REDUCE_H
#define GX2_REDUCE_H

#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

typedef struct {
	byte R;						///< Red
	byte G;						///< Green
	byte B;						///< Blue
} T_Components, T_Palette[256]; ///< A complete 256-entry RGB palette (768 bytes).

dword Round_div(dword numerator, dword divisor);
void Reduce_palette(short *used_colors, int nb_colors_asked, T_Palette palette, dword *color_usage);

#endif