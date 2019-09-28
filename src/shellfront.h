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
};
int shellfront_interpret(int argc, char **argv);

#endif