#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>

enum test_states {
	TEST_STATE_NONE,
	// t_gtkfunc_helper
	TEST_STATE_WINDOW_CLOSED,
	TEST_STATE_WINDOW_PRESENTED,
	// t_gtkfunc_helper, t_interface
	TEST_STATE_EXITED,
	// t_gtkfunc
	TEST_STATE_ERROR_STATE_PRINTED,
	TEST_STATE_TERMINAL_SIZE_SET,
	TEST_STATE_TERMINAL_SPAWNED,
	TEST_STATE_WIDGET_SHOWED,
	// t_interface_init
	TEST_STATE_PROCESS_LOCKED,
	TEST_STATE_PROCESS_UNLOCKED,
	TEST_STATE_WILL_HAVE_LOCK_FILE,
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

static inline void add_test_state(enum test_states state) {
	test_state |= 1 << state - 1;
}

static inline bool test_state_contains(enum test_states state) {
	if (state == TEST_STATE_NONE) return test_state == TEST_STATE_NONE;
	return (test_state & (1 << state - 1)) != 0;
}

static inline void clear_test_state() {
	test_state = TEST_STATE_NONE;
}

static inline void assert_test_state(int count, ...) {
	va_list argptr;
	va_start(argptr, count);
	for (int i = 0; i < count; i++) {
		enum test_states state = va_arg(argptr, enum test_states);
		if (state == TEST_STATE_NONE) assert(test_state == TEST_STATE_NONE);
		else assert((test_state & (1 << state - 1)) != 0);
	}
	clear_test_state();
	va_end(argptr);
}

#endif
