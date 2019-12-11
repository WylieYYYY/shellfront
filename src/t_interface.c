#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>

void test_interface_parse(void);
struct err_state shellfront_initialize(struct term_conf *config);

int mock_g_application_run(GApplication *application, int argc, char **argv) {
	return 0;
}

void test_interface() {
	test_interface_parse();
	// struct err_state shellfront_initialize(struct term_conf *config)
	struct term_conf config = term_conf_default;
	struct err_state state = shellfront_initialize(&config);
	assert(!state.has_error);
}