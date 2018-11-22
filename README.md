# Simple shell

![Coverity](https://scan.coverity.com/projects/17208/badge.svg)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/1d04b853d2554d3fa3676f62458061e3)](https://www.codacy.com/app/joshpritsker/ss?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=the-sushi/ss&amp;utm_campaign=Badge_Grade)

A simple shell.

## Screencast

[![asciicast](https://asciinema.org/a/BvThSoa4Q7jW8ymcYkqoZ7vSb.svg)](https://asciinema.org/a/BvThSoa4Q7jW8ymcYkqoZ7vSb)

## Usage

```
ss(1)                                SS(1)                               ss(1)



NAME
       ss - Simple Shell


DESCRIPTION
       ss,  or  Simple  Shell,  is a minimal command interpreter for UNIX-like
       systems. It is not sh-compatible, nor does it aim to be.


   LIST OF BUILTINS
       ss, cd, set, echo, pwd, exit, exec, routine,  unroutine,  listroutines,
       help



   BUILTIN USAGE
       cd     Change directory

              Usage: cd [directory]


       set    Set enviromental variable

              Usage: set [variable] [value]


       unset  Unset enviromental variable

              Usage: unset [variable]


       echo   Echo arguments back

              Usage: echo [text] <more text...>


       pwd    Path of working directory

              Usage: pwd


       exit   Exits shell

              Usage: exit <error code>


       routine
              Creates a new routine

              Usage:

              > routine [name]

                   routine> [code]

                   routine> end


       listroutines
              Lists added routines

              Usage: listroutines


       unroutine
              Deletes a routine

              Usage: unroutine [name]


       help   Display  this  help page.  If there's one command that you don't
              want to forget, it's this one

              Usage: help


SEE ALSO
       sh(1), s(1), execline(1), rc(1), es(1)


NOTES
       Is currently under development and is a WIP, not featureful enough  for
       normal usage



ss, version WIP0                   2018-11-8                             ss(1)
```

## How to build

```
$ ./build.sh
$ ./build.sh install
```

## How to uninstall

```
$ ./build.sh uninstall
```

## Future features

-  [x] Basic usability
-  [x] Variables
-  [x] Set `$SHELL`
-  [x] `$?`, `$PWD`, `$OWD`
-  [ ] Replace home path with `~` in prompt
-  [ ] Escape special chars (`\`)
-  [x] Basic routines
-  [x] Routine arguments (`$#`, `$1`, `$2`...)
-  [ ] Pipes
-  [ ] File redirection (`>`)
-  [ ] if/else
-  [ ] Loops
-  [ ] Beat `s`'s level of functionality
-  [ ] Usable as a replacement for normal shells (`s` doesn't completely reach this in my opinion)

## Possible future features
-  Globbing
-  Switch to linenoise or something similar
-  `<` redirection
-  Arrays
-  Lisp-style command nesting (`echo (echo hi)`)
-  Character aliases (`\n`, `\r`, etc)
-  Customizable prompt

## Other info

Inspired by [s](https://github.com/rain-1/s), [rc](http://doc.cat-v.org/plan_9/4th_edition/papers/rc), [es](https://github.com/wryun/es-shell), and the Suckless people
