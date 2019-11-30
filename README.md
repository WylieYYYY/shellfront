# ShellFront
[![pipeline status](https://gitlab.com/WylieYYYY/shellfront/badges/master/pipeline.svg)](https://gitlab.com/WylieYYYY/shellfront/commits/master)
[![coverage report](https://gitlab.com/WylieYYYY/shellfront/badges/master/coverage.svg)](https://gitlab.com/WylieYYYY/shellfront/commits/master)  
ShellFront is a simple frontend for linux shell scripts (Shellfront is pronounce as "shelf-front" as in the icon).
#### Screenshot
Alsamixer started from right-clicking the clock in tint2.  
![Example Screenshot](screenshot.png "shellfront -Tips 30x14 -g 3 -l 0,0 -c 'alsamixer'")
### Features:
- Undecorated popup and decorated window style;
- Customise command to toggle, activate, or deactivate;
- Uses GTK3 and VTE, portable between linux computers;
- Integrate to other applications as C library;

### Setup
1. Download the compressed package from [here](https://gitlab.com/WylieYYYY/shellfront/-/jobs/artifacts/master/download?job=build-pkg) 
   and extract to a directory with a name without whitespaces. Then, install the following packages.
2. Install the following packages:  
   For ArchLinux: `gtk3 vte3`  
   For Ubuntu: `libgtk-3-dev libvte-2.91-dev`  
   Other distros should also install GTK and VTE development package from repositories or build from source.
3. ShellFront can then be installed by using `./configure && make` and `sudo make install`

The package can be uninstalled by using `sudo make uninstall` in the same directory.
### Using directly in terminal
Different switches are available, help can be called with `shellfront --help` or `shellfront -h`
#### Gravity Setting
When gravity is set, the location variable will be interpreted as the distance from the edge specified.  
GTK's window positioning is known to be finicky.  
Default setting is `1 (Top-left)`.  
Corresponding coordinates as below:
```
+-----x
|1 2 3
|4 5 6
|7 8 9
y
```
If the gravity is centered in any axis, the corresponding x or y value of the loc variable will be ignored.  
A placeholder of any non-negative number should be used in the loc variable (Number appended by letters are also accepted).
#### Tips
To hide the console cursor, perform `printf` or `echo -n` spaces until out of bound, or by using `echo -n "$(command)[SPACES]"` on last output command.  
For the latter method, if colourised output from pipe is required, `unbuffer` from package `expect` can be appended in front of the command.
### Using as library within C program
ShellFront can customize how a terminal program appear as. Size, title, format and etc. can be fixed for maximum user experience.  
> `stderr` will still be directed to the old terminal so that error will not appear to normal users.

#### C library reference
`shellfront_catch_io_from_arg(int argc, char **argv);` allows user to decide how the program will look like by commandline argument.  
`shellfront_catch_io(int argc, char **argv, struct term_conf config);` lets you to decide how it looks.  

Both functions above return an `err_state`, the setup on the program should handle the result as follow:
1. If `has_error` of the state evaluate as true, something went wrong, 
   do standard error handling with `errmsg`, end the program without executing the main logic.
2. If `has_error` of the state evaluate as false:
   - If `errmsg` is `""`, this is currently running in ShellFront, continue the execution of the program.
   - If `errmsg` is not `""`, this is currently in the original process, end the program without executing the main logic.

Sample program in C:
```
#include "shellfront.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
	struct term_conf config = {
		.width = 20
	};
	struct err_state state = shellfront_catch_io(argc, argv, config);
	if (state.has_error) fprintf(stderr, state.errmsg);
	else if (strcmp(state.errmsg, "") != 0) return 0;
	printf("Hi\n");
	fprintf(stderr, "Errors");
	sleep(1000);
	return 0;
}
```

The `err_state` struct has default values for all variable, listed below:
```
- has_error               : FALSE or 0
- errmsg for error message: ""
```
The `term_conf` struct has default values for all variable, listed below:
```
Window and terminal properties
- grav for gravity        : 1 (Top-left)
- x and y for coordinate  : 0
- width                   : 80
- height                  : 24
- title                   : ""
- cmd for target command  : "echo -n  Hello World!; sleep infinity" (In terminal) or "" (In C library)
- interactive for input   : FALSE or 0
- ispopup                 : FALSE or 0
- once for single instance: FALSE or 0

Invoke behaviours switch
- toggle when invoked     : FALSE or 0
- killopt for killing     : FALSE or 0 
```
