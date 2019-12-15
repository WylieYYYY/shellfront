#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

void terminal_exit(VteTerminal *terminal, int status, GtkWindow *window);
void window_show(GtkWindow *window, void *user_data);
void window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window);
void window_destroy(GtkWindow *window, void *user_data);
void window_gravitate(int window_width, int window_height,
	GdkRectangle *workarea, struct term_conf *config);

static int test_state;
static GtkWindow *test_window = (GtkWindow *)0xDEADBEEF;
void mock_gtk_window_close(GtkWindow *window) {
	assert(window == (GtkWindow *)0xDEADBEEF);
	test_state = 1;
}
void mock_gtk_window_present(GtkWindow *window) {
	assert(window == (GtkWindow *)0xDEADBEEF);
	test_state = 0;
}
void mock_sig_exit(int signo) {
	assert(signo == 2);
	test_state = 2;
}

void test_gtkfunc_helper() {
	// void terminal_exit(VteTerminal *terminal, int status, GtkWindow *window)
	terminal_exit(NULL, 0, test_window);
	assert(test_state == 1);
	// void window_show(GtkWindow *window, void *user_data)
	window_show(test_window, NULL);
	assert(test_state == 0);
	// void window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window)
	window_focus_out(NULL, NULL, test_window);
	assert(test_state == 1);
	// void window_destroy(GtkWindow *window, void *user_data)
	window_destroy(NULL, NULL);
	assert(test_state == 2);
	// void window_gravitate(int *window_width, int *window_height,
	// 	GdkRectangle *workarea, struct term_conf *config)
	// grav = 9
	GdkRectangle workarea = { .x = 10, .y = 20, .width = 1590, .height = 1180 };
	struct term_conf config = term_conf_default;
	config.grav = 9;
	window_gravitate(640, 360, &workarea, &config);
	assert(config.x == 950 && config.y == 820);
	// grav = 5
	config.x = 0;
	config.y = 0;
	config.grav = 5;
	window_gravitate(640, 360, &workarea, &config);
	assert(config.x == 475 && config.y == 410);
	// grav = 1
	config.x = 0;
	config.y = 0;
	config.grav = 1;
	window_gravitate(640, 360, &workarea, &config);
	assert(config.x == 10 && config.y == 20);
}