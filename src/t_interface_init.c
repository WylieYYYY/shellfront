#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

extern char *_shellfront_tmpid;
void _shellfront_gtk_activate(GtkApplication *app, struct shellfront_term_conf *config);
struct err_state _shellfront_initialize(struct shellfront_term_conf *config, bool is_integrate);

static struct shellfront_term_conf config;
static bool run_test_state = true;
int mock_g_application_run(GApplication *application, int argc, char **argv) {
	// check application id by process id
	const char *appid = g_application_get_application_id(application);
	assert(strcmp(appid, "shellfront.proc123") == 0);
	// use signal type id and check handler with function and data
	unsigned int sigid = g_signal_lookup("activate", G_TYPE_FROM_INSTANCE(application));
	unsigned long sighandler = g_signal_handler_find(application,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		sigid, 0, NULL, &_shellfront_gtk_activate, &config);
	// handler exists
	assert(sighandler != 0);
	// needs to handle non-binary error code
	return run_test_state * 2;
}
struct err_state mock_unlock_process() {
	free(_shellfront_tmpid);
	return ((struct err_state) { .has_error = 0, .errmsg = "Unlocked process" });
}
struct err_state mock_lock_process(int pid) {
	run_test_state ^= true;
	free(_shellfront_tmpid);
	return ((struct err_state) { .has_error = run_test_state, .errmsg = "Lock process" });
}
int mock_access(const char *pathname, int mode) {
	run_test_state ^= true;
	// check provided lock file name and mode
	assert(strcmp(pathname, "/tmp/shellfront.5863446.lock") == 0);
	assert(mode == F_OK);
	// true for failure
	return -run_test_state;
}

void test_interface_init() {
	// struct err_state _shellfront_initialize(struct shellfront_term_conf *config)
	// test no lock condition flags (GTK error)
	config = shellfront_term_conf_default;
	struct err_state state = _shellfront_initialize(&config, true);
	assert(state.has_error == 2);
	assert(strcmp(state.errmsg, "GTK error") == 0);
	// test no lock condition flags (no error)
	run_test_state = false;
	state = _shellfront_initialize(&config, true);
	assert(!state.has_error);
	// test kill
	config.kill = true;
	state = _shellfront_initialize(&config, true);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "Unlocked process") == 0);
	// test once (lock error)
	config.kill = false;
	config.once = true;
	state = _shellfront_initialize(&config, true);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Lock process") == 0);
	// test once (no error)
	state = _shellfront_initialize(&config, true);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "") == 0);
	// test toggle and lock file does not exists
	config.cmd = "hi";
	config.once = false;
	config.toggle = true;
	state = _shellfront_initialize(&config, true);
	assert(state.has_error == 2);
	assert(strcmp(state.errmsg, "GTK error") == 0);
	// test toggle and lock file exists
	state = _shellfront_initialize(&config, true);
	assert(!state.has_error);
	assert(strcmp(state.errmsg, "Unlocked process") == 0);
}