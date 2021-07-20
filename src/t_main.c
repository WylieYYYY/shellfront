#include "test.h"

void test_util(void);
void test_parse(void);
void test_interface(void);
void test_gtkfunc(void);
void test_libfunc(void);

enum test_states test_state;

int main(int argc, char **argv) {
	test_util();
	test_parse();
	test_interface();
	test_gtkfunc();
	test_libfunc();
	return 0;
}
