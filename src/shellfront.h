#ifndef __SHELLFRONT__
#define __SHELLFRONT__

struct term_conf {
	unsigned int grav;
	int x, y;
	long width, height;
	char *title;
	char *cmd;
	int interactive;
	int ispopup;
	int once;

	int toggle;
	int killopt;
};
int shellfront_interpret(int argc, char **argv);
void shellfront_catch_io_from_arg(int argc, char **argv);
void shellfront_catch_io(int argc, char **argv, struct term_conf config);

#endif