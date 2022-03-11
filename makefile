
all : avltest

avltree.c : avltree.h

avltest: avltree.c avltest.c
	cc -g -o $@ avltree.c avltest.c -l m

run: avltest
	./avltest -l -i 1000
