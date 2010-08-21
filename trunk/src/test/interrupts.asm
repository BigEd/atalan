_arr equ 128
WSYNC equ 54282
color__Black equ 0
color__White equ 15
COLOR0 equ 708
COLPF equ 53270
COLBK equ 53274
dl_command__BLANK8 equ 112
dl_command__CHR0 equ 2
dl_command__NEXT equ 65
SDLSTL equ 560
NMIEN equ 54286
VDSLST equ 512
VVBLKD equ 548
_4 equ 130
_5 equ 132
   org $2e0
   dta a($2000)
   org $2000
   lda #<gr0
   sta SDLSTL
   lda #>gr0
   sta SDLSTL+1
   lda #72
   sta COLOR0+2
   lda #color__Black
   sta COLOR0+5
   ldy #1
   lda s_lo,y
   clc
   adc #1
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(37),c'Demonstration of Interrupts in ATALAN'
   dta b(0)
   lda #<cycle
   sta _4
   lda #>cycle
   sta _4+1
   lda _4
   sta VVBLKD
   lda _4+1
   sta VVBLKD+1
   lda #<set_col
   sta _5
   lda #>set_col
   sta _5+1
   lda _5
   sta VDSLST
   lda _5+1
   sta VDSLST+1
   lda #192
   sta NMIEN
   jmp *
   icl 'atari.asm'
gr0:
   dta b(dl_command__BLANK8)
   dta b(dl_command__BLANK8)
   dta b(dl_command__BLANK8)
   dta b(66)
   dta a(s)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(130)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__CHR0)
   dta b(dl_command__NEXT)
   dta a(gr0)
   dta a(gr0)
s_lo:  :24 dta l(s + #*40)
s_hi:  :24 dta h(s + #*40)
s:
   .ds 40*24
set_col .proc
   pha
   txa
   pha
   tya
   pha
   sta WSYNC
   lda #color__White
   sta COLPF+2
   pla
   tay
   pla
   tax
   pla
   rti
.endp
cycle .proc
   inc COLOR0+5
   lda COLOR0+5
   sta COLBK
   jmp $e462
.endp
