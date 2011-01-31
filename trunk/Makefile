export LC_ALL = C

SYSTEM_NAME := $(shell uname -s)
SYSTEM_ARCH := $(shell uname -m)

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

