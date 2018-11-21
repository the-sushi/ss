# Contribution guidelines

## Bug report?
Use GitHub's issue reporting feature with the bug report template.
If you do not have a GitHub account, you may email me at joshpritsker@protonmail.com, however there is no garuntee that I'll read your message.

## Patch / pull request?
Use GitHub's pull request feature.  Make sure that you clearly describe your changes, and why they should be added.
Before submitting, **please test your changes** (make sure it works without crash or anything) and make sure they follow the code style.

## Feature request?
Use GitHub's issue reporting feature with the feature addition template.  Do not email me for this (even if you don't have an account).

After some point in time, feature requests stop being accepted.  See the code style section.

## Code style violation or suggestion?
Use GitHub's issue reporting feature with no template.  Do not email me for this (even if you don't have an account).

## Code style:
Anything that should be accessible globally goes in `globals.h`, this may be hard to work with in larger projects, but for small ones like this it works great.

It is not planned for `ss` to exceed 800-1500 lines, it is meant to be small.

`ss` is written in C89/C90.  For this reason, C++ style comments are not used.

There are 3 kinds of if statements that you may write:
```
/* 1 */
if (x == y) stuff

/* 2 */
if (x == y)
{
	stuff
}

/* 3 */
if
	(
		x == y ||
		z == w
	)
{
	stuff
}
```

The same applies to all other similar things, such as while, for, switch, etc.

Braces always go on the next line.  Structs are never `typedef`ed.