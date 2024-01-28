#include "internal.h"
#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <vte/vte.h>

void test_gtkfunc_helper(void);

extern struct err_state _shellfront_fork_state;
void _shellfront_apply_opt(GtkWindow *window, VteTerminal *terminal,
	struct shellfront_term_conf *config, struct _shellfront_env_data *data);
void _shellfront_window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window);
void _shellfront_kill_child(GtkWindow *window, struct _shellfront_env_data *data);
void _shellfront_window_destroy(GtkWindow *window, void *user_data);
void _shellfront_child_process_setup(int pid, VtePty *pty, struct _shellfront_env_data *data);
void _shellfront_configure_terminal(VteTerminal *terminal, struct _shellfront_env_data *data);
void _shellfront_gtk_activate(GtkApplication *app, struct _shellfront_env_data *data);
void _shellfront_window_show(GtkWindow *window, void *user_data);
void _shellfront_terminal_exit(VteTerminal *terminal, int status, GtkWindow *window);

void mock_err_string_arg(FILE *fp, char *fmt, char *str) {
	assert_test_state(TEST_STATE_NONE);
	assert(fp == stderr);
	assert(strcmp(fmt, "%s\n") == 0);
	assert(str != NULL);
	add_test_state(TEST_STATE_ERROR_STATE_PRINTED);
}
static GtkWindow *window;
VteTerminal *test_terminal;
GtkWidget *mock_gtk_application_window_new(GtkApplication *application) {
	window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	return GTK_WIDGET(window);
}
void mock_vte_terminal_set_size(VteTerminal *terminal, long columns, long rows) {
	assert(terminal == test_terminal);
	assert(columns == 80 && rows == 24);
	add_test_state(TEST_STATE_TERMINAL_SIZE_SET);
}
void mock_vte_terminal_spawn_async(VteTerminal *terminal, VtePtyFlags pty_flags,
		const char *working_directory, char **argv, char **envv, GSpawnFlags spawn_flags_,
		GSpawnChildSetupFunc child_setup, void *child_setup_data,
		GDestroyNotify child_setup_data_destroy, int timeout, GCancellable *cancellable,
		VteTerminalSpawnAsyncCallback callback, void *user_data) {
	assert(terminal == test_terminal);
	assert(pty_flags == VTE_PTY_DEFAULT);
	assert(strcmp(argv[0], "/bin/bash") == 0);
	assert(strcmp(argv[1], "-c") == 0);
	assert(strcmp(argv[2], "command") == 0);
	assert(argv[3] == NULL);
	assert(spawn_flags_ == G_SPAWN_SEARCH_PATH);
	assert(timeout == -1);
	add_test_state(TEST_STATE_TERMINAL_SPAWNED);
}
void mock_gtk_widget_show_all(GtkWidget *widget) {
	// test once, interactive
	assert(widget == GTK_WIDGET(window));
	unsigned int sigid = g_signal_lookup("destroy", G_TYPE_FROM_INSTANCE(window));
	unsigned long sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC, sigid, 0, NULL, &_shellfront_window_destroy, NULL);
	assert(sighandler != 0);
	assert(!gtk_window_get_resizable(window));
	// test other config
	assert(strcmp(gtk_window_get_title(window), "Title") == 0);
	sigid = g_signal_lookup("show", G_TYPE_FROM_INSTANCE(window));
	sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC, sigid, 0, NULL, &_shellfront_window_show, NULL);
	assert(sighandler != 0);
	sigid = g_signal_lookup("child-exited", G_TYPE_FROM_INSTANCE(test_terminal));
	sighandler = g_signal_handler_find(test_terminal,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		sigid, 0, NULL, &_shellfront_terminal_exit, window);
	assert(sighandler != 0);
	GList *children = gtk_container_get_children(GTK_CONTAINER(window));
	assert(VTE_TERMINAL(children->data) == test_terminal);
	g_list_free(children);
	int x, y;
	gtk_window_get_position(window, &x, &y);
	assert(x == 0 && y == 0);
	assert_test_state(TEST_STATE_TERMINAL_SIZE_SET);
	assert_test_state(TEST_STATE_TERMINAL_SPAWNED);
	add_test_state(TEST_STATE_WIDGET_SHOWED);
}

void mock_configure_terminal(VteTerminal *terminal, struct _shellfront_env_data *data) {
	assert(terminal == test_terminal);
	assert(data == &mock_env_data);
	add_test_state(TEST_STATE_TERMINAL_SPAWNED);
}

int mock_main(int argc, char **argv) {
	assert_test_state(TEST_STATE_CHILD_PROCESS_SETUP);
	assert(argc == mock_env_data.argc);
	assert(argv == mock_env_data.argv);
	if (test_state_contains(TEST_STATE_WILL_FAIL_FORK)) {
		assert(_shellfront_fork_state.has_error);
		assert(strcmp(_shellfront_fork_state.errmsg, "Fork error") == 0);
	} else {
		assert(_shellfront_fork_state.has_error == 2);
		assert(strcmp(_shellfront_fork_state.errmsg, "Error message") == 0);
	}
}

static VtePty *test_pty;
void mock_vte_pty_child_setup(VtePty *pty) {
	assert(pty == test_pty);
	add_test_state(TEST_STATE_CHILD_PROCESS_SETUP);
}

VtePty *mock_vte_pty_new_sync(VtePtyFlags flags, GCancellable *cancellable, GError **error) {
	assert(flags == VTE_PTY_DEFAULT);
	assert(cancellable == NULL);
	test_pty = vte_pty_new_sync(flags, cancellable, NULL);
	*error = &mock_gerror;
	return test_pty;
}

