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

CC = gcc
CXX = gcc

CFLAGS = -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wno-pointer-sign -Wdeclaration-after-statement -std=gnu99 -Wall -Wno-switch -Wpointer-arith -Wredundant-decls -O4 -march=pentium-m -mtune=pentium-m -pipe -ffast-math -fomit-frame-pointer -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -Ilibdvdread4 -mdynamic-no-pic -falign-loops=16 -shared-libgcc -I.   -I/usr/X11/include -I/usr/X11/include/freetype2 -I/usr/X11/include
CFLAGS = -Wall -DSYSTEM_NAME=$(SYSTEM_NAME) -DSYSTEM_ARCH=$(SYSTEM_ARCH)
###### generic rules #######
all:	atalan

atalan: $(SOURCES)

	@echo System name: $(SYSTEM_NAME)
	@echo System Arch: $(SYSTEM_ARCH)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) && echo "Compile OK"

%.asm: %.atl
	./atalan -a -I src examples/$<

%.xex: %.asm
	mads examples/$< -i:./src -o:examples/$@ -x

clean:
	rm -f atalan $(/*.o)
	rm -f examples/*.asm examples/*.xex

