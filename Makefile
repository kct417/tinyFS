# $@: target
# $^: all prerequisites
# $<: first prerequisite

# File Architecture
SDIR = ./src
IDIR = ./include
LDIR = ./lib
ODIR = ./obj
TDIR = ./test_src

# Compiler
CC = gcc
CFLAGS = -Wall -std=c99 -g -I$(IDIR)

# Programs
PROG = tinyFSDemo

# Libraries
LIBS = $(LDIR)/libTinyFS.a $(LDIR)/libDisk.a

# Test Programs
TESTS = diskTest tfsTest

# Default
all: $(PROG) $(TESTS)

# Make
libs: $(LIBS)
tests: $(TESTS)

# Program Object Files
$(ODIR)/tinyFSDemo.o: $(SDIR)/tinyFSDemo.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/libTinyFS.o: $(SDIR)/libTinyFS.c $(ODIR)/libDisk.o
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/libDisk.o: $(SDIR)/libDisk.c
	$(CC) $(CFLAGS) -c -o $@ $^

# Libraries
$(LDIR)/libTinyFS.a: $(ODIR)/libDisk.o $(ODIR)/libTinyFS.o
	ar r $@ $^

$(LDIR)/libDisk.a: $(ODIR)/libDisk.o
	ar r $@ $^

# Program
tinyFSDemo: $(ODIR)/tinyFSDemo.o $(LDIR)/libTinyFS.a
	$(CC) $(CFLAGS) -o $@ $< -Llib -lTinyFS

# Test Object Files
$(ODIR)/diskTest.o: $(TDIR)/diskTest.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/tfsTest.o: $(TDIR)/tfsTest.c
	$(CC) $(CFLAGS) -c -o $@ $^

# Test Programs
diskTest: $(ODIR)/diskTest.o $(LDIR)/libDisk.a
	$(CC) $(CFLAGS) -o diskTest $< -L$(LDIR) -lDisk

tfsTest: $(ODIR)/tfsTest.o $(LDIR)/libTinyFS.a
	$(CC) $(CFLAGS) -o tfsTest $< -L$(LDIR) -lTinyFS

clean:
	rm -f $(ODIR)/*.o $(LDIR)/*.a *.dsk afile bfile

cleanall:
	rm -f $(PROG) $(TESTS) $(ODIR)/*.o $(LDIR)/*.a *.dsk afile bfile

cleandisk:
	rm -f *.dsk
