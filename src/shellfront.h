#ifndef SHELLFRONT_H
#define SHELLFRONT_H

#ifdef __cplusplus
extern "C" {
#endif 

// error structure
struct err_state {
	// flag for when error occured
	int has_error;
	// human readable error message
	char errmsg[60];
};
// configuration structure
struct term_conf {
	// window gravity toward edges
	unsigned int grav;
	// position coordinates
	int x, y;
	long width, height;
	char *title;
	// command to be executed in shellfront
	char *cmd;
	// can mouse and keyboard interact with the window
	int interactive;
	// is the window a pop up
	int ispopup;
	// is the application single instance
	int once;

	// kill single instance application if running, start one if not
	int toggle;
	// kill single instance application
	int killopt;
};
// passing arguments and use shellfront just like invoking in terminal
struct err_state shellfront_interpret(int argc, char **argv);
// let user to choose how the application is displayed
struct err_state shellfront_catch_io_from_arg(int argc, char **argv);
// you decide how the application is presented to the user
struct err_state shellfront_catch_io(int argc, char **argv, struct term_conf config);

#ifdef __cplusplus
}
#endif

#endif
