CC:=gcc
CFLAGS:=-g -Wall -Werror 

all: tests disk file

tests: apps/tests.c file disk/diskcontrol.c disk/diskcontrol.h
	$(CC) $(CFLAGS) -o apps/$@ $^

disk: disk/diskcontrol.c disk/diskcontrol.h
	$(CC) $(CFLAGS) -c -o $@ $<

file: io/File.c io/File.h disk/diskcontrol.c disk/diskcontrol.h
	$(CC) $(CFLAGS) -c -o $@ $<
