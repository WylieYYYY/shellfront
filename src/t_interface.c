#include "shellfront.h"
#include "test.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *_shellfront_tmpid;
void test_interface_init(void);
void test_interface_lock(void);

void _shellfront_sig_exit(int signo);

void mock_exit(int status) {
	assert(status == 0);
	add_test_state(TEST_STATE_EXITED);
}

void test_interface() {
	test_interface_init();
	test_interface_lock();
	// void sig_exit(int signo)
	_shellfront_tmpid = malloc(29);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.123.mockproc");
	_shellfront_sig_exit(1);
	FILE *check_lock = fopen("/tmp/shellfront.123.mockproc", "r");
	assert(check_lock == NULL);
	assert_test_state(TEST_STATE_EXITED);
}
