#include "comparator.h"
#include <string.h>

// int string_cmp(const void *a, const void *b)
// {
// 	return strcmp(*(const char **)a, *(const char **)b);
// }

int string_cmp(const void *a, const void *b)
{
	return strcmp((const char *)a, (const char *)b);
}

int int_cmp(const void *a, const void *b)
{
	int ia = *(const int *)a;
	int ib = *(const int *)b;
	return (ia > ib) - (ia < ib); // Retourne 1, 0 ou -1
}
