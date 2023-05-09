CC=gcc
CFLAGS=-Wall -ggdb -O2 -fPIC
LDFLAGS=-ldl

.PHONY : all clean

all : fork_preload.so


fork_preload.so : fork_preload.c
	$(CC) -shared -o $@ $^ ${CFLAGS} ${LDFLAGS}

clean :
	-rm -f fork_preload.so

