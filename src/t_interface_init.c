#include "internal.h"
#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

extern char *_shellfront_tmpid;
void _shellfront_gtk_activate(GtkApplication *app, struct _shellfront_env_data *data);
struct err_state _shellfront_initialize(struct _shellfront_env_data *data);

int mock_g_application_run(GApplication *application, int argc, char **argv) {
	// check application id by process id
	const char *appid = g_application_get_application_id(application);
	assert(strcmp(appid, "shellfront.proc123") == 0);
	// use signal type id and check handler with function and data
	unsigned int sigid = g_signal_lookup("activate", G_TYPE_FROM_INSTANCE(application));
	unsigned long sighandler = g_signal_handler_find(application,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		sigid, 0, NULL, &_shellfront_gtk_activate, &mock_env_data);
	// handler exists
	assert(sighandler != 0);
	// needs to handle non-binary error code
	if (test_state_contains(TEST_STATE_WILL_RETURN_ERROR)) return 2;
	return 0;
}
struct err_state mock_unlock_process() {
	free(_shellfront_tmpid);
	add_test_state(TEST_STATE_PROCESS_UNLOCKED);
	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
struct err_state mock_lock_process(int pid) {
	free(_shellfront_tmpid);
	add_test_state(TEST_STATE_PROCESS_LOCKED);
	return ((struct err_state) {
		.has_error = test_state_contains(TEST_STATE_WILL_RETURN_ERROR)
	});
}
int mock_access(const char *pathname, int mode) {
	// check provided lock file name and mode
	assert(strcmp(pathname, "/tmp/shellfront.5863446.lock") == 0);
	assert(mode == F_OK);
	if (test_state_contains(TEST_STATE_WILL_HAVE_LOCK_FILE)) return 0;
	return -1;
}

void test_interface_init() {
	// struct err_state _shellfront_initialize(struct shellfront_term_conf *config)
	// test no lock condition flags (GTK error)
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	struct shellfront_term_conf config = shellfront_term_conf_default;
	mock_env_data = ((struct _shellfront_env_data) {
		.term_conf = &config,
		.is_integrate = true,
		.argc = 0,
		.argv = NULL
	});
	struct err_state state = _shellfront_initialize(&mock_env_data);
	assert(state.has_error == 2);
	assert(strcmp(state.errmsg, "GTK error") == 0);
	clear_test_state();
	// test no lock condition flags (no error)
	state = _shellfront_initialize(&mock_env_data);
	assert(!state.has_error);
	// test kill
	config.kill = true;
	state = _shellfront_initialize(&mock_env_data);
	assert(!state.has_error);
	assert_test_state(TEST_STATE_PROCESS_UNLOCKED);
	// test once (lock error)
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	config.kill = false;
	config.once = true;
	state = _shellfront_initialize(&mock_env_data);
	assert(state.has_error);
	assert_test_state(TEST_STATE_PROCESS_LOCKED);
	// test once (no error)
	mock_env_data.is_integrate = false;
	state = _shellfront_initialize(&mock_env_data);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "") == 0);
	mock_env_data.is_integrate = true;
	// test toggle and lock file does not exist
	config.cmd = "hi";
	config.once = false;
	config.toggle = true;
	state = _shellfront_initialize(&mock_env_data);
	assert(!state.has_error);
	assert_test_state(TEST_STATE_PROCESS_LOCKED);
	// test toggle and lock file exists
	add_test_state(TEST_STATE_WILL_HAVE_LOCK_FILE);
	state = _shellfront_initialize(&mock_env_data);
	assert(!state.has_error);
	assert_test_state(TEST_STATE_PROCESS_UNLOCKED);
}
