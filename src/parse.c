#include "internal.h"
#include "shellfront.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct err_state _shellfront_validate_opt(char *locstr, char *sizestr, struct shellfront_term_conf *config, GError *gtkerr) {
	long long_x, long_y;
	if (!_shellfront_parse_coordinate(locstr, &long_x, &long_y, 0, ",")) {
		return define_error(_("Incorrect location format, should be X,Y"));
	}
	config->x = long_x;
	config->y = long_y;
	if (!_shellfront_parse_coordinate(sizestr, &(config->width), &(config->height), 1, "x")) {
		return define_error(_("Incorrect size format, should be XxY"));
	}
	if (config->grav < 1 || config->grav > 9) {
		return define_error(_("Incorrect gravity range, see README for usage"));
	}
	if ((config->toggle && config->kill) || (strcmp(config->title, "") && config->popup)) {
		return define_error(_("Conflicting arguments, see README for usage"));
	}
	// implied flag
	config->once |= (config->popup || config->toggle);
	// return propagated GTK error if there is any
	return _shellfront_gerror_to_err_state(gtkerr);
}

GOptionEntry *_shellfront_construct_opt(const char *builtin, GOptionEntry *custom,
	struct shellfront_term_conf *config, char **locstr, char **sizestr) {
	GOptionEntry builtin_entries[] = {
		{
			.long_name = "grav",
			.short_name = 'g',
			.arg = G_OPTION_ARG_INT,
			.arg_data = &(config->grav),
			.arg_description = _("ENUM"),
			.description = _("Set gravity for window, 1 = Top-left, 9 = Bottom-right")
		}, {
			.long_name = "loc",
			.short_name = 'l',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = locstr,
			.arg_description = "X,Y",
			.description = _("Set the default screen location")
		}, {
			.long_name = "size",
			.short_name = 's',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = sizestr,
			.arg_description = "XxY",
			.description = _("Set the size")
		}, {
			.long_name = "title",
			.short_name = 't',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->title),
			.arg_description = _("TITLE"),
			.description = _("Set the title for application window")
		}, {
			.long_name = "icon",
			.short_name = 'I',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->icon),
			.arg_description = _("PATH"),
			.description = _("Set the icon for application window")
		}, {
			.long_name = "cmd",
			.short_name = 'c',
			.arg = G_OPTION_ARG_STRING,
			.arg_data = &(config->cmd),
			.arg_description = _("COMMAND"),
			.description = _("Set the command to be executed")
		}, {
			.long_name = "interactive",
			.short_name = 'i',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->interactive),
			.description = _("Application can be interacted with mouse and auto focuses")
		}, {
			.long_name = "popup",
			.short_name = 'p',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->popup),
			.description = _("Display as popup instead of a window, implies -1")
		}, {
			.long_name = "once",
			.short_name = '1',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->once),
			.description = _("Set if only one instance is allowed")
		}, {
			.long_name = "toggle",
			.short_name = 'T',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->toggle),
			.description = _("Toggle single instance application, implies -1")
		}, {
			.long_name = "kill",
			.short_name = 'k',
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(config->kill),
			.description = _("Kill a single instance application according to command")
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

	// construct custom options here to fail fast
	GOptionEntry *options =
		_shellfront_construct_opt(builtin_opt, custom_opt, config, &locstr, &sizestr);

	GError *gtkerr = NULL;
	// description and register arguments
	gtk_init_with_args(&argc, &argv, config->desc, options, NULL, &gtkerr);
	free(options);
	struct err_state state = _shellfront_validate_opt(locstr, sizestr, config, gtkerr);

	return state;
}
