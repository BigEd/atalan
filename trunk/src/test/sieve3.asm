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
RTCLOCK equ 20
k equ 130
j equ 132
time equ 134
i equ 135
   org $2e0
   dta a($2000)
   org $2000
   jsr _std_print
   dta b(19),c'Computing primes...'
   dta b(128)
   dta b(0)
   lda #0
   sta k
   sta k+1
   tay
_lbl2:
   lda #<sieve
   sta _arr
   lda #>sieve
   clc
   adc k+1
   sta _arr+1
   lda #1
   sta (_arr),y
   iny
   jne _lbl8
   inc k+1
_lbl8:
   tya
   cpy #0
   jne _lbl2
   lda k+1
   cmp #32
   jne _lbl2
   sty k
   lda #0
   sta RTCLOCK
   lda #2
   sta i
_lbl4:
   ldx i
   lda sieve,x
   cmp #1
   jne _lbl5
   lda i
   asl
   sta j
   lda #0
   sta j+1
   tay
   jmp _lbl6
_lbl7:
   lda #<sieve
   sta _arr
   lda #>sieve
   clc
   adc j+1
   sta _arr+1
   lda #0
   sta (_arr),y
   tya
   clc
   adc i
   tay
   jcc _lbl9
   inc j+1
_lbl9:
_lbl6:
   lda j+1
   cmp #31
   jcc _lbl7
   jne _lbl10
   tya
   cpy #255
   jcc _lbl7
   sty j
_lbl10:
_lbl5:
   inc i
   lda i
   cmp #91
   jne _lbl4
   lda RTCLOCK
   sta time
   jsr _std_print
   dta b(11),c'Time used: '
   dta b(129),a(time)
   dta b(6),c' ticks'
   dta b(128)
   dta b(0)
   jmp *
   icl 'atari.asm'
sieve:
   .ds 8192
