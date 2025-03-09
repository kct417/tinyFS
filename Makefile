IDIR = ./include
TDIR = ./test

CC = gcc
CFLAGS = -Wall -g -I$(IDIR) -std=gnu99
PROG = tinyFSDemo
OBJS = tinyFSDemo.o libTinyFS.o libDisk.o

TEST_PROG = tfsTest
TEST_OBJS = $(TDIR)/tfsTest.o libDisk.o libTinyFS.o

all: $(TEST_PROG)

$(TEST_PROG): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_PROG) $(TEST_OBJS)

$(TDIR)/tfsTest.o: $(TDIR)/tfsTest.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

tinyFSDemo.o: ./src/tinyFSDemo.c
	$(CC) $(CFLAGS) -c -o $@ $<

libTinyFS.o: ./src/libTinyFS.c libDisk.o
	$(CC) $(CFLAGS) -c -o $@ $<

libDisk.o: ./src/libDisk.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm -f $(PROG) $(OBJS) $(TEST_PROG) $(TEST_OBJ) libDisk.o tinyFSDisk diskTest 
	
	rm -f *.dsk
