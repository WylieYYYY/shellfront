#ifndef SHELLFRONT_H
#define SHELLFRONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

// error structure
struct err_state {
	// flag for when error occured
	int has_error;
	// human readable error message
	char errmsg[100];
};
// configuration structure
struct shellfront_term_conf {
	// window gravity toward edges
	unsigned int grav;
	// position coordinates
	int x, y;
	long width, height;
	char *title;
	// filepath for the icon
	char *icon;
	// command to be executed in shellfront
	char *cmd;
	// can mouse and keyboard interact with the window
	int interactive;
	// is it a popup
	int popup;
	// is the application single instance
	int once;

	// kill single instance application if running, start one if not
	int toggle;
	// kill single instance application
	int kill;
	// application description in help menu
	char *desc;
};
// constant struct with default member values
extern const struct shellfront_term_conf shellfront_term_conf_default;
// passing arguments and use shellfront just like invoking in terminal
struct err_state shellfront_interpret(int argc, char **argv);
// decides how the terminal will look like by user and/or program
struct err_state shellfront_catch(int argc, char **argv, char *accepted_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf default_config);

#ifdef __cplusplus
}
#endif

#endif
