#include "shellfront.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct err_state define_error(char *msg) {
	struct err_state err;
	err.has_error = 1;
	strcpy(err.errmsg, msg);
	return err;
}

int parse_size_str(char *size, long *x, long *y, char *delim) {
	char *cp = strdup(size);
	*x = atol(strsep(&cp, delim));
	*y = atol(cp);
	// size cannot be 0x0 or less
	return !(*x < 1 || *y < 1);
}
int parse_loc_str(char *size, int *x, int *y, char *delim) {
	char *cp = strdup(size);
	*x = atoi(strsep(&cp, delim));
	*y = atoi(cp);
	// location must be positive
	return !(*x < 0 || *y < 0);
}
unsigned long hash(char *str) {
	unsigned long hash = 5381;
	unsigned char ch;
    while ((ch = (unsigned char)*str++)) hash = ((hash << 5) + hash) + ch;
    return hash;
}
char *sxprintf(char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	int size = vsnprintf(NULL, 0, fmt, argptr) + 1;
	char *str = malloc(size);
	va_start(argptr, fmt);
	vsnprintf(str, size, fmt, argptr);
	va_end(argptr);
	return str;
}
