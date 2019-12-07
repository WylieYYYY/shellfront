#include "internal.h"
#include "shellfront.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_util() {
	// struct err_state define_error(char *msg)
	struct err_state state = define_error("Error occured");
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Error occured") == 0);
	// int parse_size_str(char *size, long *x, long *y, char *delim)
	long lx, ly;
	assert(!parse_size_str("0x0", &lx, &ly));
	assert(!parse_size_str("0x1", &lx, &ly));
	assert(!parse_size_str("1x0", &lx, &ly));
	assert(!parse_size_str("11", &lx, &ly));
	assert(!parse_size_str("1xx1", &lx, &ly));
	assert(!parse_size_str("1yx1", &lx, &ly));
	assert(parse_size_str("1x1", &lx, &ly));
	assert(parse_size_str("1x2", &lx, &ly));
	assert(lx == 1 && ly == 2);
	// int parse_loc_str(char *size, int *x, int *y, char *delim)
	int ix, iy;
	assert(!parse_loc_str("-1,-1", &ix, &iy));
	assert(!parse_loc_str("-1,0", &ix, &iy));
	assert(!parse_loc_str("0,-1", &ix, &iy));
	assert(!parse_loc_str("00", &ix, &iy));
	assert(!parse_loc_str("0,,0", &ix, &iy));
	assert(!parse_loc_str("0`,0", &ix, &iy));
	assert(parse_loc_str("0,0", &ix, &iy));
	assert(parse_loc_str("3,4", &ix, &iy));
	assert(ix == 3 && iy == 4);
	// unsigned long hash(char *str)
	assert(hash("hi") == 5863446);
	// char *sxprintf(char *fmt, ...)
	char *output_str = sxprintf("%ix%s", 123, "d");
	assert(strcmp(output_str, "123xd") == 0);
	free(output_str);
}
