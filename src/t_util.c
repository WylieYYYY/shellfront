#include <assert.h>
#include <stdlib.h>
#include <string.h>

int parse_size_str(char *size, long *x, long *y, char *delim);
int parse_loc_str(char *size, int *x, int *y, char *delim);
unsigned long hash(char *str);
char *sxprintf(char *fmt, ...);

int main(int argc, char **argv) {
	long lx, ly;
	assert(parse_size_str("0x0", &lx, &ly, "x") == 0);
	assert(parse_size_str("0x1", &lx, &ly, "x") == 0);
	assert(parse_size_str("1x0", &lx, &ly, "x") == 0);
	assert(parse_size_str("1x1", &lx, &ly, "x") != 0);
	
	int ix, iy;
	assert(parse_loc_str("-1,-1", &ix, &iy, ",") == 0);
	assert(parse_loc_str("-1,0", &ix, &iy, ",") == 0);
	assert(parse_loc_str("0,-1", &ix, &iy, ",") == 0);
	assert(parse_loc_str("0,0", &ix, &iy, ",") != 0);
	
	assert(hash("hi") == 5863446);
	
	char *output_str = sxprintf("%ix%s", 123, "d");
	assert(strcmp(output_str, "123xd") == 0);
	free(output_str);
	
	return 0;
}
