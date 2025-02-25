IDIR = ./include

CC = gcc
CFLAGS = -Wall -g -I$(IDIR)
PROG = tinyFSDemo
OBJS = tinyFSDemo.o libTinyFS.o libDisk.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

tinyFSDemo.o: ./src/tinyFSDemo.c
	$(CC) $(CFLAGS) -c -o $@ $<

libTinyFS.o: ./src/libTinyFS.c libDisk.o
	$(CC) $(CFLAGS) -c -o $@ $<

libDisk.o: ./src/libDisk.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS)
