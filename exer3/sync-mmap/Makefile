#
# Makefile
# 

CC = gcc

# CAUTION: Always use '-pthread' when compiling POSIX threads-based
# applications, instead of linking with "-lpthread" directly.
CFLAGS = -Wall -O2 -pthread
LIBS = 

all: mandel-fork-sema mandel-fork-no-sema


## Mandel

mandel-fork-sema: mandel-lib.o mandel-fork-sema.o
	$(CC) $(CFLAGS) -o mandel-fork-sema mandel-lib.o mandel-fork-sema.o $(LIBS)

mandel-lib.o: mandel-lib.h mandel-lib.c
	$(CC) $(CFLAGS) -c -o mandel-lib.o mandel-lib.c $(LIBS)

mandel-fork-sema.o: mandel-fork-sema.c
	$(CC) $(CFLAGS) -c -o mandel-fork-sema.o mandel-fork-sema.c $(LIBS)

mandel-fork-no-sema: mandel-lib.o mandel-fork-no-sema.o
	$(CC) $(CFLAGS) -o mandel-fork-no-sema mandel-lib.o mandel-fork-no-sema.o $(LIBS)

mandel-fork-no-sema.o: mandel-fork-no-sema.c
	$(CC) $(CFLAGS) -c -o mandel-fork-no-sema.o mandel-fork-no-sema.c $(LIBS)

clean:
	rm -f *.s *.o mandel-fork-sema mandel-fork-no-sema 
