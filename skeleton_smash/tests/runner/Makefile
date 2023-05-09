CC=gcc
CFLAGS=-Wall -g
LDFLAGS=

.PHONY : all clean

all : runner printSignals


runner : runner.c
	$(CC) -o $@ $^ ${CFLAGS} ${LDFLAGS}
printSignals : printSignals.c
	$(CC) -o $@ $^ ${CFLAGS} ${LDFLAGS}
	cp $@ ../required_folder/$@.exe

clean :
	-rm -f runner
	-rm -f printSignals

