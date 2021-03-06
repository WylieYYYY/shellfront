#include "shellfront.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *_shellfront_tmpid;
struct err_state _shellfront_lock_process(int pid);
struct err_state _shellfront_unlock_process(char *exe_name);
void _shellfront_sig_exit(int signo);

static int test_state;
int mock_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
	assert(signum == SIGINT || signum == SIGTERM);
	assert(act->sa_handler == _shellfront_sig_exit);
	assert(oldact == NULL);
	test_state = 1;
}
FILE *mock_proc_fopen(const char *filename, const char *mode) {
	assert(strcmp(filename, "/proc/123/comm") == 0);
	assert(strcmp(mode, "r") == 0);
	if (test_state == -1) {
		remove("/tmp/shellfront.mock.proc");
		return NULL;
	}
	// create a verifiable process file
	FILE *procfp = fopen("/tmp/shellfront.mock.proc", "w");
	fprintf(procfp, test_state ? "grep" : "shellfront\n");
	fclose(procfp);
	return fopen("/tmp/shellfront.mock.proc", "r");
}
int mock_kill(pid_t pid, int sig) {
	assert(pid == 123);
	assert(sig == SIGTERM);
	test_state = 2;
}

void test_interface_lock() {
	// struct err_state _shellfront_lock_process(int pid)
	// no error
	_shellfront_tmpid = malloc(26);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.mock.lock");
	remove(_shellfront_tmpid);
	struct err_state state = _shellfront_lock_process(123);
	assert(!state.has_error);
	FILE *tmpfp = fopen(_shellfront_tmpid, "r");
	assert(tmpfp != NULL);
	char buf[11];
	buf[10] = '\0';
	fread(buf, 1, 10, tmpfp);
	assert(strcmp(buf, "       123") == 0);
	fclose(tmpfp);
	assert(test_state == 1);
	// existing instance error
	state = _shellfront_lock_process(123);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Existing instance is running, \
remove -1 flag or '/tmp/shellfront.mock.lock' to unlock") == 0);
	remove(_shellfront_tmpid);
	// struct err_state _shellfront_unlock_process(void)
	// PID mismatch error
	_shellfront_tmpid = malloc(26);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.mock.lock");
	char *exe_name = malloc(11);
	strcpy(exe_name, "shellfront");
	state = _shellfront_unlock_process(exe_name);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "PID mismatch in record, use system kill tool") == 0);
	assert(test_state == 1);
	// no error
	_shellfront_tmpid = malloc(26);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.mock.lock");
	_shellfront_lock_process(123);
	test_state = 0;
	exe_name = malloc(11);
	strcpy(exe_name, "shellfront");
	state = _shellfront_unlock_process(exe_name);
	assert(!state.has_error);
	assert(test_state == 2);
	// no instance error
	_shellfront_tmpid = malloc(26);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.fake.lock");
	exe_name = malloc(11);
	strcpy(exe_name, "shellfront");
	state = _shellfront_unlock_process(exe_name);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "No instance of application is running or it is not ran with -1") == 0);
	// no error, integration
	_shellfront_tmpid = malloc(26);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.mock.lock");
	_shellfront_lock_process(123);
	test_state = 1;
	exe_name = malloc(5);
	strcpy(exe_name, "grep");
	state = _shellfront_unlock_process(exe_name);
	assert(!state.has_error);
	// no process error
	_shellfront_tmpid = malloc(26);
	strcpy(_shellfront_tmpid, "/tmp/shellfront.mock.lock");
	_shellfront_lock_process(123);
	test_state = -1;
	exe_name = malloc(11);
	strcpy(exe_name, "shellfront");
	state = _shellfront_unlock_process(exe_name);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "No such process found, use system kill tool") == 0);
}