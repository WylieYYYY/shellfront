#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

void _shellfront_terminal_exit(VteTerminal *terminal, int status, GtkWindow *window);
void _shellfront_window_show(GtkWindow *window, void *user_data);
void _shellfront_window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window);
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
void mock_sig_exit(int signo) {
	assert(signo == 2);
	add_test_state(TEST_STATE_EXITED);
}

void test_gtkfunc_helper() {
	// void t_shellfront_erminal_exit(VteTerminal *terminal, int status, GtkWindow *window)
	_shellfront_terminal_exit(NULL, 0, test_window);
	assert_test_state(1, TEST_STATE_WINDOW_CLOSED);
	// void _shellfront_window_show(GtkWindow *window, void *user_data)
	_shellfront_window_show(test_window, NULL);
	assert_test_state(1, TEST_STATE_WINDOW_PRESENTED);
	// void _shellfront_window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window)
	_shellfront_window_focus_out(NULL, NULL, test_window);
	assert_test_state(1, TEST_STATE_WINDOW_CLOSED);
	// void _shellfront_window_destroy(GtkWindow *window, void *user_data)
	_shellfront_window_destroy(NULL, NULL);
	assert_test_state(1, TEST_STATE_EXITED);
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
