export LC_ALL = C

PRG = atalan
OBJ = *.o
CONFIGURATION = 

SYSTEM_NAME := $(shell uname -s)
SYSTEM_ARCH := $(shell uname -m)


CHARSET = UTF-8

prefix = $(DESTDIR)/usr/local
BINDIR = $(DESTDIR)/usr/local/bin
LIBDIR = $(DESTDIR)/usr/local/lib
MANDIR = $(DESTDIR)/usr/local/share/man

SOURCES= common.c emit.c errors.c instr.c lexer.c main.c optimize.c parser.c type.c variables.c
OBJS= common.o emit.o errors.o instr.o lexer.o main.o optimize.o parser.o type.o variables.o

CC = gcc
CXX = gcc

CFLAGS = -g -Wall -D__$(SYSTEM_NAME)__ -D__$(SYSTEM_ARCH)__
###### generic rules #######
all:	atalan
	@echo System name: $(SYSTEM_NAME)
	@echo System Arch: $(SYSTEM_ARCH)

atalan: $(OBJS)

	@$(CC) $(CFLAGS) -o $@ $(OBJS) && echo "Compile OK"

%.o: %.c

	@$(CC) -c $(CFLAGS) $<

%.asm: %.atl
	./atalan -a -I src examples/$<

%.xex: %.asm
	mads examples/$< -i:./src -o:examples/$@ -x

clean:
	rm -f atalan $(OBJS)