pid_t mock_fork(VteTerminal *terminal) {
	assert(_shellfront_fork_state.has_error == 2);
	assert(strcmp(_shellfront_fork_state.errmsg, "Error message") == 0);
	assert(vte_terminal_get_pty(terminal) == test_pty);
	if (test_state_contains(TEST_STATE_WILL_FAIL_FORK)) return -1;
	if (test_state_contains(TEST_STATE_WILL_BE_PARENT_PROCESS)) return 123;
	return 0;
}

void mock__shellfront_child_process_setup(int pid, VtePty *pty, struct _shellfront_env_data *data) {
	if (test_state_contains(TEST_STATE_WILL_FAIL_FORK)) assert(pid == -1);
	else assert(pid == 0);
	assert(pty == test_pty);
	assert(data == &mock_env_data);
	add_test_state(TEST_STATE_CHILD_PROCESS_SETUP);
}

void test_gtkfunc() {
	test_gtkfunc_helper();
	// void _shellfront_apply_opt(GtkWindow *window, VteTerminal *terminal, struct term_conf *config)
	// test ispopup
	gtk_init(0, NULL);
	struct shellfront_term_conf config = shellfront_term_conf_default;
	config.popup = 1;
	config.icon = "";
	window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	test_terminal = VTE_TERMINAL(vte_terminal_new());
	_shellfront_apply_opt(window, test_terminal, &config, &mock_env_data);
	assert(!gtk_widget_get_sensitive(GTK_WIDGET(test_terminal)));
	assert(!gtk_window_get_decorated(window));
	assert(gtk_window_get_skip_taskbar_hint(window));
	unsigned int sigid = g_signal_lookup("focus-out-event", G_TYPE_FROM_INSTANCE(window));
	unsigned long sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		sigid, 0, NULL, &_shellfront_window_focus_out, window);
	assert(sighandler != 0);
	sigid = g_signal_lookup("destroy", G_TYPE_FROM_INSTANCE(window));
	sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC, sigid, 0, NULL, &_shellfront_kill_child, window);
	assert(sighandler != 0);
	assert(gtk_window_get_icon(window) == NULL);
	// test icon error
	config.icon = "/";
	_shellfront_apply_opt(window, test_terminal, &config, &mock_env_data);
	assert_test_state(TEST_STATE_ERROR_STATE_PRINTED);
	assert(gtk_window_get_icon(window) == NULL);
	gtk_widget_destroy(GTK_WIDGET(window));
	// void gtk_activate(GtkApplication *app, struct term_conf *config)
	config.once = 1;
	config.interactive = 1;
	config.popup = 0;
	config.title = "Title";
	config.icon = "favicon.png";
	config.cmd = "command";
	config.grav = 5;
	mock_env_data.term_conf = &config;
	mock_env_data.is_integrate = 0;
	_shellfront_gtk_activate(NULL, &mock_env_data);
	assert_test_state(TEST_STATE_WIDGET_SHOWED);
	int x, y;
	gtk_window_get_position(window, &x, &y);
	assert(x != 0 && y != 0);
	assert(gtk_window_get_icon(window) != NULL);
	// void _shellfront_configure_terminal(VteTerminal *terminal, struct _shellfront_env_data *data)
	// integrate, child process
	mock_env_data.is_integrate = 1;
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	_shellfront_configure_terminal(test_terminal, &mock_env_data);
	assert_test_state(TEST_STATE_CHILD_PROCESS_SETUP);
	// integrate, fork error
	add_test_state(TEST_STATE_WILL_FAIL_FORK);
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	_shellfront_configure_terminal(test_terminal, &mock_env_data);
	assert_test_state(TEST_STATE_CHILD_PROCESS_SETUP);
	// integrate, parent process
	add_test_state(TEST_STATE_WILL_BE_PARENT_PROCESS);
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	_shellfront_configure_terminal(test_terminal, &mock_env_data);
	assert_test_state_not(TEST_STATE_CHILD_PROCESS_SETUP);
	assert(mock_env_data.child_pid == 123);
	// no integrate
	mock_env_data.is_integrate = 0;
	_shellfront_configure_terminal(test_terminal, &mock_env_data);
	assert_test_state(TEST_STATE_TERMINAL_SPAWNED);
	gtk_widget_destroy(GTK_WIDGET(test_terminal));
	gtk_widget_destroy(GTK_WIDGET(window));
	// void _shellfront_child_process_setup(int pid, VtePty *pty, struct _shellfront_env_data *data)
	// once flag
	mock_env_data.argc = 2;
	mock_env_data.argv = (char *[]){ "lorem", "ipsum" };
	_shellfront_child_process_setup(0, test_pty, &mock_env_data);
	assert_test_state(TEST_STATE_EXITED);
	// no once flag
	config.once = 0;
	_shellfront_child_process_setup(0, test_pty, &mock_env_data);
	assert_test_state(TEST_STATE_EXITED);
	// dup error
	add_test_state(TEST_STATE_WILL_RETURN_ERROR);
	_shellfront_child_process_setup(0, test_pty, &mock_env_data);
	clear_test_state();
	// fork error
	add_test_state(TEST_STATE_WILL_FAIL_FORK);
	_shellfront_child_process_setup(-1, test_pty, &mock_env_data);
	assert_test_state(TEST_STATE_EXITED);
}
