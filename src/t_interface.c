#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>

void test_interface_parse(void);
void test_interface_init(void);

void sig_exit(int signo);
struct err_state lock_process(int pid);

extern char *tmpid;
int mock_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
	assert(signum == SIGINT || signum == SIGTERM);
	assert(act->sa_handler == sig_exit);
	assert(oldact == NULL);
}

void test_interface() {
	test_interface_parse();
	test_interface_init();
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
	// existing instance error
	state = lock_process(123);
	assert(state.has_error);
	assert(strcmp(state.errmsg, "Existing instance is running, \
remove -1 flag or '/tmp/shellfront.5863446.lock' to unlock") == 0);
	remove(tmpid);
	// struct err_state unlock_process(void)
}