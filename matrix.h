#ifndef MATRIX_H
#define MATRIX_H

static const double bayer_matrix_4x4[4][4] = {{0.0 / 16.0, 12.0 / 16.0, 3.0 / 16.0, 15.0 / 16.0},
											  {8.0 / 16.0, 4.0 / 16.0, 11.0 / 16.0, 7.0 / 16.0},
											  {2.0 / 16.0, 14.0 / 16.0, 1.0 / 16.0, 13.0 / 16.0},
											  {10.0 / 16.0, 6.0 / 16.0, 9.0 / 16.0, 5.0 / 16.0}};

static const double bayer_matrix_4x4_low[4][4] = {{4.0 / 8.0, 2.0 / 8.0, 7.0 / 8.0, 5.0 / 8.0},
											  {3.0 / 8.0, 1.0 / 8.0, 8.0 / 8.0, 6.0 / 8.0},
											  {7.0 / 8.0, 5.0 / 8.0, 4.0 / 8.0, 2.0 / 8.0},
											  {8.0 / 8.0, 6.0 / 8.0, 3.0 / 8.0, 1.0 / 8.0}};

double bayer_matrix_4x4_low_2[4][4] = {{7.0 / 16.0, 13.0 / 16.0, 11.0 / 16.0, 4.0 / 16.0},
									   {12.0 / 16.0, 16.0 / 16.0, 14.0 / 16.0, 8.0 / 16.0},
									   {10.0 / 16.0, 15.0 / 16.0, 6.0 / 16.0, 2.0 / 16.0},
									   {5.0 / 16.0, 9.0 / 16.0, 3.0 / 16.0, 1.0 / 16.0}};

double clash_matrix[4][4] = {{0.0625, 0.5625, 0.1875, 0.6875},
							 {0.8125, 0.3125, 0.9375, 0.4375},
							 {0.25, 0.75, 0.125, 0.625},
							 {1.0, 0.5, 0.875, 0.375}};

static const double bayer_matrix_8x8[8][8] = {
    {0.015625, 0.265625, 0.203125, 0.453125, 0.062500, 0.312500, 0.250000, 0.500000},
    {0.515625, 0.765625, 0.703125, 0.953125, 0.562500, 0.812500, 0.750000, 1.000000},
    {0.140625, 0.390625, 0.078125, 0.328125, 0.187500, 0.437500, 0.125000, 0.375000},
    {0.640625, 0.890625, 0.578125, 0.828125, 0.687500, 0.937500, 0.625000, 0.875000},
    {0.046875, 0.296875, 0.234375, 0.484375, 0.031250, 0.281250, 0.218750, 0.468750},
    {0.546875, 0.796875, 0.734375, 0.984375, 0.531250, 0.781250, 0.718750, 0.968750},
    {0.171875, 0.421875, 0.109375, 0.359375, 0.156250, 0.406250, 0.093750, 0.343750},
    {0.671875, 0.921875, 0.609375, 0.859375, 0.656250, 0.906250, 0.593750, 0.843750}
};


double blue_noise_8x8[8][8] = {
    {0.015625, 0.515625, 0.140625, 0.640625, 0.046875, 0.546875, 0.171875, 0.671875},
    {0.765625, 0.265625, 0.890625, 0.390625, 0.796875, 0.296875, 0.921875, 0.421875},
    {0.203125, 0.703125, 0.078125, 0.578125, 0.234375, 0.734375, 0.109375, 0.609375},
    {0.953125, 0.453125, 0.828125, 0.328125, 0.984375, 0.484375, 0.859375, 0.359375},
    {0.062500, 0.562500, 0.187500, 0.687500, 0.031250, 0.531250, 0.156250, 0.656250},
    {0.812500, 0.312500, 0.937500, 0.437500, 0.781250, 0.281250, 0.906250, 0.406250},
    {0.250000, 0.750000, 0.125000, 0.625000, 0.218750, 0.718750, 0.093750, 0.593750},
    {1.000000, 0.500000, 0.875000, 0.375000, 0.968750, 0.468750, 0.843750, 0.343750}
};



// Matrice en rotation

