#ifndef INTERNAL_H
#define INTERNAL_H

#include "shellfront.h"

#include <gtk/gtk.h>

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

// gtkfunc.c

// the start of the GTK+ app
void _shellfront_gtk_activate(GtkApplication *app, struct shellfront_term_conf *config);

// interface.c

// cleanup no matter how it is terminated
void _shellfront_sig_exit(int signo);
// start the GUI with configuration struct
struct err_state _shellfront_initialize(struct shellfront_term_conf *config);
// accept and parse flags into configuration struct
struct err_state _shellfront_parse(int argc, char **argv, struct shellfront_term_conf *parsed_conf);

#endif
