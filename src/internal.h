#ifndef INTERNAL_H
#define INTERNAL_H

#include "shellfront.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

// util.c

// set error struct faster
struct err_state define_error(char *msg);
// parse window size argument
int _shellfront_parse_size_str(char *size, long *x, long *y);
// parse window location argument
int _shellfront_parse_loc_str(char *loc, int *x, int *y);
// djb2 hash function for lock file name for normalization
unsigned long djb_hash(char *str);
// allocate and perform snprintf automatically
char *sxprintf(char *fmt, ...);
// strip variable part to prepare command for hashing
char *_shellfront_prepare_hashable(char *cmd, char **exe_name, int is_integrate);

// gtkfunc.c

// the start of the GTK+ app
void _shellfront_gtk_activate(GtkApplication *app, struct shellfront_term_conf *config);

// interface.c

// cleanup no matter how it is terminated
void _shellfront_sig_exit(int signo);
// start the GUI with configuration struct
struct err_state _shellfront_initialize(struct shellfront_term_conf *config, int is_integrate);
// accept and parse flags into configuration struct
struct err_state _shellfront_parse(int argc, char **argv, char *builtin_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *parsed_conf);

#endif
