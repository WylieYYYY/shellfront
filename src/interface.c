#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNIT_TEST
	#define getpid() 123
	int mock_g_application_run(GApplication *application, int argc, char **argv);
	#define g_application_run(x,y,z) mock_g_application_run(x,y,z)
	#define _shellfront_prepare_hashable(x,y,z) (strdup(x))
	struct err_state mock_unlock_process(char *exe_name);
	#define _SHELLFRONT_UNLOCK_PROCESS(x) mock_unlock_process(x)
	struct err_state mock_lock_process(int pid);
	#define _SHELLFRONT_LOCK_PROCESS(x) mock_lock_process(x)
	int mock_access(const char *pathname, int mode);
	#define access(x,y) mock_access(x,y)
	int mock_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
	#define sigaction(x,y,z) mock_sigaction(x,y,z)
	FILE *mock_proc_fopen(const char *filename, const char *mode);
	#define FOPEN(x,y) mock_proc_fopen(x,y)
	int mock_kill(pid_t pid, int sig);
	#define kill(x,y) mock_kill(x,y)
	void mock_exit(int status);
	#define exit(x) mock_exit(x)
#else
	#define _SHELLFRONT_UNLOCK_PROCESS(x) _shellfront_unlock_process(x)
	#define _SHELLFRONT_LOCK_PROCESS(x) _shellfront_lock_process(x)
	#define FOPEN(x,y) fopen(x,y)
#endif

// temporary lock file location, public to be seen by signal handler
char *_shellfront_tmpid;

void _shellfront_sig_exit(int signo) {
	// remove lock file and free the ID
	remove(_shellfront_tmpid);
	free(_shellfront_tmpid);
	exit(0);
}

struct err_state _shellfront_unlock_process(char *exe_name) {
	// target must have "once" flag, so lock file must be accessible
	FILE *tmpfp = fopen(_shellfront_tmpid, "r");
	if (tmpfp == NULL) {
		free(exe_name);
		free(_shellfront_tmpid);
		return define_error(_("No instance of application is running or it is not ran with -1"));
	}
	// HDB UUCP lock file format process ID must be no longer than 10 characters
	char pid_buf[11];
	pid_buf[10] = '\0';
	fread(pid_buf, 1, 10, tmpfp);
	fclose(tmpfp);
	// don't actually care much about validation, all will be clear with the description
	int pid = strtol(pid_buf, NULL, 10);
	// open the process description file
	char *procid = sxprintf("/proc/%i/comm", pid);
	FILE *procfp = FOPEN(procid, "r");
	free(procid);
	struct err_state state = { .has_error = 0, .errmsg = "" };
	if (procfp == NULL) state = define_error(_("No such process found, use system kill tool"));
	else {
		char *name_buf = malloc(strlen(exe_name) + 1);
		name_buf[strlen(exe_name)] = '\0';
		fread(name_buf, 1, strlen(exe_name), procfp);
		fclose(procfp);
		// if it is indeed belong to the executable, kill it
		if (strcmp(name_buf, exe_name) == 0) kill(pid, SIGTERM);
		else state = define_error(_("PID mismatch in record, use system kill tool"));
		free(name_buf);
	}
	free(exe_name);
	// remove lock file and free resource
	remove(_shellfront_tmpid);
	free(_shellfront_tmpid);
	return state;
}
struct err_state _shellfront_lock_process(int pid) {
	// write HDB UUCP lock file if ran with "once" flag
	FILE *tmpfp = fopen(_shellfront_tmpid, "wx");
	if (tmpfp == NULL) {
		char *msg = sxprintf(_("Existing instance is running, remove -1 flag or '%s' to unlock"), _shellfront_tmpid);
		struct err_state state = define_error(msg);
		free(_shellfront_tmpid);
		free(msg);
		return state;
	}
	int pidlen = snprintf(NULL, 0, "%i", pid);
	fprintf(tmpfp, "%*s%i\r\n", 10 - pidlen, "", pid);
	fclose(tmpfp);
	// clean up if terminated with SIGTERM or SIGINT (for terminal ^C)
	struct sigaction exithandler = { .sa_handler = _shellfront_sig_exit };
	sigaction(SIGINT, &exithandler, NULL);
	sigaction(SIGTERM, &exithandler, NULL);

	return ((struct err_state) { .has_error = 0, .errmsg = "" });
}
struct err_state _shellfront_initialize(struct _shellfront_env_data *data) {
	struct shellfront_term_conf *config = data->term_conf;
	// get lock file name
	char *exe_name;
	char *prepared_cmd = _shellfront_prepare_hashable(config->cmd, &exe_name, data->is_integrate);
	_shellfront_tmpid = sxprintf("/tmp/shellfront.%lu.lock", djb_hash(prepared_cmd));
	free(prepared_cmd);
	// if it is killing by flag or toggle
	if (config->kill || (config->toggle && access(_shellfront_tmpid, F_OK) != -1)) {
		return _SHELLFRONT_UNLOCK_PROCESS(exe_name);
	}

	// get this process's process ID
	pid_t pid = getpid();
	if (config->once) {
		struct err_state state = _SHELLFRONT_LOCK_PROCESS(pid);
		if (state.has_error) return state;
		// free the lock file name because it is irrelevant for multiple instance application
	} else free(_shellfront_tmpid);

	// PID is guarenteed unique and can be used as APPID
	char *appid = sxprintf("shellfront.proc%li", pid);
	GtkApplication *app = gtk_application_new(appid, G_APPLICATION_DEFAULT_FLAGS);
	free(appid);
	// link terminal setup function
	g_signal_connect(app, "activate", G_CALLBACK(_shellfront_gtk_activate), data);
	struct err_state state = { .errmsg = "" };
	state.has_error = g_application_run(G_APPLICATION(app), 0, NULL);
	if (state.has_error) strcpy(state.errmsg, _("GTK error"));
	g_object_unref(app);
	if (data->is_integrate) free(data->argv);
	return state;
}
