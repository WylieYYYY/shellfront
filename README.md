# ShellFront
ShellFront is a simple frontend for linux shell scripts.
#### Screenshot
Alsamixer started from right-clicking the clock in tint2.  
![Example Screenshot](screenshot.png "shellfront -Tips 30x14 -g 3 -l 0,30 -c 'alsamixer'")
### Features:
- Undecorated popup and decorated application style;
- Customise command to toggle, activate, or deactivate;
- Uses GTK+3 and VTE, portable between linux computers with those packages;

### Setup
`make` or `make install` to intall without additional setting.  
`make clean` to install and remove all object created after building.  
`make remove` to remove the application according to makefile.  
Appending options:
> `install` in the following options is necessary for installing. Orders after make is important for correct behaviour.

`make remove install` to ignore any namespace collision and install it. **Potentially harmful to system**  
`make remove install && shellfront [OPTIONS]` to run in testing environment.
### Using directly in terminal
Different switches are available, help can be called with `shellfront --help` or `shellfront -h`
#### Gravity Setting
When gravity is set, the location variable will be interpreted as the distance from the edge specified.  
GTK+'s window positioning is known to be finicky.  
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
`shellfront_catch_io_from_arg(int argc, char **argv);` allows user to decide how the program will look like by commandline argument.  
`shellfront_catch_io(int argc, char **argv, struct term_conf config);` lets you to decide how it looks.  
> `stderr` will still be directed to the old terminal so that error will not appear to normal users.

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
