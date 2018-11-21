# Simple shell

![Coverity](https://scan.coverity.com/projects/17208/badge.svg)

A simple shell.

## Screencast:

[![asciicast](https://asciinema.org/a/BvThSoa4Q7jW8ymcYkqoZ7vSb.svg)](https://asciinema.org/a/BvThSoa4Q7jW8ymcYkqoZ7vSb)

## Usage:

```
ss - help:

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
		  routine> [code]
		  routine> end

listroutines - Lists routines
		Usage: listroutines

unroutine - Deletes a routine
		Usage: unroutine [name]

help - Display this.  If there's one command that you don't want to forget, it's this one
		Usage: help
```

See **ss(1)** for more (or not).

## How to build

```
$ ./build.sh
$ ./build.sh install
```

## How to uninstall

```
$ ./build.sh uninstall
```

## Future features:

- [x] Basic usability
- [x] Variables
- [x] Set `$SHELL`
- [x] `$?`, `$PWD`, `$OWD`
- [ ] Replace home path with `~` in prompt
- [ ] Escape special chars (`\`)
- [x] Basic routines
- [x] Routine arguments (`$#`, `$1`, `$2`...)
- [ ] Pipes
- [ ] File redirection (`>`)
- [ ] if/else
- [ ] Loops
- [ ] Beat `s`'s level of functionality
- [ ] Usable as a replacement for normal shells (`s` doesn't completely reach this in my opinion)

## Possible future features:
- Globbing
- Switch to linenoise or something similar
- `<` redirection
- Arrays
- Lisp-style command nesting (`echo (echo hi)`)
- Character aliases (`\n`, `\r`, etc)
- Customizable prompt

## Other info

Inspired by [s](https://github.com/rain-1/s), [rc](http://doc.cat-v.org/plan_9/4th_edition/papers/rc), [es](https://github.com/wryun/es-shell), and the Suckless people