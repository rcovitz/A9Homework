a9: main.c
	gcc -o a9 main.c

run:
	gcc -o a9 main.c && ./a9

memcheck:
	gcc -o a9 main.c && valgrind --leak-check=yes ./a9
