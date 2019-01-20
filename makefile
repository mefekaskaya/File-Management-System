all:
	gcc -o test.out test.c fms.h fms.c -std=c99 -ljson -I.