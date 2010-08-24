export LC_ALL = C

PRG = atalan

SYSTEM_NAME := $(shell uname -s)
SYSTEM_ARCH := $(shell uname -m)


CHARSET = UTF-8

prefix = $(DESTDIR)/usr/local
BINDIR = $(DESTDIR)/usr/local/bin
LIBDIR = $(DESTDIR)/usr/local/lib
MANDIR = $(DESTDIR)/usr/local/share/man

SOURCES= common.c emit.c errors.c instr.c lexer.c main.c optimize.c parser.c type.c variables.c

CC = gcc
CXX = gcc

CFLAGS = -Wall -DSYSTEM_NAME=$(SYSTEM_NAME) -DSYSTEM_ARCH=$(SYSTEM_ARCH)
###### generic rules #######
all:	atalan

atalan:
	@echo System name: $(SYSTEM_NAME)
	@echo System Arch: $(SYSTEM_ARCH)
	cd src; make

clean:
	cd src; make clean
	cd examples; make clean

examples:
	cd examples; make all

