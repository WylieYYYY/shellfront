#ifndef INTERNAL_H
#define INTERNAL_H

#include "shellfront.h"

#include <gtk/gtk.h>

// util.c

// set error struct faster
struct err_state define_error(char *msg);
// parse window size argument
int parse_size_str(char *size, long *x, long *y);
// parse window location argument
int parse_loc_str(char *loc, int *x, int *y);
// djb2 hash function for lock file name for normalization
unsigned long hash(char *str);
// allocate and perform snprintf automatically
char *sxprintf(char *fmt, ...);

// gtkfunc.c

// the start of the GTK+ app
void gtk_activate(GtkApplication *app, struct term_conf *config);

// interface.c

// cleanup no matter how it is terminated
void sig_exit(int signo);
// start the GUI with configuration struct
struct err_state shellfront_initialize(struct term_conf *config);
// accept and parse flags into configuration struct
struct err_state shellfront_parse(int argc, char **argv, struct term_conf *parsed_conf);

#endif
