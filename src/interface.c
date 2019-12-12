#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNIT_TEST
	#define getpid() 123
	int mock_g_application_run(GApplication *application, int argc, char **argv);
	#define g_application_run(x,y,z) mock_g_application_run(x,y,z)
	struct err_state mock_unlock_process(void);
	#define UNLOCK_PROCESS() mock_unlock_process()
	struct err_state mock_lock_process(int pid);
	#define LOCK_PROCESS(x) mock_lock_process(x)
	int mock_access(const char *pathname, int mode);
	#define access(x,y) mock_access(x,y)
	int mock_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
	#define sigaction(x,y,z) mock_sigaction(x,y,z)
#else
	#define UNLOCK_PROCESS() unlock_process()
	#define LOCK_PROCESS(x) lock_process(x)
#endif

// temporary lock file location, public to be seen by signal handler
char *tmpid;

void sig_exit(int signo) {
	// remove lock file and free the ID
	remove(tmpid);
	free(tmpid);
	exit(0);
}

struct err_state validate_opt(char *locstr, char *sizestr, struct term_conf *config, GError *gtkerr) {
	if (!parse_loc_str(locstr, &(config->x), &(config->y))) {
		return define_error("Incorrect location format, should be X,Y");
	}
	if (!parse_size_str(sizestr, &(config->width), &(config->height))) {
		return define_error("Incorrect size format, should be XxY");
	}
	if (config->grav < 1 || config->grav > 9) {
		return define_error("Incorrect gravity range, see README for usage");
	}
	if ((config->toggle && config->killopt) || (strcmp(config->title, "") && config->ispopup)) {
		return define_error("Conflicting arguments, see README for usage");
	}
	// implied flag
	config->once |= (config->ispopup || config->toggle);
	// return propagated GTK error if there is any
	if (gtkerr == NULL) return ((struct err_state) { .has_error = 0, .errmsg = "" });
	struct err_state state = { .has_error = gtkerr->code };
	strcpy(state.errmsg, gtkerr->message);
	return state;
}

struct err_state shellfront_parse(int argc, char **argv, struct term_conf *config) {
	// default configurations
	*config = term_conf_default;
	config->cmd = "echo 'Hello World!'; echo 'Press Enter To Exit...'; read";
	config->interactive = 1;
	char *locstr = "0,0";
	char *sizestr = "80x24";
	
	// options and help messages
	GOptionEntry options[] = {
		{
			.long_name = "once",
			.short_name = '1',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->once),
			.description = "Set if only one instance is allowed"
		}, {
			.long_name = "grav",
			.short_name = 'g',
			.arg = G_OPTION_ARG_INT,
			.arg_data = &(config->grav),
			.arg_description = "ENUM",
			.description = "Set gravity for window, see README for detail"
		}, {
			.long_name = "loc",
			.short_name = 'l',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &locstr,
			.arg_description = "X,Y",
			.description = "Set the default screen location"
		}, {
			.long_name = "size",
			.short_name = 's',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &sizestr,
			.arg_description = "XxY",
			.description = "Set the size"
		}, {
			.long_name = "title",
			.short_name = 't',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->title),
			.arg_description = "TITLE",
			.description = "Set the title for application window"
		}, {
			.long_name = "cmd",
			.short_name = 'c',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->cmd),
			.arg_description = "COMMAND",
			.description = "Set the command to be executed"
		}, {
			.long_name = "interactive",
			.short_name = 'i',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->interactive),
			.description = "Application can be interacted with mouse and auto focuses"
		}, {
			.long_name = "ispopup",
			.short_name = 'p',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->ispopup),
			.description = "Display as popup instead of application, implies -1"
		}, {
			.long_name = "toggle",
			.short_name = 'T',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->toggle),
			.description = "Toggle single instance application, implies -1"
		}, {
			.long_name = "killopt",
			.short_name = 'k',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->killopt),
			.description = "Kill a single instance application according to command"
		}, { 0 }
	};

	GError *gtkerr = NULL;
	// description and register arguments
	gtk_init_with_args(&argc, &argv, "- simple frontend for shell scripts", options, NULL, &gtkerr);
	struct err_state state = validate_opt(locstr, sizestr, config, gtkerr);
	g_clear_error(&gtkerr);

	return state;
}

struct err_state unlock_process(void) {
	// target must have "once" flag, so lock file must be accessible
	FILE *tmpfp = fopen(tmpid, "r");
	if (tmpfp == NULL) {
		free(tmpid);
		return define_error("No instance of application is running or it is not ran with -1");
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
	struct err_state state;
	if (procfp == NULL) state = define_error("No such process found, use system kill tool");
	else {
		fread(buf, 1, 10, procfp);
		fclose(procfp);
		// if it is indeed belong to "shellfront", kill it
		if (strcmp(buf, "shellfront") == 0) kill(pid, SIGTERM);
		else state = define_error("PID mismatch in record, use system kill tool");
	}
	// remove lock file and free resource
	remove(tmpid);
	free(tmpid);
	return state;
}
struct err_state lock_process(int pid) {
	// write HDB UUCP lock file if ran with "once" flag
	FILE *tmpfp = fopen(tmpid, "wx");
	if (tmpfp == NULL) {
		char *msg = sxprintf("Existing instance is running, remove -1 flag or '%s' to unlock", tmpid);
		struct err_state state = define_error(msg);
		free(tmpid);
		free(msg);
		return state;
	}
	int pidlen = snprintf(NULL, 0, "%i", pid);
	fprintf(tmpfp, "%*s%i\r\n", 10 - pidlen, "", pid);
	fclose(tmpfp);
	// clean up if terminated with SIGTERM or SIGINT (for terminal ^C)
	struct sigaction exithandler = { .sa_handler = sig_exit };
	sigaction(SIGINT, &exithandler, NULL);
	sigaction(SIGTERM, &exithandler, NULL);

	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
struct err_state shellfront_initialize(struct term_conf *config) {
	// get lock file name
	tmpid = sxprintf("/tmp/shellfront.%lu.lock", hash(config->cmd));
	// if it is killing by flag or toggle
	if (config->killopt || (config->toggle && access(tmpid, F_OK) != -1)) return UNLOCK_PROCESS();
	
	// get this process's process ID
	int pid = getpid();
	if (config->once) {
		struct err_state state = LOCK_PROCESS(pid);
		if (state.has_error) return state;
		// free the lock file name because it is irrelevant for multiple instance application
	} else free(tmpid);
	
	// PID is guarenteed unique and can be used as APPID
	char *appid = sxprintf("shellfront.proc%i", pid);
	GtkApplication *app = gtk_application_new(appid, G_APPLICATION_FLAGS_NONE);
	free(appid);
	// link terminal setup function
	g_signal_connect(app, "activate", G_CALLBACK(gtk_activate), config);
	struct err_state state = { .has_error = 0, .errmsg = "" };
	state.has_error = g_application_run(G_APPLICATION(app), 0, NULL);
	if (state.has_error) strcpy(state.errmsg, "GTK error");
	g_object_unref(app);
	return state;
}
