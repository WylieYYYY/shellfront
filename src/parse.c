#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct err_state _shellfront_validate_opt(char *locstr, char *sizestr, struct shellfront_term_conf *config, GError *gtkerr) {
	if (!_shellfront_parse_loc_str(locstr, &(config->x), &(config->y))) {
		return define_error("Incorrect location format, should be X,Y");
	}
	if (!_shellfront_parse_size_str(sizestr, &(config->width), &(config->height))) {
		return define_error("Incorrect size format, should be XxY");
	}
	if (config->grav < 1 || config->grav > 9) {
		return define_error("Incorrect gravity range, see README for usage");
	}
	if ((config->toggle && config->killopt) || (strcmp(config->title, "") && config->ispopup)) {
		return define_error("Conflicting arguments, see README for usage");
	}
	// implied flag
	config->once |= (config->ispopup || config->toggle);
	// return propagated GTK error if there is any
	if (gtkerr == NULL) return ((struct err_state) { .has_error = 0, .errmsg = "" });
	struct err_state state = { .has_error = gtkerr->code };
	strcpy(state.errmsg, gtkerr->message);
	return state;
}

struct err_state _shellfront_parse(int argc, char **argv, struct shellfront_term_conf *config) {
	// default configurations
	*config = shellfront_term_conf_default;
	config->cmd = "echo 'Hello World!'; read";
	char *locstr = "0,0";
	char *sizestr = "80x24";
	
	// options and help messages
	GOptionEntry options[] = {
		{
			.long_name = "grav",
			.short_name = 'g',
			.arg = G_OPTION_ARG_INT,
			.arg_data = &(config->grav),
			.arg_description = "ENUM",
			.description = "Set gravity for window, see README for detail"
		}, {
			.long_name = "loc",
			.short_name = 'l',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &locstr,
			.arg_description = "X,Y",
			.description = "Set the default screen location"
		}, {
			.long_name = "size",
			.short_name = 's',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &sizestr,
			.arg_description = "XxY",
			.description = "Set the size"
		}, {
			.long_name = "title",
			.short_name = 't',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->title),
			.arg_description = "TITLE",
			.description = "Set the title for application window"
		}, {
			.long_name = "cmd",
			.short_name = 'c',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->cmd),
			.arg_description = "COMMAND",
			.description = "Set the command to be executed"
		}, {
			.long_name = "interactive",
			.short_name = 'i',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->interactive),
			.description = "Application can be interacted with mouse and auto focuses"
		}, {
			.long_name = "ispopup",
			.short_name = 'p',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->ispopup),
			.description = "Display as popup instead of application, implies -1"
		}, {
			.long_name = "once",
			.short_name = '1',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->once),
			.description = "Set if only one instance is allowed"
		}, {
			.long_name = "toggle",
			.short_name = 'T',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->toggle),
			.description = "Toggle single instance application, implies -1"
		}, {
			.long_name = "killopt",
			.short_name = 'k',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->killopt),
			.description = "Kill a single instance application according to command"
		}, { 0 }
	};

	GError *gtkerr = NULL;
	// description and register arguments
	gtk_init_with_args(&argc, &argv, "- simple frontend for shell scripts", options, NULL, &gtkerr);
	struct err_state state = _shellfront_validate_opt(locstr, sizestr, config, gtkerr);
	g_clear_error(&gtkerr);

	return state;
}