#include "shellfront.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <string.h>

void test_interface_parse(void);
void test_interface_init(void);

void test_interface() {
	test_interface_parse();
	test_interface_init();
}