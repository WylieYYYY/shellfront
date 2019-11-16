#include "internal.h"
#include "shellfront.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int shellfront_interpret(int argc, char **argv) {
	return shellfront_initialize(shellfront_parse(argc, argv));
}

void shellfront_catch_io_from_arg(int argc, char **argv) {
	// check whether this instance should use shellfront
	bool use_shellfront = true;
	for (int i = 0; i < argc; i++) {
		// this function is intended to catch itself as command
		if (strcmp(argv[i], "-c") == 0) {
			fprintf(stderr, "This application is intended to run without -c switch\n");
			exit(1);
		}
		if (strcmp(argv[i], "--no-shellfront") == 0) {
			use_shellfront = false;
		}
	}
	if (use_shellfront) {
		struct term_conf config = shellfront_parse(argc, argv);
		// if the program is reinitialized in shellfront, it should not initialize shellfront again in new instance
		// redirect error to the old terminal
		char *invoke_cmd = sxprintf("%s --no-shellfront 2>%s", argv[0], ttyname(STDIN_FILENO));
		config.cmd = invoke_cmd;
		int status = shellfront_initialize(config);
		free(invoke_cmd);
		exit(status);
	}
}
void shellfront_catch_io(int argc, char **argv, struct term_conf config) {
	int use_shellfront = true;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--no-shellfront") == 0) {
			use_shellfront = false;
		}
	}
	// this time argv is ignored except the "--no-shellfront" flag, so if there is "-c" in argv, it doesn't matter
	if (config.cmd != NULL) {
		fprintf(stderr, "This application is intended to run without -c switch\n");
		exit(1);
	}
	if (use_shellfront) {
		char *invoke_cmd = sxprintf("%s --no-shellfront 2>%s", argv[0], ttyname(STDIN_FILENO));
		config.cmd = invoke_cmd;
		// initialize with default values if not defined in the struct
		if (config.grav == 0) {
			config.grav = 1;
		}
		if (config.width == 0) {
			config.width = 80;
		}
		if (config.height == 0) {
			config.height = 24;
		}
		int status = shellfront_initialize(config);
		free(invoke_cmd);
		exit(status);
	}
}
