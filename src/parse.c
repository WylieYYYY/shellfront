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
	if ((config->toggle && config->kill) || (strcmp(config->title, "") && config->popup)) {
		return define_error("Conflicting arguments, see README for usage");
	}
	// implied flag
	config->once |= (config->popup || config->toggle);
	// return propagated GTK error if there is any
	if (gtkerr == NULL) return ((struct err_state) { .has_error = 0, .errmsg = "" });
	struct err_state state = { .has_error = gtkerr->code };
	strcpy(state.errmsg, gtkerr->message);
	return state;
}

GOptionEntry *_shellfront_construct_opt(const char *builtin, GOptionEntry *custom,
	struct shellfront_term_conf *config, char **locstr, char **sizestr) {
	GOptionEntry builtin_entries[] = {
		{
			.long_name = "grav",
			.short_name = 'g',
			.arg = G_OPTION_ARG_INT,
			.arg_data = &(config->grav),
			.arg_description = "ENUM",
			.description = "Set gravity for window, 1 = Top-left, 9 = Bottom-right"
		}, {
			.long_name = "loc",
			.short_name = 'l',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = locstr,
			.arg_description = "X,Y",
			.description = "Set the default screen location"
		}, {
			.long_name = "size",
			.short_name = 's',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = sizestr,
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
			.long_name = "icon",
			.short_name = 'I',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->icon),
			.arg_description = "PATH",
			.description = "Set the icon for application window"
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
			.long_name = "popup",
			.short_name = 'p',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->popup),
			.description = "Display as popup instead of a window, implies -1"
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
			.long_name = "kill",
			.short_name = 'k',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->kill),
			.description = "Kill a single instance application according to command"
		}
	};

	const int option_size = sizeof (GOptionEntry);
	int custom_count = 0;
	const GOptionEntry end_block = { 0 };
	// count custom options and copy them and end block to the bottom half of memory
	while (custom != NULL && memcmp(&custom[custom_count], &end_block, option_size) != 0) {
		custom_count++;
	}
	GOptionEntry *options = malloc((strlen(builtin) + custom_count + 1) * option_size);
	if (custom == NULL) {
		options[strlen(builtin)] = (GOptionEntry){ 0 };
	} else {
		memcpy(&options[strlen(builtin)], custom, (custom_count + 1) * option_size);
	}

	const int builtin_count = sizeof builtin_entries / option_size;
	// current allocated index to place builtin option
	int alloc_index = 0;
	for (int builtin_index = 0; builtin_index < builtin_count; builtin_index++) {
		// no target short name found, pointer moved to NULL
		if (strchr(builtin, builtin_entries[builtin_index].short_name) == NULL) continue;
		options[alloc_index++] = builtin_entries[builtin_index];
	}
	return options;
}

struct err_state _shellfront_parse(int argc, char **argv, char *builtin_opt,
	GOptionEntry *custom_opt, struct shellfront_term_conf *config) {
	// default configurations
	config->cmd = "echo 'Hello World!'; read";
	char *locstr = "0,0";
	char *sizestr = "80x24";

	GOptionEntry *options =
		_shellfront_construct_opt(builtin_opt, custom_opt, config, &locstr, &sizestr);

	GError *gtkerr = NULL;
	// description and register arguments
	gtk_init_with_args(&argc, &argv, config->desc, options, NULL, &gtkerr);
	free(options);
	struct err_state state = _shellfront_validate_opt(locstr, sizestr, config, gtkerr);
	g_clear_error(&gtkerr);

	return state;
}