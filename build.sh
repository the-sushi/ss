#!/bin/sh

if [ "$1" == "debug" ]
then
	echo "Building: Debug"
	$CC --std=c90 -Wall *.c -ledit -o ss -g
elif [ "$1" == "" ]
then
	echo "Building"
	$CC --std=c90 -Wall -O3 *.c -ledit -o ss
elif [ "$1" == "install" ]
then
	echo "Installing"
	cp ss.1 /usr/share/man/man1/
	install ss /usr/local/bin
elif [ "$1" == "uninstall" ]
then
	echo "Removing files"
	rm /usr/share/man/man1/ss.1
	rm /usr/local/bin/ss
else
	echo "Incorrect flag"
	exit 1
fi

echo "Done."