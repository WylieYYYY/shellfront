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

int parse_size_str(char *size, long *x, long *y) {
	char *cp = strdup(size);
	char *x_str = strsep(&cp, "x");
	// no delimiter, pointer moved to the end
	if (cp == NULL) return 0;
	// if entire string is a number, pointer moved to the end
	char *strtol_end;
	*x = strtol(x_str, &strtol_end, 10);
	if (*strtol_end != '\0') return 0;
	*y = strtol(cp, &strtol_end, 10);
	if (*strtol_end != '\0') return 0;
	// size cannot be 0x0 or less
	return *x > 0 && *y > 0;
}
int parse_loc_str(char *loc, int *x, int *y) {
	char *cp = strdup(loc);
	char *x_str = strsep(&cp, ",");
	if (cp == NULL) return 0;
	char *strtol_end;
	*x = strtol(x_str, &strtol_end, 10);
	if (*strtol_end != '\0') return 0;
	*y = strtol(cp, &strtol_end, 10);
	if (*strtol_end != '\0') return 0;
	// location cannot be negative
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
