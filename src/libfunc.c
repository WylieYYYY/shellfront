#include "internal.h"
#include "shellfront.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNIT_TEST
	struct err_state mock_initialize(struct term_conf config);
	#define shellfront_initialize(x) mock_initialize(x)
	struct err_state mock_parse(int argc, char **argv, struct term_conf *config);
	#define shellfront_parse(x,y,z) mock_parse(x,y,z)
	struct err_state mock_start_process(char *prog_name, struct term_conf config, char *current_tty);
	#define SHELLFRONT_START_PROCESS(x,y,z) mock_start_process(x,y,z)
#else
	#define SHELLFRONT_START_PROCESS(x,y,z) shellfront_start_process(x,y,z)
#endif

struct err_state shellfront_start_process(char *prog_name, struct term_conf config, char *current_tty) {
	char *invoke_cmd = sxprintf("%s --no-shellfront 2>%s", prog_name, current_tty);
	config.cmd = invoke_cmd;
	struct err_state state = shellfront_initialize(config);
	free(invoke_cmd);
	// if executed with no error, indicate it is the original process
	if (!state.has_error) strcpy(state.errmsg, "Original process, please end");
	return state;
}

struct err_state shellfront_interpret(int argc, char **argv) {
	struct term_conf config;
	struct err_state state = shellfront_parse(argc, argv, &config);
	// parse error or continue execution
	if (state.has_error) return state;
	return shellfront_initialize(config);
}

struct err_state shellfront_catch_io_from_arg(int argc, char **argv) {
	// check whether this instance should use shellfront
	bool use_shellfront = true;
	for (int i = 1; i < argc; i++) {
		// this function is intended to catch itself as command
		if (strcmp(argv[i], "-c") == 0) return define_error("This application is intended to run without -c switch");
		if (strcmp(argv[i], "--no-shellfront") == 0) use_shellfront = false;
	}
	if (use_shellfront) {
		struct term_conf config;
		struct err_state state = shellfront_parse(argc, argv, &config);
		// parse error
		if (state.has_error) return state;
		// return the state of the process
		return SHELLFRONT_START_PROCESS(argv[0], config, ttyname(STDIN_FILENO));
	}
	// this is running in ShellFront, return no error
	return ((struct err_state) {});
}
struct err_state shellfront_catch_io(int argc, char **argv, struct term_conf config) {
	int use_shellfront = true;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--no-shellfront") == 0) use_shellfront = false;
	}
	// this time argv is ignored except the "--no-shellfront" flag, so if there is "-c" in argv, it doesn't matter
	if (config.cmd != NULL) return define_error("This application is intended to run without -c switch");
	if (use_shellfront) {
		// initialize with default values if not defined in the struct
		if (config.grav == 0) config.grav = 1;
		if (config.width == 0) config.width = 80;
		if (config.height == 0) config.height = 24;
		// return the state of the process
		return SHELLFRONT_START_PROCESS(argv[0], config, ttyname(STDIN_FILENO));
	}
	// this is running in ShellFront, return no error
	return ((struct err_state) {});
}