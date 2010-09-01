ATLDESTS= fifteen.xex

all: $(ATLDESTS)

fifteen.xex: fifteen.asm
	mads $< -i:../../src -o:$@ -x
	dd if=$@ of=$@.in bs=1 skip=2
	cp little15music.obj $@
	cat $@.in >>$@
	rm $@.in

%.asm: %.atl
	../../src/atalan -a -I ../../src $<

clean:
	rm -fv *.xex *.asm

