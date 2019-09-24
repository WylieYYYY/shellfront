#include <gtk/gtk.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vte/vte.h>

char *tmpid;
struct term_conf {
	unsigned int grav;
	int x, y;
	long width, height;
	char *title;
	char *cmd;
	int interactive;
	int ispopup;
	int once;
};

static int parse_size_str(char *size, long *x, long *y, char *delim) {
	char *cp = strdup(size);
	*x = atol(strsep(&cp, delim));
	*y = atol(cp);
	return !(*x < 1 || *y < 1);
}
static int parse_loc_str(char *size, int *x, int *y, char *delim) {
	char *cp = strdup(size);
	*x = atoi(strsep(&cp, delim));
	*y = atoi(cp);
	return !(*x < 0 || *y < 0);
}
static unsigned long hash(char *str) {
	unsigned long hash = 5381;
	unsigned char ch;
    while ((ch = (unsigned char)*str++)) hash = ((hash << 5) + hash) + ch;
    return hash;
}
static char *sxprintf(char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	int size = vsnprintf(NULL, 0, fmt, argptr) + 1;
	char *str = malloc(size);
	va_start(argptr, fmt);
	vsnprintf(str, size, fmt, argptr);
	va_end(argptr);
	return str;
}

static void window_show(GtkWindow *window, void *user_data) { gtk_window_present(window); }
static void sig_exit(int signo) {
	remove(tmpid);
	free(tmpid);
	exit(0);
}
static void window_destroy(GtkWindow *window, void *user_data) { sig_exit(2); }
static void terminal_exit(VteTerminal *terminal, int status, GtkWindow *window) { gtk_window_close(window); }

static void activate(GtkApplication *app, struct term_conf *config) {
	GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
	VteTerminal *terminal = VTE_TERMINAL(vte_terminal_new());
	
	if (config->once) {
		g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), NULL);
	}
	
	if (!config->interactive) gtk_widget_set_sensitive(GTK_WIDGET(terminal), FALSE);
	else g_signal_connect(window, "show", G_CALLBACK(window_show), NULL);
	if (config->ispopup) {
		gtk_window_set_decorated(window, FALSE);
		gtk_window_set_skip_taskbar_hint(window, TRUE);
	} else gtk_window_set_resizable(window, FALSE);
	gtk_window_set_title(window, config->title);

	vte_terminal_set_size(terminal, config->width, config->height);
	g_signal_connect(terminal, "child-exited", G_CALLBACK(terminal_exit), window);
	char **argv = (char *[]){(char *)g_getenv("SHELL"), "-c", config->cmd, NULL};
	vte_terminal_spawn_async(terminal, VTE_PTY_DEFAULT, NULL, argv, NULL,
		G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, -1, NULL, NULL, NULL);

	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(terminal));
	gtk_widget_show_all(GTK_WIDGET(window));
	
	GdkRectangle workarea;
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(
		gdk_display_get_default()), &workarea);
	int window_width, window_height;
	gtk_window_get_size(window, &window_width, &window_height);
	if (config->grav % 3 == 0) config->x = (workarea.width - window_width) - config->x;
	else if (config->grav % 3 == 2) config->x = (workarea.width - window_width) / 2;
	if (config->grav > 6) config->y = (workarea.height - window_height) - config->y;
	else if (config->grav > 3) config->y = (workarea.height - window_height) / 2;
	gtk_window_move(window, config->x, config->y);
	if (config->ispopup) gtk_window_set_keep_above(window, TRUE);
}

int main(int argc, char **argv) {
	struct term_conf config = {
		.grav = 1,
		.title = "",
		.cmd = "echo -n Hello World!; sleep infinity",
		.interactive = FALSE
	};
	char *loc = "0,0";
	char *size = "80x24";
	int toggle = FALSE;
	int killopt = FALSE;
	
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
			.arg_data = &toggle,
			.description = "Toggle single instance application, implies -1"
		}, {
			.long_name = "kill",
			.short_name = 'k',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &killopt,
			.description = "Kill a single instance application according to command"
		}, {}
	};
	
	GError *error = NULL;
	if (!gtk_init_with_args(&argc, &argv,
		"- simple frontend for shell scripts", options, NULL, &error)) {
		fprintf(stderr, "%s\n", error->message);
		return 1;
	}
	if (!parse_loc_str(loc, &config.x, &config.y, ",")) {
		fprintf(stderr, "Incorrect location format, should be X,Y\n");
		return 1;
	}
	if (!parse_size_str(size, &config.width, &config.height, "x")) {
		fprintf(stderr, "Incorrect size format, should be XxY\n");
		return 1;
	}
	if (config.grav < 1 || config.grav > 9) {
		fprintf(stderr, "Incorrect gravity range, see README for usage\n");
		return 1;
	}
	if ((toggle && killopt) || (strcmp(config.title, "") && config.ispopup)) {
		fprintf(stderr, "Conflicting arguments, see README for usage\n");
		return 1;
	}
	config.once |= (config.ispopup || toggle);
	
	tmpid = sxprintf("/tmp/shellfront.%lu.lock", hash(config.cmd));
	if (killopt || (toggle && access(tmpid, F_OK) != -1)) {
		FILE *tmpfp = fopen(tmpid, "r");
		if (tmpfp == NULL) {
			fprintf(stderr, "No instance of application is running or it is not ran with -1\n");
			free(tmpid);
			return 1;
		}
		char *buf = malloc(11);
		buf[10] = '\0';
		fread(buf, 1, 10, tmpfp);
		fclose(tmpfp);
		int pid = atoi(buf);
		char *procid = sxprintf("/proc/%i/comm", pid);
		FILE *procfp = fopen(procid, "r");
		free(procid);
		if (procfp == NULL) fprintf(stderr, "No such process found, use system kill tool\n");
		else {
			fread(buf, 1, 10, procfp);
			fclose(procfp);
			if (strcmp(buf, "shellfront") == 0) kill(pid, SIGTERM);
			else fprintf(stderr, "PID mismatch in record, use system kill tool\n");
		}
		remove(tmpid);
		free(buf);
		free(tmpid);
		return procfp == NULL;
	}
	
	int pid = getpid();
	if (config.once) {
		FILE *tmpfp = fopen(tmpid, "wx");
		if (tmpfp == NULL) {
			fprintf(stderr, "Existing instance is running, remove -1 flag or '%s' to unlock\n", tmpid);
			free(tmpid);
			return 1;
		}
		int pidlen = snprintf(NULL, 0, "%i", pid);
		fprintf(tmpfp, "%*s%i\r\n", 10 - pidlen, "", pid);
		fclose(tmpfp);
		struct sigaction exithandler = { .sa_handler = sig_exit };
		sigaction(SIGINT, &exithandler, NULL);
		sigaction(SIGTERM, &exithandler, NULL);
	} else free(tmpid);
	
	char *appid = sxprintf("shellfront.proc%i", pid);
	GtkApplication *app = gtk_application_new(appid, G_APPLICATION_FLAGS_NONE);
	free(appid);
	g_signal_connect(app, "activate", G_CALLBACK(activate), &config);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}