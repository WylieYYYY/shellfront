#include "internal.h"
#include "shellfront.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNIT_TEST
	struct err_state mock_initialize(struct shellfront_term_conf *config);
	#define _shellfront_initialize(x) mock_initialize(x)
	struct err_state mock_parse(int argc, char **argv, struct shellfront_term_conf *config);
	#define _shellfront_parse(x,y,z) mock_parse(x,y,z)
	struct err_state mock_start_process(char *prog_name, struct shellfront_term_conf config, char *current_tty);
	#define _SHELLFRONT_START_PROCESS(x,y,z) mock_start_process(x,y,z)
#else
	#define _SHELLFRONT_START_PROCESS(x,y,z) _shellfront_start_process(x,y,z)
#endif

const struct shellfront_term_conf shellfront_term_conf_default = {
	.grav = 1,
	.x = 0,
	.y = 0,
	.width = 80,
	.height = 24,
	.title = "",
	.cmd = "",
	.interactive = 0,
	.ispopup = 0,
	.once = 0,
	.toggle = 0,
	.killopt = 0
};

struct err_state _shellfront_start_process(char *prog_name, struct shellfront_term_conf config, char *current_tty) {
	char *invoke_cmd = sxprintf("%s --no-shellfront 2>%s", prog_name, current_tty);
	config.cmd = invoke_cmd;
	struct err_state state = _shellfront_initialize(&config);
	free(invoke_cmd);
	// if executed with no error, indicate it is the original process
	if (!state.has_error) strcpy(state.errmsg, "Original process, please end");
	return state;
}

struct err_state shellfront_interpret(int argc, char **argv) {
	struct shellfront_term_conf config;
	struct err_state state = _shellfront_parse(argc, argv, &config);
	// parse error or continue execution
	if (state.has_error) return state;
	return _shellfront_initialize(&config);
}

struct err_state shellfront_catch_io_from_arg(int argc, char **argv) {
	// check whether this instance should use shellfront
	bool use_shellfront = true;
	for (int i = 1; i < argc; i++) {
		// this function is intended to catch itself as command
		if (strcmp(argv[i], "-c") == 0 || (strncmp(argv[i], "--cmd", 5) == 0 &&
			(argv[i][5] == '\0' || argv[i][5] == '='))) {
			return define_error("This application is intended to run without -c switch");
		}
		if (strcmp(argv[i], "--no-shellfront") == 0) use_shellfront = false;
	}
	if (use_shellfront) {
		struct shellfront_term_conf config;
		struct err_state state = _shellfront_parse(argc, argv, &config);
		// parse error
		if (state.has_error) return state;
		// return the state of the process
		return _SHELLFRONT_START_PROCESS(argv[0], config, ttyname(STDIN_FILENO));
	}
	// this is running in ShellFront, return no error
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
struct err_state shellfront_catch_io(int argc, char **argv, struct shellfront_term_conf config) {
	int use_shellfront = true;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--no-shellfront") == 0) use_shellfront = false;
	}
	// this time argv is ignored except the "--no-shellfront" flag, so if there is "-c" in argv, it doesn't matter
	if (config.cmd != NULL) return define_error("ShellFront integration does not require cmd");
	if (use_shellfront) {
		// return the state of the process
		return _SHELLFRONT_START_PROCESS(argv[0], config, ttyname(STDIN_FILENO));
	}
	// this is running in ShellFront, return no error
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
