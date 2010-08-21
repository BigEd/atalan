_arr equ 128
SDLSTL equ 560
timer equ 20
RANDOM equ 53770
CHBAS equ 756
LMARGN equ 82
RMARGN equ 83
VCOUNT equ 54283
WSYNC equ 54282
STRIG equ 644
STICK equ 632
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
sum__r equ 131
sumdiv__e equ 132
sumdiv__f equ 133
sumdiv__s equ 134
sumdiv__d equ 135
a equ 136
b equ 137
c equ 138
   org $2e0
   dta a($2000)
   org $2000
   lda #10
   sta c
   sta sumdiv__e
   lda #3
   sta sumdiv__f
   jsr sumdiv
   lda sumdiv__s
   sta a
   lda sumdiv__d
   sta b
   jsr _std_print_out
   dta b(7),c'Sum is '
   dta b(129),a(a)
   dta b(9),c', div is '
   dta b(129),a(b)
   dta b(128)
   dta b(0)
   jmp *
   icl 'atari.asm'
sum .proc
   lda sum__e
   clc
   adc sum__f
   sta sum__r
   rts
.endp
sumdiv .proc
   lda sumdiv__e
   clc
   adc sumdiv__f
   sta sumdiv__s
   lda sumdiv__e
   sec
   sbc sumdiv__f
   sta sumdiv__d
   rts
.endp
