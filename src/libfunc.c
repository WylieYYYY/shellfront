#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNIT_TEST
	struct err_state mock_initialize(struct shellfront_term_conf *config, bool is_integrate);
	#define _shellfront_initialize(x,y) mock_initialize(x,y)
	struct err_state mock_parse(int argc, char **argv, char *builtin_opt,
		GOptionEntry *custom_opt, struct shellfront_term_conf *config);
	#define _shellfront_parse(a,b,c,d,e) mock_parse(a,b,c,d,e)
	struct err_state mock_start_process(char *prog_name, struct shellfront_term_conf *config, char *current_tty);
	#define _SHELLFRONT_START_PROCESS(x,y,z) mock_start_process(x,y,z)
#else
	#define _SHELLFRONT_START_PROCESS(x,y) _shellfront_start_process(x,y)
#endif

const struct shellfront_term_conf shellfront_term_conf_default = {
	.grav = 1,
	.x = 0,
	.y = 0,
	.width = 80,
	.height = 24,
	.title = "",
	.icon = "",
	.cmd = "",
	.interactive = 0,
	.popup = 0,
	.once = 0,
	.toggle = 0,
	.kill = 0,
	.desc = ""
};

struct err_state _shellfront_start_process(struct _shellfront_env_data *data, char *current_tty) {
	// program path for hashable identifier
	data->term_conf->cmd = data->argv[0];
	struct err_state state = _shellfront_initialize(data);
	// if executed with no error, indicate it is the original process
	if (!state.has_error) strcpy(state.errmsg, _("Original process, please end"));
	return state;
}

struct err_state shellfront_interpret(int argc, char **argv) {
	struct shellfront_term_conf config = shellfront_term_conf_default;
	config.desc = _("- simple frontend for shell scripts");
	struct err_state state = _shellfront_parse(argc, argv, "glstIcip1Tk", NULL, &config);
	// parse error or continue execution
	if (state.has_error) return state;
	struct _shellfront_env_data data = { .term_conf = &config, .is_integrate = false };
	return _shellfront_initialize(&data);
}

struct err_state shellfront_catch(int argc, char **argv, char *accepted_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf default_config) {
	// this function is intended to catch itself as command
	if (strchr(accepted_opt, 'c') != NULL) {
		return define_error(_("ShellFront integration does not allow cmd option"));
	}
	// non-NULL if forked, return no error to indicate forked, or return error
	if (_shellfront_fork_state != NULL) return *_shellfront_fork_state; //FIXME: Apply custom options
	struct err_state state = _shellfront_parse(argc, argv, accepted_opt, custom_opt, &default_config);
	// parse error
	if (state.has_error) return state;
	struct _shellfront_env_data data = {
		.term_conf = &default_config,
		.is_integrate = true,
		.argc = argc,
		.argv = argv
	};
	// return the state of the process
	return _SHELLFRONT_START_PROCESS(&data, ttyname(STDIN_FILENO));
}
