#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNIT_TEST
	struct err_state mock_initialize(struct _shellfront_env_data *data);
	#define _shellfront_initialize(x) mock_initialize(x)
	struct err_state mock_parse(int argc, char **argv, char *builtin_opt,
		GOptionEntry *custom_opt, struct shellfront_term_conf *config);
	#define _shellfront_parse(a,b,c,d,e) mock_parse(a,b,c,d,e)
	bool mock_g_option_context_parse(GOptionContext *context, int *argc, char ***argv, GError **error);
	#define g_option_context_parse(a,b,c,d) mock_g_option_context_parse(a,b,c,d)
	struct err_state mock_start_process(int argc, char **argv, char *accepted_opt,
		GOptionEntry *custom_opt, struct shellfront_term_conf *default_config);
	#define _SHELLFRONT_START_PROCESS(a,b,c,d,e) mock_start_process(a,b,c,d,e)
#else
	#define _SHELLFRONT_START_PROCESS(a,b,c,d,e) _shellfront_start_process(a,b,c,d,e)
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

struct err_state _shellfront_start_process(int argc, char **argv, char *accepted_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *default_config) {
	int unparsed_argc = argc;
	// does not need to be a deep copy, only needs to maintain references to pointers
	char **unparsed_argv = malloc(sizeof (char *) * argc);
	memcpy(unparsed_argv, argv, sizeof (char *) * argc);
	struct err_state state = _shellfront_parse(argc, argv, accepted_opt, custom_opt, default_config);
	// parse error
	if (state.has_error) return state;
	// program path for hashable identifier
	default_config->cmd = unparsed_argv[0];
	struct _shellfront_env_data data = {
		.term_conf = default_config,
		.is_integrate = true,
		.argc = unparsed_argc,
		.argv = unparsed_argv
	};
	state = _shellfront_initialize(&data);
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
	// non-NULL if forked, return no error to indicate forked, or return error
	if (_shellfront_fork_state._forked) {
		const GOptionEntry empty_custom_opt[] = {{ 0 }};
		GOptionContext *option_context = g_option_context_new(NULL);
		g_option_context_add_main_entries(option_context, custom_opt == NULL? empty_custom_opt : custom_opt, NULL);
		GError *gliberr = NULL;
		g_option_context_parse(option_context, &argc, &argv, &gliberr);
		// ignore code zero error as parsing ShellFront options here will yield unknown error and
		// if there are really problems, it will be detected in the first parsing pass
		if (gliberr != NULL && gliberr->code != 0) _shellfront_fork_state = _shellfront_gerror_to_err_state(gliberr);
		g_option_context_free(option_context);
		return _shellfront_fork_state;
	}
	// this function is intended to catch itself as command
	if (strchr(accepted_opt, 'c') != NULL) {
		return define_error(_("ShellFront integration does not allow cmd option"));
	}
	// return the state of the process
	return _SHELLFRONT_START_PROCESS(argc, argv, accepted_opt, custom_opt, &default_config);
}
