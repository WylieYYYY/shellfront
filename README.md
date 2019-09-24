# ShellFront
ShellFront is a simple frontend for shell scripts.
### Features:
- Undecorated popup and decorated application style;
- Customise command to toggle, activate, or deactivate;
- Uses GTK+3 and VTE, portable between linux computers with those packages;

### Setup
`make` or `make install` to intall without additional setting.  
`make remove` to remove the application according to makefile.  
Appending options:
> `install` in the following options is necessary for installing. Orders after make is important for correct behaviour.

`make remove install` to ignore any namespace collision and install it. **Potentially harmful to system**  
`make remove install && shellfront [OPTIONS]` to run in testing environment.
### Gravity Setting Manual
When gravity is set, the location variable will be interpreted as the distance from the edge specified.  
GTK+'s window positioning is known to be finicky.  
Default setting is 1 (Top-left).  
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
### Tips
To hide the console cursor, perform `printf` or `echo -n` spaces until out of bound, or by using `echo -n "$(command)[SPACES]"` on last output command.  
For the latter method, if colourised output from pipe is required, `unbuffer` from package `expect` can be appended in front of the command.
