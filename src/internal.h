#ifndef INTERNAL_H
#define INTERNAL_H

#include "shellfront.h"

#include <gtk/gtk.h>

// util.c

// parse window size argument
int parse_size_str(char *size, long *x, long *y, char *delim);
// parse window location argument
int parse_loc_str(char *size, int *x, int *y, char *delim);
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
int shellfront_initialize(struct term_conf config);
// accept and parse flags into configuration struct
struct term_conf shellfront_parse(int argc, char **argv);

#endif
