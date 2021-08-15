#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

enum test_states {
	TEST_STATE_NONE,
	TEST_STATE_META_ASSERTED,
	// t_gtkfunc_helper
	TEST_STATE_WINDOW_CLOSED,
	TEST_STATE_WINDOW_PRESENTED,
	// t_gtkfunc_helper, t_interface
	TEST_STATE_EXITED,
	// t_gtkfunc
	TEST_STATE_CHILD_PROCESS_SETUP,
	TEST_STATE_ERROR_STATE_PRINTED,
	TEST_STATE_TERMINAL_SIZE_SET,
	TEST_STATE_TERMINAL_SPAWNED,
	TEST_STATE_WIDGET_SHOWED,
	TEST_STATE_WILL_BE_PARENT_PROCESS,
	TEST_STATE_WILL_FAIL_FORK,
	// t_interface_init
	TEST_STATE_PROCESS_LOCKED,
	TEST_STATE_PROCESS_UNLOCKED,
	TEST_STATE_WILL_HAVE_LOCK_FILE,
	// t_interface_init, t_util
	TEST_STATE_WILL_RETURN_ERROR,
	// t_interface_lock
	TEST_STATE_PROCESS_KILLED,
	TEST_STATE_SIGNAL_HANDLER_REGISTERED,
	TEST_STATE_WILL_BE_INTEGRATION,
	TEST_STATE_WILL_FAIL_PROC_FOPEN,
	// t_libfunc
	TEST_STATE_WILL_FAIL_INITIALIZE,
	TEST_STATE_WILL_FAIL_PARSE,
	TEST_STATE_WILL_FAIL_START_PROCESS
};

extern enum test_states test_state;
extern GError mock_gerror;
extern struct _shellfront_env_data mock_env_data;

static inline bool _check_flag_from_enum(enum test_states state) {
	return (test_state & (1 << state - 1)) != 0;
}

static inline void clear_test_state() {
	test_state = TEST_STATE_NONE;
}

static inline void add_test_state(enum test_states state) {
	if (_check_flag_from_enum(TEST_STATE_META_ASSERTED)) clear_test_state();
	test_state |= 1 << state - 1;
}

static inline bool test_state_contains(enum test_states state) {
	if (state == TEST_STATE_NONE) {
		return test_state == TEST_STATE_NONE || test_state == TEST_STATE_META_ASSERTED;
	}
	return _check_flag_from_enum(state);
}

static inline void _assert_test_state(char *filename, int line, enum test_states state, bool inverted) {
	printf("Assertion in %s at line %i. Current state: %i.\n", filename, line, test_state);
	if (state == TEST_STATE_NONE) {
		if (_check_flag_from_enum(TEST_STATE_META_ASSERTED)) clear_test_state();
		assert((test_state == TEST_STATE_NONE || test_state == TEST_STATE_META_ASSERTED) != inverted);
	}
	else assert(_check_flag_from_enum(state) != inverted);
	add_test_state(TEST_STATE_META_ASSERTED);
}

#define assert_test_state(x) _assert_test_state(__FILE__,__LINE__,x, false)
#define assert_test_state_not(x) _assert_test_state(__FILE__,__LINE__,x,true)

#endif
