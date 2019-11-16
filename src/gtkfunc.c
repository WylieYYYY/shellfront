#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vte/vte.h>

// temporary lock file location, public to be seen by signal handler
static char *tmpid;

// change focus to the window
static void window_show(GtkWindow *window, void *user_data) { gtk_window_present(window); }
static void sig_exit(int signo) {
	// remove lock file and free the ID
	remove(tmpid);
	free(tmpid);
	exit(0);
}
// also give out signal to handler when closedd with "once" flag
static void window_destroy(GtkWindow *window, void *user_data) { sig_exit(2); }
// if terminal closed, close the window
static void terminal_exit(VteTerminal *terminal, int status, GtkWindow *window) { gtk_window_close(window); }
static void window_focus_out(GtkWidget *widget, GdkEvent *event, GtkWindow *window) { gtk_window_close(window); }

static void gtk_activate(GtkApplication *app, struct term_conf *config) {
	GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
	VteTerminal *terminal = VTE_TERMINAL(vte_terminal_new());
	
	// remove the lock file and free ID string when window destroy
	if (config->once) {
		g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), NULL);
	}
	
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
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(
		gdk_display_get_default()), &workarea);
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

struct term_conf shellfront_parse(int argc, char **argv) {
	// default configurations
	struct term_conf config = {
		.grav = 1,
		.title = "",
		.cmd = "echo -n Hello World!; sleep infinity",
	};
	char *loc = "0,0";
	char *size = "80x24";
	
	// options and help messages
	GOptionEntry options[] = {
		{
			.long_name = "once",
			.short_name = '1',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &config.once,
			.description = "Set if only one instance is allowed"
		}, {
			.long_name = "gravity",
			.short_name = 'g',
			.arg = G_OPTION_ARG_INT,
			.arg_data = &config.grav,
			.arg_description = "ENUM",
			.description = "Set gravity for window, see README for detail"
		}, {
			.long_name = "loc",
			.short_name = 'l',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &loc,
			.arg_description = "X,Y",
			.description = "Set the default screen location"
		}, {
			.long_name = "size",
			.short_name = 's',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &size,
			.arg_description = "XxY",
			.description = "Set the size"
		}, {
			.long_name = "title",
			.short_name = 't',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &config.title,
			.arg_description = "TITLE",
			.description = "Set the title for application window"
		}, {
			.long_name = "command",
			.short_name = 'c',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &config.cmd,
			.arg_description = "COMMAND",
			.description = "Set the command to be executed"
		}, {
			.long_name = "interactive",
			.short_name = 'i',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &config.interactive,
			.description = "Application can be interacted with mouse and auto focuses"
		}, {
			.long_name = "popup",
			.short_name = 'p',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &config.ispopup,
			.description = "Display as popup instead of application, implies -1"
		}, {
			.long_name = "toggle",
			.short_name = 'T',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &config.toggle,
			.description = "Toggle single instance application, implies -1"
		}, {
			.long_name = "kill",
			.short_name = 'k',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &config.killopt,
			.description = "Kill a single instance application according to command"
		}, {}
	};
	
	GError *error = NULL;
	// description and error report
	if (!gtk_init_with_args(&argc, &argv,
		"- simple frontend for shell scripts", options, NULL, &error)) {
		fprintf(stderr, "%s\n", error->message);
		exit(1);
	}
	// options validation
	if (!parse_loc_str(loc, &config.x, &config.y, ",")) {
		fprintf(stderr, "Incorrect location format, should be X,Y\n");
		exit(1);
	}
	if (!parse_size_str(size, &config.width, &config.height, "x")) {
		fprintf(stderr, "Incorrect size format, should be XxY\n");
		exit(1);
	}
	if (config.grav < 1 || config.grav > 9) {
		fprintf(stderr, "Incorrect gravity range, see README for usage\n");
		exit(1);
	}
	if ((config.toggle && config.killopt) || (strcmp(config.title, "") && config.ispopup)) {
		fprintf(stderr, "Conflicting arguments, see README for usage\n");
		exit(1);
	}
	// implied flag
	config.once |= (config.ispopup || config.toggle);
	
	return config;
}
int shellfront_initialize(struct term_conf config) {
	// get lock file name
	tmpid = sxprintf("/tmp/shellfront.%lu.lock", hash(config.cmd));
	// if it is killing by flag or toggle
	if (config.killopt || (config.toggle && access(tmpid, F_OK) != -1)) {
		// target must have "once" flag, so lock file must be accessible
		FILE *tmpfp = fopen(tmpid, "r");
		if (tmpfp == NULL) {
			fprintf(stderr, "No instance of application is running or it is not ran with -1\n");
			free(tmpid);
			return 1;
		}
		// HDB UUCP lock file format process ID must be no longer than 10 characters
		char buf[11];
		buf[10] = '\0';
		fread(buf, 1, 10, tmpfp);
		fclose(tmpfp);
		int pid = atoi(buf);
		// open the process description file
		char *procid = sxprintf("/proc/%i/comm", pid);
		FILE *procfp = fopen(procid, "r");
		free(procid);
		if (procfp == NULL) fprintf(stderr, "No such process found, use system kill tool\n");
		else {
			fread(buf, 1, 10, procfp);
			fclose(procfp);
			// if it is indeed belong to "shellfront", kill it
			if (strcmp(buf, "shellfront") == 0) kill(pid, SIGTERM);
			else fprintf(stderr, "PID mismatch in record, use system kill tool\n");
		}
		// remove lock file and free resource
		remove(tmpid);
		free(tmpid);
		return procfp == NULL;
	}
	
	// get this process's process ID
	int pid = getpid();
	if (config.once) {
		// write HDB UUCP lock file if ran with "once" flag
		FILE *tmpfp = fopen(tmpid, "wx");
		if (tmpfp == NULL) {
			fprintf(stderr, "Existing instance is running, remove -1 flag or '%s' to unlock\n", tmpid);
			free(tmpid);
			return 1;
		}
		int pidlen = snprintf(NULL, 0, "%i", pid);
		fprintf(tmpfp, "%*s%i\r\n", 10 - pidlen, "", pid);
		fclose(tmpfp);
		// clean up if terminated with SIGTERM or SIGINT (for terminal ^C)
		struct sigaction exithandler = { .sa_handler = sig_exit };
		sigaction(SIGINT, &exithandler, NULL);
		sigaction(SIGTERM, &exithandler, NULL);
		// free the lock file name because it is irrelevant for multiple instance application
	} else free(tmpid);
	
	// PID is guarenteed unique and can be used as APPID
	char *appid = sxprintf("shellfront.proc%i", pid);
	GtkApplication *app = gtk_application_new(appid, G_APPLICATION_FLAGS_NONE);
	free(appid);
	// link terminal setup function
	g_signal_connect(app, "activate", G_CALLBACK(gtk_activate), &config);
	int status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);
	return status;
}
