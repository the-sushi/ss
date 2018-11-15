if [ "$1" == "debug" ]
then
	$CC --std=c90 -Wall *.c -ledit -o ss -g
	cppcheck . --std=c89
else
	$CC --std=c90 -Wall -O3 *.c -ledit -o ss
fi