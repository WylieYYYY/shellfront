#include "config.h"
#include "shellfront.h"

#include <stdio.h>

int main(int argc, char **argv) {
	struct err_state state = shellfront_interpret(argc, argv);
	if (state.has_error) fprintf(stderr, "%s\n", state.errmsg);
	return state.has_error;
}
