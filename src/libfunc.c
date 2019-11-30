#include "internal.h"
#include "shellfront.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct err_state shellfront_start_process(char *prog_name, struct term_conf config, char *current_tty) {
	char *invoke_cmd = sxprintf("%s --no-shellfront 2>%s", prog_name, current_tty);
	config.cmd = invoke_cmd;
	struct err_state state = shellfront_initialize(config);
	free(invoke_cmd);
	if (!state.has_error) strcpy(state.errmsg, "Original process, please end");
	return state;
}

struct err_state shellfront_interpret(int argc, char **argv) {
	struct term_conf config;
	struct err_state state = shellfront_parse(argc, argv, &config);
	if (state.has_error) return state;

	state = shellfront_initialize(config);
	return state;
}

struct err_state shellfront_catch_io_from_arg(int argc, char **argv) {
	// check whether this instance should use shellfront
	bool use_shellfront = true;
	for (int i = 0; i < argc; i++) {
		// this function is intended to catch itself as command
		if (strcmp(argv[i], "-c") == 0) return define_error("This application is intended to run without -c switch");
		if (strcmp(argv[i], "--no-shellfront") == 0) use_shellfront = false;
	}
	if (use_shellfront) {
		struct term_conf config;
		struct err_state state = shellfront_parse(argc, argv, &config);
		if (state.has_error) return state;
		return shellfront_start_process(argv[0], config, ttyname(STDIN_FILENO));
	}

	return ((struct err_state) {});
}
struct err_state shellfront_catch_io(int argc, char **argv, struct term_conf config) {
	int use_shellfront = true;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--no-shellfront") == 0) use_shellfront = false;
	}
	// this time argv is ignored except the "--no-shellfront" flag, so if there is "-c" in argv, it doesn't matter
	if (config.cmd != NULL) return define_error("This application is intended to run without -c switch");
	if (use_shellfront) {
		// initialize with default values if not defined in the struct
		if (config.grav == 0) config.grav = 1;
		if (config.width == 0) config.width = 80;
		if (config.height == 0) config.height = 24;
		return shellfront_start_process(argv[0], config, ttyname(STDIN_FILENO));
	}

	return ((struct err_state) {});
}