#include "shellfront.h"

#include <glib.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNIT_TEST
	char *mock_realpath(const char *path, char *resolved_path);
	#define realpath(x,y) mock_realpath(x,y)
	void mock_g_clear_error(GError **err);
	#define g_clear_error(x) mock_g_clear_error(x)
#endif

struct err_state define_error(char *msg) {
	struct err_state err = { .has_error = 1 };
	strcpy(err.errmsg, msg);
	return err;
}

int _shellfront_parse_coordinate(char *size, long *left,
	long *right, long min, char *delim) {
	char *cp = strdup(size);
	char *left_str = strsep(&cp, delim);
	// no delimiter, pointer moved to NULL
	if (cp == NULL) {
		free(left_str);
		return 0;
	}
	// if entire string is a number, pointer moved to the end
	int success = 1;
	char *strtol_end;
	*left = strtol(left_str, &strtol_end, 10);
	if (*strtol_end != '\0') success = 0;
	*right = strtol(cp, &strtol_end, 10);
	if (*strtol_end != '\0') success = 0;
	free(left_str);
	// validate result
	return *left >= min && *right >= min && success;
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

struct err_state _shellfront_gerror_to_err_state(GError *gerror) {
	struct err_state state = { .has_error = 0, .errmsg = "" };
	if (gerror != NULL) {
		state.has_error = gerror->code;
		strcpy(state.errmsg, gerror->message);
	}
	g_clear_error(&gerror);
	return state;
}
