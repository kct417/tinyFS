IDIR = ./include

CC = gcc
CFLAGS = -Wall -g -I$(IDIR)
PROG = tinyFSDemo
OBJS = tinyFSDemo.o libTinyFS.o libDisk.o

TEST_PROG = diskTest
TEST_SRC = ./test/diskTest.c
TEST_OBJ = ./test/diskTest.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

tinyFSDemo.o: ./src/tinyFSDemo.c
	$(CC) $(CFLAGS) -c -o $@ $<

libTinyFS.o: ./src/libTinyFS.c libDisk.o
	$(CC) $(CFLAGS) -c -o $@ $<

libDisk.o: ./src/libDisk.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_PROG): $(TEST_OBJ) libDisk.o
	$(CC) $(CFLAGS) -o $(TEST_PROG) $(TEST_OBJ) libDisk.o

$(TEST_OBJ): $(TEST_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS) $(TEST_PROG) $(TEST_OBJ) libDisk.o
	rm -f *.dsk
