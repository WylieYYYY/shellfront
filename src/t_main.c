#include "internal.h"
#include "test.h"

void test_util(void);
void test_parse(void);
void test_interface(void);
void test_gtkfunc(void);
void test_libfunc(void);

long int test_state;
GError mock_gerror = { .code = 2, .message = "Error message" };
struct _shellfront_env_data mock_env_data;

int main(int argc, char **argv) {
	test_util();
	test_parse();
	test_interface();
	test_gtkfunc();
	test_libfunc();
	return 0;
}
