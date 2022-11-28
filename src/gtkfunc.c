#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vte/vte.h>

#ifdef UNIT_TEST
	#include "test.h"
	#include <string.h>
	void mock_gtk_window_close(GtkWindow *window);
	#define gtk_window_close(x) mock_gtk_window_close(x)
	void mock_gtk_window_present(GtkWindow *window);
	#define gtk_window_present(x) mock_gtk_window_present(x)
	int mock_kill(pid_t pid, int sig);
	#define kill(x,y) mock_kill(x,y)
	pid_t mock_waitpid(pid_t pid, int *status, int options);
	#define waitpid(x,y,z) mock_waitpid(x,y,z)
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
	void mock_configure_terminal(VteTerminal *terminal, struct _shellfront_env_data *data);
	#define _SHELLFRONT_CONFIGURE_TERMINAL(x,y) mock_configure_terminal(x,y)
	#define vte_get_user_shell() strdup("/bin/bash")
	int mock_main(int argc, char **argv);
	#define main(x,y) mock_main(x,y)
	#define dup(x) test_state_contains(TEST_STATE_WILL_RETURN_ERROR)?-1:dup(x)
	void mock_vte_pty_child_setup(VtePty *pty);
	#define vte_pty_child_setup(x) mock_vte_pty_child_setup(x)
	int mock_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
	#define sigaction(x,y,z) mock_sigaction(x,y,z)
	void mock_exit(int status);
	#define exit(x) mock_exit(x)
	VtePty *mock_vte_pty_new_sync(VtePtyFlags flags, GCancellable *cancellable, GError **error);
	#define vte_pty_new_sync(x,y,z) mock_vte_pty_new_sync(x,y,z)
	pid_t mock_fork(VteTerminal *terminal);
	#define fork() mock_fork(terminal)
	void mock__shellfront_child_process_setup(int pid, VtePty *pty, struct _shellfront_env_data *data);
	#define _SHELLFRONT_CHILD_PROCESS_SETUP(x,y,z) mock__shellfront_child_process_setup(x,y,z)
#else
	#define _SHELLFRONT_CONFIGURE_TERMINAL(x,y) _shellfront_configure_terminal(x,y)
	#define _SHELLFRONT_CHILD_PROCESS_SETUP(x,y,z) _shellfront_child_process_setup(x,y,z)
#endif

struct err_state _shellfront_fork_state = {};

// change focus to the window
void _shellfront_window_show(GtkWindow *window, void *user_data) {
	gtk_window_present(window);
}
void _shellfront_kill_child(GtkWindow *window, struct _shellfront_env_data *data) {
	kill(data->child_pid, SIGTERM);
	waitpid(data->child_pid, NULL, WNOHANG);
}
// also give out signal to handler when closed with "once" flag
void _shellfront_window_destroy(GtkWindow *window, struct _shellfront_env_data *data) {
	_shellfront_kill_child(window, data);
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
void _shellfront_apply_opt(GtkWindow *window, VteTerminal *terminal,
	struct shellfront_term_conf *config, struct _shellfront_env_data *data) {
	// remove the lock file and free ID string when window destroy
	if (config->once) g_signal_connect(window, "destroy", G_CALLBACK(_shellfront_window_destroy), data);
	else g_signal_connect(window, "destroy", G_CALLBACK(_shellfront_kill_child), data);

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
void _shellfront_child_process_setup(int pid, VtePty *pty, struct _shellfront_env_data *data) {
	if (pid == -1) _shellfront_fork_state = define_error(_("Fork error"));
	_shellfront_fork_state._forked = 1;
	// redirect all IO
	fflush(stderr);
	int parent_stderr_fd = dup(2);
	vte_pty_child_setup(pty);
	if (parent_stderr_fd != -1) dup2(parent_stderr_fd, 2);
	main(data->argc, data->argv);
	if (data->term_conf->once) _shellfront_sig_exit(2);
	else exit(0);
}
void _shellfront_configure_terminal(VteTerminal *terminal, struct _shellfront_env_data *data) {
	if (data->is_integrate) {
		GError *gtkerr = NULL;
		VtePty *pty = vte_pty_new_sync(VTE_PTY_DEFAULT, NULL, &gtkerr);
		// parent process will not check this variable, safe to define here to indicate fork
		_shellfront_fork_state = _shellfront_gerror_to_err_state(gtkerr);
		vte_terminal_set_pty(terminal, pty);
		pid_t pid = fork();
		if (pid <= 0) _SHELLFRONT_CHILD_PROCESS_SETUP(pid, pty, data);
		data->child_pid = pid;
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

	_shellfront_apply_opt(window, terminal, config, data);

	// set the window title
	gtk_window_set_title(window, config->title);
	// change focus to window when shown
	g_signal_connect(window, "show", G_CALLBACK(_shellfront_window_show), NULL);

	vte_terminal_set_size(terminal, config->width, config->height);
	// when command ends, terminal exits
	g_signal_connect(terminal, "child-exited", G_CALLBACK(_shellfront_terminal_exit), window);
	_SHELLFRONT_CONFIGURE_TERMINAL(terminal, data);

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
