#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <vte/vte.h>

#ifdef UNIT_TEST
	void mock_gtk_window_close(GtkWindow *window);
	#define gtk_window_close(x) mock_gtk_window_close(x)
	void mock_gtk_window_present(GtkWindow *window);
	#define gtk_window_present(x) mock_gtk_window_present(x)
	void mock_sig_exit(int signo);
	#define _shellfront_sig_exit(x) mock_sig_exit(x)
	void mock_err_string_arg(FILE *fp, char *fmt, char *str);
	#define fprintf(x,y,z) mock_err_string_arg(x,y,z)
	GtkWidget *mock_gtk_application_window_new(GtkApplication *application);
	#define gtk_application_window_new(x) mock_gtk_application_window_new(x)
	extern VteTerminal *test_terminal;
	#define vte_terminal_new() GTK_WIDGET(test_terminal)
	void mock_vte_terminal_set_size(VteTerminal *terminal, long columns, long rows);
	#define vte_terminal_set_size(x,y,z) mock_vte_terminal_set_size(x,y,z)
	void mock_gtk_widget_show_all(GtkWidget *widget);
	#define gtk_widget_show_all(x) mock_gtk_widget_show_all(x)
	void mock_vte_terminal_spawn_async(VteTerminal *terminal, VtePtyFlags pty_flags,
		const char *working_directory, char **argv, char **envv, GSpawnFlags spawn_flags_,
		GSpawnChildSetupFunc child_setup, void *child_setup_data,
		GDestroyNotify child_setup_data_destroy, int timeout, GCancellable *cancellable,
		VteTerminalSpawnAsyncCallback callback, void *user_data);
	#define vte_terminal_spawn_async(a,b,c,d,e,f,g,h,i,j,k,l,m) mock_vte_terminal_spawn_async(a,b,c,d,e,f,g,h,i,j,k,l,m)
#endif

struct err_state *_shellfront_fork_state = NULL;

// change focus to the window
void _shellfront_window_show(GtkWindow *window, void *user_data) {
	gtk_window_present(window);
}
// also give out signal to handler when closed with "once" flag
void _shellfront_window_destroy(GtkWindow *window, void *user_data) {
	_shellfront_sig_exit(2);
}
// if terminal closed, close the window
void _shellfront_terminal_exit(VteTerminal *terminal, int status, GtkWindow *window) {
	gtk_window_close(window);
}
void _shellfront_window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window) {
	gtk_window_close(window);
}

void _shellfront_window_gravitate(int window_width, int window_height,
	GdkRectangle *workarea, struct shellfront_term_conf *config) {
	// calculations with gravity setting
	if (config->grav % 3 == 0) config->x = (workarea->width - window_width) - config->x;
	else if (config->grav % 3 == 2) config->x = (workarea->width - window_width) / 2;
	// take horizontal taskbar into consideration
	else config->x += workarea->x;
	if (config->grav > 6) config->y = (workarea->height - window_height) - config->y;
	else if (config->grav > 3) config->y = (workarea->height - window_height) / 2;
	// take vertical taskbar into consideration
	else config->y += workarea->y;
}
void _shellfront_apply_opt(GtkWindow *window, VteTerminal *terminal, struct shellfront_term_conf *config) {
	// remove the lock file and free ID string when window destroy
	if (config->once) g_signal_connect(window, "destroy", G_CALLBACK(_shellfront_window_destroy), NULL);

	// don't give any attention if it is not interactive
	if (!config->interactive) gtk_widget_set_sensitive(GTK_WIDGET(terminal), FALSE);

	// all type of window is not resizable
	if (config->popup) {
		// no title bar and icon
		gtk_window_set_decorated(window, FALSE);
		gtk_window_set_skip_taskbar_hint(window, TRUE);
		// close popup when focus is out
		gtk_widget_set_events(GTK_WIDGET(window), GDK_FOCUS_CHANGE_MASK);
		g_signal_connect(GTK_WIDGET(window), "focus-out-event", G_CALLBACK(_shellfront_window_focus_out), window);
	} else gtk_window_set_resizable(window, FALSE);
	if (*config->icon != '\0') {
		GError *gtkerr = NULL;
		gtk_window_set_icon_from_file(window, config->icon, &gtkerr);
		if (gtkerr != NULL) fprintf(stderr, "%s\n", gtkerr->message);
		g_clear_error(&gtkerr);
	}
}
void _shellfront_configure_terminal(VteTerminal *terminal, struct _shellfront_env_data *data) {
	if (data->is_integrate) {
		VtePty *pty = vte_pty_new_sync(VTE_PTY_DEFAULT, NULL, NULL);//TODO: Handle Error
		vte_terminal_set_pty(terminal, pty);
		pid_t pid = fork();
		if (pid > 0) return;
		// define _shellfront_fork_state to indicate forked
		_shellfront_fork_state = &((struct err_state) { .has_error = 0, .errmsg = "" });
		struct err_state fork_error = define_error(_("Fork error"));
		if (pid == -1) _shellfront_fork_state = &fork_error;
		// redirect all IO
		fflush(stderr);
		int parent_stderr_fd = dup(2); //FIXME: Handle dup error
		vte_pty_child_setup(pty);
		dup2(parent_stderr_fd, 2);
		main(data->argc, data->argv);
		//FIXME: Terminate gracefully
	} else {
		// using the default terminal shell
		char *shell = vte_get_user_shell();
		char **argv = (char *[]){ shell, "-c", data->term_conf->cmd, NULL };
		// no timeout for command
		vte_terminal_spawn_async(terminal, VTE_PTY_DEFAULT, NULL, argv, NULL,
			G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, -1, NULL, NULL, NULL);
		free(shell);
	}
}

void _shellfront_gtk_activate(GtkApplication *app, struct _shellfront_env_data *data) {
	struct shellfront_term_conf *config = data->term_conf;
	GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
	VteTerminal *terminal = VTE_TERMINAL(vte_terminal_new());

	_shellfront_apply_opt(window, terminal, config);

	// set the window title
	gtk_window_set_title(window, config->title);
	// change focus to window when shown
	g_signal_connect(window, "show", G_CALLBACK(_shellfront_window_show), NULL);

	vte_terminal_set_size(terminal, config->width, config->height);
	// when command ends, terminal exits
	g_signal_connect(terminal, "child-exited", G_CALLBACK(_shellfront_terminal_exit), window);
	_shellfront_configure_terminal(terminal, data);

	// put the terminal into the window and show both
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(terminal));
	gtk_widget_show_all(GTK_WIDGET(window));

	GdkRectangle workarea;
	// get the available area of the screen
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	int window_width, window_height;
	// move position after displaying because the exact size is not determined before displaying
	gtk_window_get_size(window, &window_width, &window_height);
	_shellfront_window_gravitate(window_width, window_height, &workarea, config);
	gtk_window_move(window, config->x, config->y);
}
