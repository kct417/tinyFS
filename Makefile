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

# Object Files
OBJS = $(ODIR)/tinyFSDemo.o $(ODIR)/tinyFS.o $(ODIR)/disk.o $(ODIR)/diskTest.o $(ODIR)/tfsTest.o

# Default
all: $(PROG)

# Make
libs: $(LIBS)
tests: $(TESTS)

# Program
tinyFSDemo: $(ODIR)/tinyFSDemo.o $(LDIR)/libTinyFS.a
	$(CC) $(CFLAGS) -o $@ $< -L$(LDIR) -lTinyFS

# Libraries
$(LDIR)/libTinyFS.a: $(ODIR)/disk.o $(ODIR)/tinyFS.o
	@mkdir -p $(LDIR)
	ar r $@ $^

$(LDIR)/libDisk.a: $(ODIR)/disk.o
	@mkdir -p $(LDIR)
	ar r $@ $^

# Test Programs
diskTest: $(ODIR)/diskTest.o $(LDIR)/libDisk.a
	$(CC) $(CFLAGS) -o diskTest $< -L$(LDIR) -lDisk

tfsTest: $(ODIR)/tfsTest.o $(LDIR)/libTinyFS.a
	$(CC) $(CFLAGS) -o tfsTest $< -L$(LDIR) -lTinyFS

# Program Object Files
$(ODIR)/tinyFSDemo.o: $(SDIR)/tinyFSDemo.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/tinyFS.o: $(SDIR)/tinyFS.c $(ODIR)/disk.o
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/disk.o: $(SDIR)/disk.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

# Test Program Object Files
$(ODIR)/diskTest.o: $(TDIR)/diskTest.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/tfsTest.o: $(TDIR)/tfsTest.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

# Clean
clean:
	rm -rf $(ODIR)

clean-fs:
	rm -f *.dsk

clean-all:
	rm -f $(PROG)
	rm -f $(TESTS)
	rm -rf $(LDIR)
	rm -rf $(ODIR)
	rm -f *.dsk

# Rebuild
rebuild: clean-all
	make

rebuild-tests: clean-all
	make tests