#include "config.h"
#include "shellfront.h"

#include <glib/gi18n.h>
#include <stdio.h>

int main(int argc, char **argv) {
	bindtextdomain(GETTEXT_PACKAGE, SHELLFRONT_LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	struct err_state state = shellfront_interpret(argc, argv);
	if (state.has_error) fprintf(stderr, "%s\n", state.errmsg);
	return state.has_error;
}
