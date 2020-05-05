#include "shellfront.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct err_state define_error(char *msg);
struct err_state _shellfront_start_process(char *prog_name, struct shellfront_term_conf *config, char *current_tty);
struct err_state shellfront_interpret(int argc, char **argv);

static bool process_test_state;
struct err_state mock_initialize(struct shellfront_term_conf *config) {
	process_test_state ^= true;
	// write the command detail for error message
	if (process_test_state) return define_error(config->cmd);
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
static bool parse_test_state = true;
struct err_state mock_parse(int argc, char **argv, char *builtin_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *config) {
	parse_test_state ^= true;
	if (parse_test_state) return define_error("Parse error");
	// put command in for mock_initialize
	config->cmd = "Parsed command";
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
struct err_state mock_start_process(char *prog_name, struct shellfront_term_conf *config, char *current_tty) {
	process_test_state ^= true;
	if (process_test_state) return define_error("Start process error");
	// return message to indicate it is outside ShellFront
	return ((struct err_state) { .has_error = 0, .errmsg = "Original process, please end" });
}

void test_libfunc() {
	// struct err_state _shellfront_start_process(char *prog_name, struct shellfront_term_conf config, char *current_tty)
	// test state[1, 0] (Initialize error)
	struct shellfront_term_conf config = shellfront_term_conf_default;
	struct err_state state = _shellfront_start_process("procname", &config, "ttyname");
	assert(state.has_error);
	assert(strcmp(state.errmsg, "procname --no-shellfront 2>ttyname") == 0);
	// test state[0, 0] (No error)
	state = _shellfront_start_process("procname", &config, "ttyname");
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "Original process, please end") == 0);
	// struct err_state shellfront_interpret(int argc, char **argv)
	// test state[1, 0] (Initialize error)
	char **argv = (char *[]){ "shellfront", "-c", "--no-shellfront" };
	state = shellfront_interpret(3, argv);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Parsed command") == 0);
	// test state[0, 1] (Parse error)
	state = shellfront_interpret(3, argv);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	// struct err_state shellfront_catch(int argc, char **argv, char *accepted_opt,
	//     GOptionEntry *custom_opt, struct shellfront_term_conf default_config)
	// test state[0, 0] (c error)
	state = shellfront_catch(3, argv, "gcT", NULL, shellfront_term_conf_default);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "ShellFront integration does not allow cmd option") == 0);
	// test state[0, 0] (No error, in ShellFront)
	state = shellfront_catch(3, argv, "", NULL, shellfront_term_conf_default);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "") == 0);
	// test state[1, 0] (Start process error)
	process_test_state = 0;
	state = shellfront_catch(2, argv, "", NULL, shellfront_term_conf_default);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Start process error") == 0);
	// test state[0, 1] (Parse error)
	state = shellfront_catch(2, argv, "", NULL, shellfront_term_conf_default);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	// test state[0, 0] (No error)
	state = shellfront_catch(2, argv, "", NULL, shellfront_term_conf_default);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "Original process, please end") == 0);
}
