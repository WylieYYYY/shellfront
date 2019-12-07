#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>

struct err_state validate_opt(char *locstr, char *sizestr, struct term_conf *config, GError *gtkerr);
struct err_state shellfront_parse(int argc, char **argv, struct term_conf *config);

void test_interface() {
	// struct err_state validate_opt(char *locstr, char *sizestr, struct term_conf *config)
	// parse loc error
	struct term_conf config = term_conf_default;
	struct err_state state = validate_opt("1,,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect location format, should be X,Y") == 0);
	// parse size error
	state = validate_opt("1,2", "300xx200", &config, NULL);
	assert(strcmp(state.errmsg, "Incorrect size format, should be XxY") == 0);
	// no error
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(!state.has_error);
	assert(config.x == 1 && config.y == 2);
	assert(config.width == 300 && config.height == 200);
	// grav < 1 error
	config.grav = 0;
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect gravity range, see README for usage") == 0);
	// grav > 9 error
	config.grav = 10;
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect gravity range, see README for usage") == 0);
	// test once implied by ispopup
	config.grav = 1;
	config.ispopup = 1;
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(!state.has_error);
	assert(config.once);
	// ispopup title conflict error
	config.title = "abc";
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Conflicting arguments, see README for usage") == 0);
	// test once implied by toggle
	config.once = 0;
	config.ispopup = 0;
	config.toggle = 1;
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(!state.has_error);
	assert(config.once);
	// toggle killopt conflict error
	config.killopt = 1;
	state = validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Conflicting arguments, see README for usage") == 0);
	// GTK error
	config.killopt = 0;
	GError gtkerr = { .code = 1, .message = "GTK error" };
	state = validate_opt("1,2", "300x200", &config, &gtkerr);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "GTK error") == 0);
	// struct err_state shellfront_parse(int argc, char **argv, struct term_conf *config)
	// no error (all flags except ispopup and killopt)
	config.toggle = 0;
	char **argv = (char *[]) { "shellfront", "-1iTg", "3", "-l", "1,2", "-s", "10x20", "-c", "command", "-t", "Title" };
	state = shellfront_parse(11, argv, &config);
	assert(!state.has_error);
	assert(config.once && config.interactive && config.toggle && !config.killopt && !config.ispopup);
	assert(config.grav == 3);
	assert(config.x == 1 && config.y == 2);
	assert(config.width == 10 && config.height == 20);
	assert(strcmp(config.cmd, "command") == 0);
	assert(strcmp(config.title, "Title") == 0);
}