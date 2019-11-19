#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <signal.h>
#include <stdlib.h>
#include <vte/vte.h>

// change focus to the window
static void window_show(GtkWindow *window, void *user_data) {
	gtk_window_present(window);
}

// also give out signal to handler when closedd with "once" flag
static void window_destroy(GtkWindow *window, void *user_data) {
	sig_exit(2);
}
// if terminal closed, close the window
static void terminal_exit(VteTerminal *terminal, int status, GtkWindow *window) {
	gtk_window_close(window);
}
static void window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window) {
	gtk_window_close(window);
}

void gtk_activate(GtkApplication *app, struct term_conf *config) {
	GtkWindow *window = GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(app)));
	VteTerminal *terminal = VTE_TERMINAL(vte_terminal_new());
	
	// remove the lock file and free ID string when window destroy
	if (config->once) g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), NULL);
	
	// don't give any attention if it is not interactive
	if (!config->interactive) gtk_widget_set_sensitive(GTK_WIDGET(terminal), FALSE);
	
	// all type of window is not resizable
	if (config->ispopup) {
		// no title bar and icon
		gtk_window_set_decorated(window, FALSE);
		gtk_window_set_skip_taskbar_hint(window, TRUE);
		// close popup when focus is out
		gtk_widget_set_events(GTK_WIDGET(window), GDK_FOCUS_CHANGE_MASK);
		g_signal_connect(GTK_WIDGET(window), "focus-out-event", G_CALLBACK(window_focus_out), window);
	} else gtk_window_set_resizable(window, FALSE);
	
	// set the window title
	gtk_window_set_title(window, config->title);
	// change focus to window when shown
	g_signal_connect(window, "show", G_CALLBACK(window_show), NULL);

	vte_terminal_set_size(terminal, config->width, config->height);
	// when command ends, terminal exits
	g_signal_connect(terminal, "child-exited", G_CALLBACK(terminal_exit), window);
	// using the default terminal shell
	char **argv = (char *[]){(char *)g_getenv("SHELL"), "-c", config->cmd, NULL};
	// no timeout for command
	vte_terminal_spawn_async(terminal, VTE_PTY_DEFAULT, NULL, argv, NULL,
		G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, -1, NULL, NULL, NULL);

	// put the terminal into the window and show both
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(terminal));
	gtk_widget_show_all(GTK_WIDGET(window));
	
	GdkRectangle workarea;
	// get the available area of the screen
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	int window_width, window_height;
	// move position after displaying because the exact size is not determined before displaying
	gtk_window_get_size(window, &window_width, &window_height);
	// calculations with gravity setting
	if (config->grav % 3 == 0) config->x = (workarea.width - window_width) - config->x;
	else if (config->grav % 3 == 2) config->x = (workarea.width - window_width) / 2;
	// take horizontal taskbar into consideration
	else config->x += workarea.x;
	if (config->grav > 6) config->y = (workarea.height - window_height) - config->y;
	else if (config->grav > 3) config->y = (workarea.height - window_height) / 2;
	// take vertical taskbar into consideration
	else config->y += workarea.y;
	gtk_window_move(window, config->x, config->y);
	// keep the popup on top
	if (config->ispopup) gtk_window_set_keep_above(window, TRUE);
}
