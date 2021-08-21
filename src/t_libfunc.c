#include "internal.h"
#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct err_state define_error(char *msg);
struct err_state _shellfront_start_process(int argc, char **argv, char *accepted_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *default_config);
struct err_state shellfront_interpret(int argc, char **argv);

static int mock_argc = 2;
static char **mock_argv = (char *[]){ "shellfront", "--dummy" };

struct err_state mock_initialize(struct _shellfront_env_data *data) {
	add_test_state(TEST_STATE_INITIALIZED);
	assert_test_state(TEST_STATE_PARSED);
	if (test_state_contains(TEST_STATE_WILL_BE_INTEGRATION)) {
		assert(data->is_integrate);
		assert(data->argc == mock_argc);
		assert(&(data->argc) != &mock_argc);
		assert(memcmp(*(data->argv), *mock_argv, mock_argc * sizeof (char *)) == 0);
		assert(data->argv != mock_argv);
	} else assert(!data->is_integrate);
	// write the command detail for error message
	if (test_state_contains(TEST_STATE_WILL_FAIL_INITIALIZE)) return define_error("Initialize error");
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
struct err_state mock_parse(int argc, char **argv, char *builtin_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *config) {
	add_test_state(TEST_STATE_PARSED);
	if (test_state_contains(TEST_STATE_WILL_FAIL_PARSE)) return define_error("Parse error");
	// put command in for mock_initialize
	config->cmd = "Parsed command";
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
bool mock_g_option_context_parse(GOptionContext *context, int *argc, char ***argv, GError **error) {
	GError *ignored_error = NULL;
	g_option_context_parse(context, argc, argv, &ignored_error);
	g_clear_error(&ignored_error);
	if (test_state_contains(TEST_STATE_WILL_FAIL_PARSE)) {
		add_test_state(TEST_STATE_WILL_RETURN_ERROR);
		mock_gerror.code = 0;
		*error = &mock_gerror;
	} else if (test_state_contains(TEST_STATE_WILL_RETURN_ERROR)) {
		*error = &mock_gerror;
	}
	return true;
}
struct err_state mock_start_process(int argc, char **argv, char *accepted_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *default_config) {
	if (test_state_contains(TEST_STATE_WILL_FAIL_START_PROCESS)) return define_error("Start process error");
	// return message to indicate it is outside ShellFront
	return ((struct err_state) { .has_error = 0, .errmsg = "Original process, please end" });
}

void test_libfunc() {
	// struct err_state _shellfront_start_process(int argc, char **argv, char *accepted_opt,
	//     GOptionEntry *custom_opt, struct shellfront_term_conf *default_config)
	// test state[1, 0] (Initialize error)
	add_test_state(TEST_STATE_WILL_BE_INTEGRATION);
	add_test_state(TEST_STATE_WILL_FAIL_INITIALIZE);
	struct shellfront_term_conf config = shellfront_term_conf_default;
	struct err_state state = _shellfront_start_process(mock_argc, mock_argv, "", NULL, &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Initialize error") == 0);
	clear_test_state();
	// test state[0, 1] (Parse error)
	add_test_state(TEST_STATE_WILL_BE_INTEGRATION);
	add_test_state(TEST_STATE_WILL_FAIL_PARSE);
	state = _shellfront_start_process(mock_argc, mock_argv, "", NULL, &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	clear_test_state();
	// test state[0, 0] (No error)
	add_test_state(TEST_STATE_WILL_BE_INTEGRATION);
	state = _shellfront_start_process(mock_argc, mock_argv, "", NULL, &config);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "Original process, please end") == 0);
	clear_test_state();
	// struct err_state shellfront_interpret(int argc, char **argv)
	// test state[1, 0] (Initialize error)
	add_test_state(TEST_STATE_WILL_FAIL_INITIALIZE);
	state = shellfront_interpret(mock_argc, mock_argv);
	assert_test_state(TEST_STATE_INITIALIZED);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Initialize error") == 0);
	clear_test_state();
	// test state[0, 1] (Parse error)
	add_test_state(TEST_STATE_WILL_FAIL_PARSE);
	state = shellfront_interpret(mock_argc, mock_argv);
	assert_test_state(TEST_STATE_PARSED);
	assert_test_state_not(TEST_STATE_INITIALIZED);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Parse error") == 0);
	clear_test_state();
	// struct err_state shellfront_catch(int argc, char **argv, char *accepted_opt,
	//     GOptionEntry *custom_opt, struct shellfront_term_conf default_config)
	// test state[0, 0] (c error)
	_shellfront_fork_state = (struct err_state){ .has_error = 0, .errmsg = "" };
	state = shellfront_catch(mock_argc, mock_argv, "gcT", NULL, shellfront_term_conf_default);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "ShellFront integration does not allow cmd option") == 0);
	// test state[0, 0] (No error, in ShellFront)
	_shellfront_fork_state._forked = true;
	state = shellfront_catch(1, (char *[]){ "shellfront" }, "", NULL, shellfront_term_conf_default);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "") == 0);
	// test state[0, 0] (Second parse error, in ShellFront)
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	state = shellfront_catch(1, (char *[]){ "shellfront" }, "", NULL, shellfront_term_conf_default);
	assert(state.has_error == 2);
	assert(strcmp(state.errmsg, "Error message") == 0);
	clear_test_state();
	// test state[0, 0] (Code zero error, in ShellFront)
	_shellfront_fork_state = ((struct err_state){ ._forked = true });
	add_test_state(TEST_STATE_WILL_FAIL_PARSE);
	state = shellfront_catch(1, (char *[]){ "shellfront" }, "", NULL, shellfront_term_conf_default);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "") == 0);
	mock_gerror.code = 2;
	clear_test_state();
	// test state[1, 0] (Start process error)
	_shellfront_fork_state._forked = false;
	add_test_state(TEST_STATE_WILL_BE_INTEGRATION);
	add_test_state(TEST_STATE_WILL_FAIL_START_PROCESS);
	state = shellfront_catch(1, (char *[]){ "shellfront" }, "", NULL, shellfront_term_conf_default);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Start process error") == 0);
	clear_test_state();
	// test state[0, 1] (No error, non-null custom option, in ShellFront)
	_shellfront_fork_state._forked = true;
	bool dummy_opt_data = false;
	GOptionEntry mock_custom_opt[] = {
		{
			.long_name = "dummy",
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &dummy_opt_data,
			.description = "Dummy option"
		}, { 0 }
	};
	state = shellfront_catch(mock_argc, mock_argv, "", mock_custom_opt, shellfront_term_conf_default);
	assert(!state.has_error);
	assert(dummy_opt_data);
	// test state[0, 0] (No error)
	_shellfront_fork_state._forked = false;
	state = shellfront_catch(1, (char *[]){ "shellfront" }, "", NULL, shellfront_term_conf_default);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "Original process, please end") == 0);
}
