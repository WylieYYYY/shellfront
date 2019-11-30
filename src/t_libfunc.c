#include "shellfront.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct err_state define_error(char *msg);
char *sxprintf(char *fmt, ...);
struct err_state shellfront_start_process(char *prog_name, struct term_conf config, char *current_tty);
struct err_state shellfront_interpret(int argc, char **argv);

static bool process_test_state;
struct err_state mock_initialize(struct term_conf config) {
	// every time it will be used twice, flip state
	process_test_state ^= true;
	// write the command detail for error message
	if (process_test_state) return define_error(config.cmd);
	// else return a blank err_state
	return ((struct err_state) {});
}
static bool parse_test_state = true;
struct err_state mock_parse(int argc, char **argv, struct term_conf *config) {
	parse_test_state ^= true;
	if (parse_test_state) return define_error("Parse error");
	// put command in for mock_initialize
	config->cmd = "Parsed command";
	return ((struct err_state) {});
}
struct err_state mock_start_process(char *prog_name, struct term_conf config, char *current_tty) {
	process_test_state ^= true;
	if (process_test_state) return define_error("Start process error");
	// return grav, width and height for testing default setting
	char *setting = sxprintf("%u,%ld,%ld", config.grav, config.width, config.height);
	struct err_state state;
	strcpy(state.errmsg, setting);
	free(setting);
	return state;
}

static void check_config(char *list, int grav, long width, long height) {
	char *cp = strdup(list);
	assert(grav == atoi(strsep(&cp, ",")));
	assert(width == atol(strsep(&cp, ",")));
	assert(height == atol(cp));
}

void test_libfunc() {
	// struct err_state shellfront_start_process(char *prog_name, struct term_conf config, char *current_tty)
	// test state[1, 0] (Initialize error)
	struct term_conf config;
	struct err_state state = shellfront_start_process("procname", config, "ttyname");
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "procname --no-shellfront 2>ttyname") == 0);
	// test state[0, 0] (No error)
	state = shellfront_start_process("procname", config, "ttyname");
	assert(state.has_error == 0);
	assert(strcmp(state.errmsg, "Original process, please end") == 0);
	// struct err_state shellfront_interpret(int argc, char **argv)
	// test state[1, 0] (Initialize error)
	char **argv = (char *[]){ "shellfront", "-c", "--no-shellfront", "echo hi" };
	state = shellfront_interpret(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "Parsed command") == 0);
	// test state[0, 1] (Parse error)
	state = shellfront_interpret(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	// struct err_state shellfront_catch_io_from_arg(int argc, char **argv)
	// test state[0, 0] (-c error)
	state = shellfront_catch_io_from_arg(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "This application is intended to run without -c switch") == 0);
	// test state[0, 0] (No error, in ShellFront)
	argv[1] = "-p";
	state = shellfront_catch_io_from_arg(4, argv);
	assert(state.has_error == 0);
	// test state[1, 0] (Start process error)
	process_test_state = 0;
	argv[2] = "-T";
	state = shellfront_catch_io_from_arg(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "Start process error") == 0);
	// test state[0, 1] (Parse error)
	state = shellfront_catch_io_from_arg(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	// struct err_state shellfront_catch_io(int argc, char **argv, struct term_conf config)
	// test state[0, 0] (No error, default setting)
	config.cmd = NULL;
	state = shellfront_catch_io(4, argv, config);
	assert(state.has_error == 0);
	check_config(state.errmsg, 1, 80, 24);
	// test state[1, 0] (-c error)
	config.cmd = "echo hi";
	state = shellfront_catch_io(4, argv, config);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "ShellFront integration does not require cmd") == 0);
	// test state[1, 0] (No error, in ShellFront)
	config.cmd = NULL;
	argv[2] = "--no-shellfront";
	state = shellfront_catch_io(4, argv, config);
	assert(state.has_error == 0);
	// test state[0, 0] (No error, custom setting)
	process_test_state = true;
	argv[2] = "-T";
	config.grav = 3;
	config.width = 40;
	config.height = 12;
	state = shellfront_catch_io(4, argv, config);
	assert(state.has_error == 0);
	check_config(state.errmsg, 3, 40, 12);
}