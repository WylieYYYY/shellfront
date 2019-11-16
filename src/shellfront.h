#ifndef SHELLFRONT_H
#define SHELLFRONT_H

#ifdef __cplusplus
extern "C" {
#endif 

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
int shellfront_interpret(int argc, char **argv);
// let user to choose how the application is displayed
void shellfront_catch_io_from_arg(int argc, char **argv);
// you decide how the application is presented to the user
void shellfront_catch_io(int argc, char **argv, struct term_conf config);

#ifdef __cplusplus
}
#endif

#endif
