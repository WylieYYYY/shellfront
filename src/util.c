#include "shellfront.h"

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNIT_TEST
	char *mock_realpath(const char *path, char *resolved_path);
	#define realpath(x,y) mock_realpath(x,y)
#endif

struct err_state define_error(char *msg) {
	struct err_state err;
	err.has_error = 1;
	strcpy(err.errmsg, msg);
	return err;
}

int _shellfront_parse_size_str(char *size, long *x, long *y) {
	char *cp = strdup(size);
	char *x_str = strsep(&cp, "x");
	// no delimiter, pointer moved to NULL
	if (cp == NULL) {
		free(x_str);
		return 0;
	}
	// if entire string is a number, pointer moved to the end
	int success = 1;
	char *strtol_end;
	*x = strtol(x_str, &strtol_end, 10);
	if (*strtol_end != '\0') success = 0;
	*y = strtol(cp, &strtol_end, 10);
	if (*strtol_end != '\0') success = 0;
	free(x_str);
	// size cannot be 0x0 or less
	return *x > 0 && *y > 0 && success;
}
int _shellfront_parse_loc_str(char *loc, int *x, int *y) {
	char *cp = strdup(loc);
	char *x_str = strsep(&cp, ",");
	if (cp == NULL) {
		free(x_str);
		return 0;
	}
	int success = 1;
	char *strtol_end;
	*x = strtol(x_str, &strtol_end, 10);
	if (*strtol_end != '\0') success = 0;
	*y = strtol(cp, &strtol_end, 10);
	if (*strtol_end != '\0') success = 0;
	free(x_str);
	// location cannot be negative
	return !(*x < 0 || *y < 0) && success;
}
unsigned long djb_hash(char *str) {
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
	str[size - 1] = '\0';
	va_end(argptr);
	va_start(argptr, fmt);
	vsnprintf(str, size, fmt, argptr);
	va_end(argptr);
	return str;
}
char *_shellfront_prepare_hashable(char *cmd, char **exe_name, int is_integrate) {
	char *cmd_dup = strdup(cmd);
	// paths with '/' does not use PATH variable and should be resolved to realpath
	char *cmd_file = strsep(&cmd_dup, " ");
	char *real_path = realpath(cmd_file, NULL);
	char *prepared = strchr(cmd_file, '/') == NULL? cmd_file : real_path;
	// need to allocate a new string to keep content and free
	if (cmd_dup == NULL) prepared = strdup(prepared);
	else prepared = sxprintf("%s %s", prepared, cmd_dup);
	free(cmd_file);
	if (is_integrate) *exe_name = strdup(basename(real_path));
	else *exe_name = strdup("shellfront");
	free(real_path);
	return prepared;
}