int8_t matrix[] =
{
	0,  8,	 2,    10,
	12, 4,	 14,   6,
	3,  11,	 1,    9,
	15, 7,	 13,   5
};

int32_t repeat_matrix_size[] = { 1000, 1000 };
double _A = 4.0;
double _B = 3.0;
double _C = 5.0;

typedef struct _rectangle {
	int8_t *matrix;
	int32_t size[2];
} rectangle;

int32_t matrix_size[] = { 4, 4 };

int get_position(int32_t x, int32_t y, int32_t stride)
{
	return y * stride + x;
}

rectangle create_matrix(int8_t *base_matrix, int32_t xsize, int32_t ysize)
{
	int8_t *ret_matrix = calloc(xsize * ysize, sizeof(uint8_t));
	int32_t index = 0;

	for (int32_t y = 0; y < ysize; y++) {
		for (int32_t x = 0; x < xsize; x++) {
			int8_t val = base_matrix[index + (x % matrix_size[0])];
			ret_matrix[get_position(x, y, xsize)] = val;
		}

		index += matrix_size[0];
		if (index >= (matrix_size[1] * matrix_size[0])) {
			index = 0;
		}
	}
	
	rectangle rect;
	rect.matrix = ret_matrix;
	rect.size[0] = xsize;
	rect.size[1] = ysize;
	
	return rect;
}

rectangle rotate_matrix(rectangle rect, float a, float b, float c)
{

	int32_t around_x = rect.size[0] / 2;
	int32_t around_y = rect.size[1] / 2;
	int8_t *new_matrix = calloc(rect.size[0] * rect.size[1], sizeof(uint8_t));

	memset(new_matrix, -1, rect.size[0] * rect.size[1]);

	for (int32_t y = 0; y < rect.size[1]; y++) {
		for (int32_t x = 0; x < rect.size[0]; x++) {
			if (rect.matrix[get_position(x, y, rect.size[0])] >= 0) {

				int32_t new_x = (int) round((a / c) * (x - around_x) - (b / c) * (y - around_y) + around_x);
				int32_t new_y = rect.size[1] - (int) round((b / c) * (x - around_x) + (a / c) * (y - around_y) + around_y);

				if (new_x < rect.size[0] && new_y < rect.size[1] && new_x >= 0 && new_y >= 0) {
					new_matrix[get_position(new_x, new_y, rect.size[0])] = rect.matrix[get_position(x, y, rect.size[0])];
				}
			}
		}
	}

	rectangle ret_rect;
	ret_rect.matrix = new_matrix;
	ret_rect.size[0] = rect.size[0];
	ret_rect.size[1] = rect.size[1];
	return ret_rect;
}


rectangle find_max_rectangle_in_matrix(rectangle rect)
{
	// find upper and bottom line
	int bottom = 0;
	int top = -1;

	for (int y = 0; y < rect.size[1] ; y++) {
		int val_bottom = 0;
		int full_line = 0;
		for (int x = 0; x < rect.size[0]; x++) {
			if (rect.matrix[get_position(x, y, rect.size[0])] >= 0) {
				full_line++;
			}
		}

		if (full_line == rect.size[0] && top == -1) {
			top = y;
		}

		if (full_line == rect.size[0] && y > bottom) {
			bottom = y;
		}
	}

	uint32_t size = bottom - top;
	int8_t *rect_matrix = calloc(size * rect.size[0], sizeof(int8_t));
	int line = 0;

	for (int y = top; y < bottom; y++) {
		for (int x = 0; x < rect.size[0]; x++) {
			rect_matrix[get_position(x, line, rect.size[0])] = rect.matrix[get_position(x, y, rect.size[0])];
		}
		line++;
	}

	rectangle r;
	r.matrix = rect_matrix;
	r.size[0] = rect.size[0];
	r.size[1] = size;
	return r;
}

void create_rotated_matrix(rectangle *rect)
{
	rectangle repeat_matrix = create_matrix(matrix, repeat_matrix_size[0], repeat_matrix_size[1]);
	rectangle rotated_matrix = rotate_matrix(repeat_matrix, _A, _B, _C);
	*rect = find_max_rectangle_in_matrix(rotated_matrix);
}

#endif