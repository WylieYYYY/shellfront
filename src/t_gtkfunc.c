#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>
#include <vte/vte.h>

void test_gtkfunc_helper(void);

void apply_opt(GtkWindow *window, VteTerminal *terminal, struct term_conf *config);
void window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window);
void window_destroy(GtkWindow *window, void *user_data);
void gtk_activate(GtkApplication *app, struct term_conf *config);
void window_show(GtkWindow *window, void *user_data);
void terminal_exit(VteTerminal *terminal, int status, GtkWindow *window);

static int test_state;
static GtkWindow *window;
VteTerminal *test_terminal;
GtkWidget *mock_gtk_application_window_new(GtkApplication *application) {
	window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	return GTK_WIDGET(window);
}
void mock_vte_terminal_set_size(VteTerminal *terminal, long columns, long rows) {
	assert(terminal == test_terminal);
	assert(columns == 80 && rows == 24);
	test_state++;
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
	test_state++;
}
void mock_gtk_widget_show_all(GtkWidget *widget) {
	// test once, interactive
	assert(widget == GTK_WIDGET(window));
	unsigned int sigid = g_signal_lookup("destroy", G_TYPE_FROM_INSTANCE(window));
	unsigned long sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC, sigid, 0, NULL, &window_destroy, NULL);
	assert(sighandler != 0);
	assert(!gtk_window_get_resizable(window));
	// test other config
	assert(strcmp(gtk_window_get_title(window), "Title") == 0);
	sigid = g_signal_lookup("show", G_TYPE_FROM_INSTANCE(window));
	sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC, sigid, 0, NULL, &window_show, NULL);
	assert(sighandler != 0);
	sigid = g_signal_lookup("child-exited", G_TYPE_FROM_INSTANCE(test_terminal));
	sighandler = g_signal_handler_find(test_terminal,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		sigid, 0, NULL, &terminal_exit, window);
	assert(sighandler != 0);
	GList *children = gtk_container_get_children(GTK_CONTAINER(window));
	assert(VTE_TERMINAL(children->data) == test_terminal);
	g_list_free(children);
	assert(test_state == 2);
	test_state++;
}

void test_gtkfunc() {
	test_gtkfunc_helper();
	// void apply_opt(GtkWindow *window, VteTerminal *terminal, struct term_conf *config)
	// test ispopup
	gtk_init(0, NULL);
	struct term_conf config = term_conf_default;
	config.ispopup = 1;
	window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	test_terminal = VTE_TERMINAL(vte_terminal_new());
	apply_opt(window, test_terminal, &config);
	assert(!gtk_widget_get_sensitive(GTK_WIDGET(test_terminal)));
	assert(!gtk_window_get_decorated(window));
	assert(gtk_window_get_skip_taskbar_hint(window));
	unsigned int sigid = g_signal_lookup("focus-out-event", G_TYPE_FROM_INSTANCE(window));
	unsigned long sighandler = g_signal_handler_find(window,
		G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		sigid, 0, NULL, &window_focus_out, window);
	assert(sighandler != 0);
	gtk_widget_destroy(GTK_WIDGET(window));
	// void gtk_activate(GtkApplication *app, struct term_conf *config)
	config.once = 1;
	config.interactive = 1;
	config.ispopup = 0;
	config.title = "Title";
	config.cmd = "command";
	config.grav = 5;
	gtk_activate(NULL, &config);
	assert(test_state == 3);
	int x, y;
	gtk_window_get_position(window, &x, &y);
	assert(x != 0 && y != 0);
	gtk_widget_destroy(GTK_WIDGET(test_terminal));
	gtk_widget_destroy(GTK_WIDGET(window));
}