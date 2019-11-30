#include "shellfront.h"

#include <assert.h>
#include <string.h>

struct err_state define_error(char *msg);
struct err_state shellfront_start_process(char *prog_name, struct term_conf config, char *current_tty);
struct err_state shellfront_interpret(int argc, char **argv);

static int initialize_test_state;
struct err_state mock_initialize(struct term_conf config) {
	// every time it will be used twice, flip state
	initialize_test_state ^= 1;
	// write the command detail for error message
	if (initialize_test_state) return define_error(config.cmd);
	// else return a blank err_state
	return ((struct err_state) {});
}
static int parse_test_state = 1;
struct err_state mock_parse(int argc, char **argv, struct term_conf *config) {
	parse_test_state ^= 1;
	if (parse_test_state) return define_error("Parse error");
	config->cmd = "Parsed command";
	return ((struct err_state) {});
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
	char **argv = (char *[]){ "shellfront", "--no-shellfront", "-c", "echo hi" };
	state = shellfront_interpret(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "Parsed command") == 0);
	// test state[0, 1] (Parse error)
	state = shellfront_interpret(4, argv);
	assert(state.has_error != 0);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	// test state[1, 0]
}