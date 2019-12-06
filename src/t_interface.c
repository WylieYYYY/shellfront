#include "shellfront.h"

#include <assert.h>
#include <string.h>

struct err_state validate_opt(char *locstr, char *sizestr, struct term_conf *config);

void test_interface() {
	// struct err_state validate_opt(char *locstr, char *sizestr, struct term_conf *config)
	// parse loc error
	struct term_conf config = term_conf_default;
	struct err_state state = validate_opt("1,,2", "300x200", &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect location format, should be X,Y") == 0);
	// parse size error
	state = validate_opt("1,2", "300xx200", &config);
	assert(strcmp(state.errmsg, "Incorrect size format, should be XxY") == 0);
	// no error
	state = validate_opt("1,2", "300x200", &config);
	assert(!state.has_error);
	assert(config.x == 1);
	assert(config.y == 2);
	assert(config.width == 300);
	assert(config.height == 200);
	// grav < 1 error
	config.grav = 0;
	state = validate_opt("1,2", "300x200", &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect gravity range, see README for usage") == 0);
	// grav > 9 error
	config.grav = 10;
	state = validate_opt("1,2", "300x200", &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Incorrect gravity range, see README for usage") == 0);
	// test once implied by ispopup
	config.grav = 1;
	config.ispopup = 1;
	state = validate_opt("1,2", "300x200", &config);
	assert(!state.has_error);
	assert(config.once);
	// ispopup title conflict error
	config.title = "abc";
	state = validate_opt("1,2", "300x200", &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Conflicting arguments, see README for usage") == 0);
	// test once implied by toggle
	config.once = 0;
	config.ispopup = 0;
	config.toggle = 1;
	state = validate_opt("1,2", "300x200", &config);
	assert(!state.has_error);
	assert(config.once);
	// toggle killopt conflict error
	config.killopt = 1;
	state = validate_opt("1,2", "300x200", &config);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Conflicting arguments, see README for usage") == 0);
}