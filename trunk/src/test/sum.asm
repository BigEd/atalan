_arr equ 128
RANDOM equ 53770
CHBAS equ 756
VCOUNT equ 54283
WSYNC equ 54282
STRIG0 equ 644
STRIG1 equ 645
DMACTL equ 559
player_size equ 53256
player_x equ 53248
player_gfx equ 53261
player_color equ 704
player_col2 equ 53266
missile_size equ 53260
missile_x equ 53252
GRACTL equ 53277
CH equ 764
DOSVEC equ 10
a equ 130
b equ 131
c equ 132
   org $2e0
   dta a($2000)
   org $2000
   lda #10
   sta a
   lda #20
   sta b
   clc
   lda #30
   sta c
   jsr _std_print
   dta b(7),c'Sum of '
   dta b(129),a(a)
   dta b(5),c' and '
   dta b(129),a(b)
   dta b(4),c' is '
   dta b(129),a(c)
   dta b(1),c'.'
   dta b(128)
   dta b(0)
   jmp *
   icl 'atari.asm'
