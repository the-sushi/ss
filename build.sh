if [ "$1" == "debug" ]
then
	$CC --std=c90 *.c -ledit -o ss -g
	cppcheck . --std=c89
else
	$CC --std=c90 -O3 *.c -ledit -o ss
fi