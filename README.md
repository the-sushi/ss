# Simple shell

A simple shell.

## Usage:

```
cd - Change directory
		Usage: cd [directory]

set - Set enviromental variable
		Usage: set [variable] [value]

echo - Echo arguments back
		Usage: echo [text] <more text...>

pwd - Path of working directory
		Usage: pwd

exit - Exits shell
		Usage: exit <error code>

routine - Creates a new routine
		Usage:
~ > routine [name]
  routine > [code]
  routine > end

unroutine - Deletes a routine
		Usage: unroutine [name]

help - Display this.  If there's one command that you don't want to forget, it's this one
		Usage: help
```

## How to build

```
$ ./build.sh
$ install shell /path/to/your/bin
```