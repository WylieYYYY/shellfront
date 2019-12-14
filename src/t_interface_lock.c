#include "shellfront.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *tmpid;
struct err_state lock_process(int pid);
struct err_state unlock_process(void);
void sig_exit(int signo);

static int test_state;
int mock_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
	assert(signum == SIGINT || signum == SIGTERM);
	assert(act->sa_handler == sig_exit);
	assert(oldact == NULL);
	test_state = 1;
}
FILE *mock_proc_fopen(const char *filename, const char *mode) {
	assert(strcmp(filename, "/proc/123/comm") == 0);
	assert(strcmp(mode, "r") == 0);
	if (test_state == -1) return NULL;
	// create a verifiable process file
	FILE *procfp = fopen("/tmp/shellfront.123.mockproc", "w");
	fprintf(procfp, test_state ? "grep" : "shellfront\n");
	fclose(procfp);
	return fopen("/tmp/shellfront.123.mockproc", "r");
}
int mock_kill(pid_t pid, int sig) {
	assert(pid == 123);
	assert(sig == SIGTERM);
	test_state = 2;
}

void test_interface_lock() {
	// struct err_state lock_process(int pid)
	// no error
	tmpid = malloc(29);
	strcpy(tmpid, "/tmp/shellfront.5863446.lock");
	remove(tmpid);
	struct err_state state = lock_process(123);
	assert(!state.has_error);
	FILE *tmpfp = fopen(tmpid, "r");
	assert(tmpfp != NULL);
	char buf[11];
	buf[10] = '\0';
	fread(buf, 1, 10, tmpfp);
	assert(strcmp(buf, "       123") == 0);
	fclose(tmpfp);
	assert(test_state == 1);
	// existing instance error
	state = lock_process(123);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Existing instance is running, \
remove -1 flag or '/tmp/shellfront.5863446.lock' to unlock") == 0);
	remove(tmpid);
	// struct err_state unlock_process(void)
	// PID mismatch error
	tmpid = malloc(29);
	strcpy(tmpid, "/tmp/shellfront.5863446.lock");
	state = unlock_process();
	assert(state.has_error);
	assert(strcmp(state.errmsg, "PID mismatch in record, use system kill tool") == 0);
	assert(test_state == 1);
	// no error
	tmpid = malloc(29);
	strcpy(tmpid, "/tmp/shellfront.5863446.lock");
	lock_process(123);
	test_state = 0;
	state = unlock_process();
	assert(!state.has_error);
	assert(test_state == 2);
	// no instance error
	tmpid = malloc(23);
	strcpy(tmpid, "/tmp/shellfront.0.lock");
	state = unlock_process();
	assert(state.has_error);
	assert(strcmp(state.errmsg, "No instance of application is running or it is not ran with -1") == 0);
	// no process error
	tmpid = malloc(29);
	strcpy(tmpid, "/tmp/shellfront.5863446.lock");
	lock_process(123);
	test_state = -1;
	state = unlock_process();
	assert(state.has_error);
	assert(strcmp(state.errmsg, "No such process found, use system kill tool") == 0);
}