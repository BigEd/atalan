_arr equ 128
RANDOM equ 53770
CHBAS equ 756
VCOUNT equ 54283
WSYNC equ 54282
STRIG0 equ 644
STRIG1 equ 645
DMACTL equ 559
COLOR0 equ 708
player_size equ 53256
player_x equ 53248
player_gfx equ 53261
player_color equ 704
player_col2 equ 53266
missile_size equ 53260
missile_x equ 53252
GRACTL equ 53277
AUDF1 equ 53760
AUDC1 equ 53761
AUDCTL equ 53768
SKCTL equ 53775
CH equ 764
DOSVEC equ 10
   org $2e0
   dta a($2000)
   org $2000
   jsr test
   jmp *
   icl 'atari.asm'
test .proc
   jsr _std_print
   dta b(12),c'To jest test'
   dta b(128)
   dta b(0)
   rts
.endp
