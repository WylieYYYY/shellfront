#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct err_state define_error(char *msg);
int _shellfront_parse_size_str(char *size, long *x, long *y);
int _shellfront_parse_loc_str(char *loc, int *x, int *y);
unsigned long djb_hash(char *str);
char *sxprintf(char *fmt, ...);
char *_shellfront_prepare_hashable(char *cmd, char **exe_name, int is_integrate);
struct err_state _shellfront_gerror_to_err_state(GError *gerror);

char *mock_realpath(const char *path, char *resolved_path) {
	assert(resolved_path == NULL);
	resolved_path = malloc(strlen(path) + 6);
	resolved_path[strlen(path) + 5] = '\0';
	strcpy(resolved_path, "/bin/");
	strcpy(resolved_path + 5, path);
	return resolved_path;
}

void mock_g_clear_error(GError **err) {
	if (*err == NULL) {
		assert_test_state(TEST_STATE_NONE);
	} else {
		assert_test_state(TEST_STATE_WILL_RETURN_ERROR);
		if (*err != &mock_gerror) g_clear_error(err);
	}
}

void test_util() {
	// struct err_state define_error(char *msg)
	struct err_state state = define_error("Error occured");
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Error occured") == 0);
	// int _shellfront_parse_size_str(char *size, long *x, long *y, char *delim)
	long lx, ly;
	assert(!_shellfront_parse_size_str("0x0", &lx, &ly));
	assert(!_shellfront_parse_size_str("0x1", &lx, &ly));
	assert(!_shellfront_parse_size_str("1x0", &lx, &ly));
	assert(!_shellfront_parse_size_str("11", &lx, &ly));
	assert(!_shellfront_parse_size_str("1xx1", &lx, &ly));
	assert(!_shellfront_parse_size_str("1yx1", &lx, &ly));
	assert(_shellfront_parse_size_str("1x1", &lx, &ly));
	assert(_shellfront_parse_size_str("1x2", &lx, &ly));
	assert(lx == 1 && ly == 2);
	// int _shellfront_parse_loc_str(char *size, int *x, int *y, char *delim)
	int ix, iy;
	assert(!_shellfront_parse_loc_str("-1,-1", &ix, &iy));
	assert(!_shellfront_parse_loc_str("-1,0", &ix, &iy));
	assert(!_shellfront_parse_loc_str("0,-1", &ix, &iy));
	assert(!_shellfront_parse_loc_str("00", &ix, &iy));
	assert(!_shellfront_parse_loc_str("0,,0", &ix, &iy));
	assert(!_shellfront_parse_loc_str("0`,0", &ix, &iy));
	assert(_shellfront_parse_loc_str("0,0", &ix, &iy));
	assert(_shellfront_parse_loc_str("3,4", &ix, &iy));
	assert(ix == 3 && iy == 4);
	// unsigned long djb_hash(char *str)
	assert(djb_hash("hi") == 5863446);
	// char *sxprintf(char *fmt, ...)
	char *output_str = sxprintf("%ix%s", 123, "d");
	assert(strcmp(output_str, "123xd") == 0);
	free(output_str);
	// char *_shellfront_prepare_hashable(char *cmd, char **exe_name, int is_integrate)
	// integrate use PATH
	char *exe_name;
	char *prepared_cmd = _shellfront_prepare_hashable("exe --no-shellfront", &exe_name, 1);
	assert(strcmp(exe_name, "exe") == 0);
	assert(strcmp(prepared_cmd, "exe --no-shellfront") == 0);
	free(exe_name);
	free(prepared_cmd);
	// interpret use PATH
	prepared_cmd = _shellfront_prepare_hashable("exe -c command", &exe_name, 0);
	assert(strcmp(exe_name, "shellfront") == 0);
	assert(strcmp(prepared_cmd, "exe -c command") == 0);
	free(exe_name);
	free(prepared_cmd);
	// integrate no PATH
	prepared_cmd = _shellfront_prepare_hashable("rel_path/exe --no-shellfront", &exe_name, 1);
	assert(strcmp(exe_name, "exe") == 0);
	assert(strcmp(prepared_cmd, "/bin/rel_path/exe --no-shellfront") == 0);
	free(exe_name);
	free(prepared_cmd);
	// interpret no PATH
	prepared_cmd = _shellfront_prepare_hashable("rel_path/exe", &exe_name, 0);
	assert(strcmp(exe_name, "shellfront") == 0);
	assert(strcmp(prepared_cmd, "/bin/rel_path/exe") == 0);
	free(exe_name);
	free(prepared_cmd);
	// struct err_state _shellfront_gerror_to_err_state(GError *gerror)
	// gerror is NULL
	state = _shellfront_gerror_to_err_state(NULL);
	assert(!state.has_error);
	// gerror is not NULL
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	state = _shellfront_gerror_to_err_state(&mock_gerror);
	assert(state.has_error == 2);
	assert(strcmp(state.errmsg, "Error message") == 0);
	clear_test_state();
}
