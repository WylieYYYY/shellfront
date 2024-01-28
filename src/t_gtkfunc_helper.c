#include "internal.h"
#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

void _shellfront_terminal_exit(VteTerminal *terminal, int status, GtkWindow *window);
void _shellfront_window_show(GtkWindow *window, void *user_data);
void _shellfront_window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window);
void _shellfront_kill_child(GtkWindow *window, struct _shellfront_env_data *data);
void _shellfront_window_destroy(GtkWindow *window, void *user_data);
void _shellfront_window_gravitate(int window_width, int window_height,
	GdkRectangle *workarea, struct shellfront_term_conf *config);

static GtkWindow *test_window = (GtkWindow *)0xDEADBEEF;
void mock_gtk_window_close(GtkWindow *window) {
	assert(window == (GtkWindow *)0xDEADBEEF);
	add_test_state(TEST_STATE_WINDOW_CLOSED);
}
void mock_gtk_window_present(GtkWindow *window) {
	assert(window == (GtkWindow *)0xDEADBEEF);
	add_test_state(TEST_STATE_WINDOW_PRESENTED);
}
pid_t mock_waitpid(pid_t pid, int *status, int options) {
	assert(pid == 123 && options == WNOHANG);
	add_test_state(TEST_STATE_CHILD_WAITED);
}
void mock_sig_exit(int signo) {
	assert(signo == 2);
	add_test_state(TEST_STATE_EXITED);
}

void test_gtkfunc_helper() {
	// void t_shellfront_terminal_exit(VteTerminal *terminal, int status, GtkWindow *window)
	_shellfront_terminal_exit(NULL, 0, test_window);
	assert_test_state(TEST_STATE_WINDOW_CLOSED);
	// void _shellfront_window_show(GtkWindow *window, void *user_data)
	_shellfront_window_show(test_window, NULL);
	assert_test_state(TEST_STATE_WINDOW_PRESENTED);
	// void _shellfront_window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window)
	_shellfront_window_focus_out(NULL, NULL, test_window);
	assert_test_state(TEST_STATE_WINDOW_CLOSED);
	// void _shellfront_kill_child(GtkWindow *window, struct _shellfront_env_data *data)
	mock_env_data.child_pid = 123;
	_shellfront_kill_child(NULL, &mock_env_data);
	assert_test_state(TEST_STATE_PROCESS_KILLED);
	assert_test_state(TEST_STATE_CHILD_WAITED);
	// void _shellfront_window_destroy(GtkWindow *window, void *user_data)
	_shellfront_window_destroy(NULL, &mock_env_data);
	assert_test_state(TEST_STATE_EXITED);
	// void _shellfront_window_gravitate(int *window_width, int *window_height,
	// 	GdkRectangle *workarea, struct shellfront_term_conf *config)
	// grav = 9
	GdkRectangle workarea = { .x = 10, .y = 20, .width = 1590, .height = 1180 };
	struct shellfront_term_conf config = shellfront_term_conf_default;
	config.grav = 9;
	_shellfront_window_gravitate(640, 360, &workarea, &config);
	assert(config.x == 950 && config.y == 820);
	// grav = 5
	config.x = 0;
	config.y = 0;
	config.grav = 5;
	_shellfront_window_gravitate(640, 360, &workarea, &config);
	assert(config.x == 475 && config.y == 410);
	// grav = 1
	config.x = 0;
	config.y = 0;
	config.grav = 1;
	_shellfront_window_gravitate(640, 360, &workarea, &config);
	assert(config.x == 10 && config.y == 20);
}
