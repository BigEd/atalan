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
   org $2e0
   dta a($2000)
   org $2000
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
   lda #0
   sta COLOR0+2
   sta COLOR0+5
   ldy #0
   lda s_lo,y
   clc
   adc #0
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(28),c'*** ATARI Using direct write'
   dta b(0)
   jmp *
   icl 'atari.asm'
dl:
   dta b(112)
   dta b(112)
   dta b(112)
   dta b(66)
   dta a(s)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(2)
   dta b(65)
   dta a(dl)
s_lo:  :24 dta l(s + #*40)
s_hi:  :24 dta h(s + #*40)
s:
   .ds 40*24
