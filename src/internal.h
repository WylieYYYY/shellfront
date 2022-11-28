#ifndef _SHELLFRONT_INTERNAL_H
#define _SHELLFRONT_INTERNAL_H

#include "shellfront.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

// util.c

// set error struct faster
struct err_state define_error(char *msg);
// parse coordinate arguments
int _shellfront_parse_coordinate(char *size, long *left,
	long *right, long min, char *delim);
// djb2 hash function for lock file name for normalization
unsigned long djb_hash(char *str);
// allocate and perform snprintf automatically
char *sxprintf(char *fmt, ...);
// strip variable part to prepare command for hashing
char *_shellfront_prepare_hashable(char *cmd, char **exe_name, int is_integrate);
// convert a GError to an err_state and free gerror
struct err_state _shellfront_gerror_to_err_state(GError *gerror);

// gtkfunc.c

// the error state after forking, NULL if unforked, public to be seen by main()
extern struct err_state _shellfront_fork_state;
// data that are used by GTK+ functions
struct _shellfront_env_data {
	// configuration of ShellFront
	struct shellfront_term_conf *term_conf;
	// whether ShellFront is called with shellfront_catch()
	int is_integrate;
	// argument count of original main()
	int argc;
	// argument vector of original main()
	char **argv;
	// forked child's PID
	pid_t child_pid;
};
// the start of the GTK+ app
void _shellfront_gtk_activate(GtkApplication *app, struct _shellfront_env_data *data);

// interface.c

// cleanup no matter how it is terminated
void _shellfront_sig_exit(int signo);
// start the GUI with configuration struct
struct err_state _shellfront_initialize(struct _shellfront_env_data *data);
// accept and parse flags into configuration struct
struct err_state _shellfront_parse(int argc, char **argv, char *builtin_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *parsed_conf);

// main.c
int main(int argc, char **argv);

#endif
