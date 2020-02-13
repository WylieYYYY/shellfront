#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>

struct err_state _shellfront_validate_opt(char *locstr, char *sizestr, struct shellfront_term_conf *config, GError *gtkerr);
struct err_state _shellfront_parse(int argc, char **argv, struct shellfront_term_conf *config);

void test_parse() {
	// struct err_state _shellfront_validate_opt(char *locstr, char *sizestr, struct shellfront_term_conf *config)
	// parse loc error
	struct shellfront_term_conf config = shellfront_term_conf_default;
	struct err_state state = _shellfront_validate_opt("1,,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect location format, should be X,Y") == 0);
	// parse size error
	state = _shellfront_validate_opt("1,2", "300xx200", &config, NULL);
	assert(strcmp(state.errmsg, "Incorrect size format, should be XxY") == 0);
	// no error
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(!state.has_error);
	assert(config.x == 1 && config.y == 2);
	assert(config.width == 300 && config.height == 200);
	// grav < 1 error
	config.grav = 0;
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect gravity range, see README for usage") == 0);
	// grav > 9 error
	config.grav = 10;
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect gravity range, see README for usage") == 0);
	// test once implied by ispopup
	config.grav = 1;
	config.ispopup = 1;
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(!state.has_error);
	assert(config.once);
	// ispopup title conflict error
	config.title = "abc";
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Conflicting arguments, see README for usage") == 0);
	// test once implied by toggle
	config.once = 0;
	config.ispopup = 0;
	config.toggle = 1;
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(!state.has_error);
	assert(config.once);
	// toggle killopt conflict error
	config.killopt = 1;
	state = _shellfront_validate_opt("1,2", "300x200", &config, NULL);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Conflicting arguments, see README for usage") == 0);
	// GTK error
	config.killopt = 0;
	GError gtkerr = { .code = 1, .message = "GTK error" };
	state = _shellfront_validate_opt("1,2", "300x200", &config, &gtkerr);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "GTK error") == 0);
	// struct err_state _shellfront_parse(int argc, char **argv, struct shellfront_term_conf *config)
	// no error (all flags except ispopup and killopt)
	config.toggle = 0;
	char **argv = (char *[]) { "shellfront", "-1iTg", "3", "-l", "1,2", "-s", "10x20", "-c", "command", "-t", "Title" };
	state = _shellfront_parse(11, argv, &config);
	assert(!state.has_error);
	assert(config.once && config.interactive && config.toggle && !config.killopt && !config.ispopup);
	assert(config.grav == 3);
	assert(config.x == 1 && config.y == 2);
	assert(config.width == 10 && config.height == 20);
	assert(strcmp(config.cmd, "command") == 0);
	assert(strcmp(config.title, "Title") == 0);
}