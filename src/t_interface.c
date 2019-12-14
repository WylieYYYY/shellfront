#include "shellfront.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *tmpid;
void test_interface_parse(void);
void test_interface_init(void);
void test_interface_lock(void);

void sig_exit(int signo);

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
	tmpid = malloc(29);
	strcpy(tmpid, "/tmp/shellfront.123.mockproc");
	sig_exit(1);
	assert(fopen("/tmp/shellfront.123.mockproc", "r") == NULL);
	assert(test_state == 1);
}