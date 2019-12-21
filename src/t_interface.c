#include "shellfront.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *_shellfront_tmpid;
void test_interface_parse(void);
void test_interface_init(void);
void test_interface_lock(void);

void _shellfront_sig_exit(int signo);

static int test_state;
void mock_exit(int status) {
	assert(status == 0);
	test_state = 1;
}

void test_interface() {
	test_interface_parse();
	test_interface_init();
	test_interface_lock();
	// void sig_exit(int signo)
	_shellfront_tmpid = malloc(29);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.123.mockproc");
	_shellfront_sig_exit(1);
	FILE *check_lock = fopen("/tmp/shellfront.123.mockproc", "r");
	if (check_lock != NULL) {
		fclose(check_lock);
		remove("/tmp/shellfront.123.mockproc");
	}
	assert(check_lock == NULL);
	assert(test_state == 1);
}