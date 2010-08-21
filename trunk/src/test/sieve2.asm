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
key__none equ 255
RTCLOCK equ 20
CHAR1 equ 754
k equ 130
j equ 132
time equ 134
i equ 135
   org $2e0
   dta a($2000)
   org $2000
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
   jne _lab1
   inc k+1
_lab1:
   sty k
   lda k
   cmp #<8192
   jne _lbl2
   lda k+1
   cmp #>8192
   jne _lbl2
   lda #0
   sta RTCLOCK
   lda #2
   sta i
   ldy j+0
_lbl4:
   ldx i
   lda sieve,x
   cmp #1
   bne _lbl5
   lda i
   asl
   sta j
   lda #0
   sta j+1
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
   jcc _skip1
   inc j+1
_skip1:
_lbl6:
   sty j
   lda j+1
   cmp #>8191
   jcc _lbl7
   bne ?skip1
   lda j
   cmp #<8191
   jcc _lbl7
?skip1:
_lbl5:
   inc i
   lda i
   cmp #91
   jne _lbl4
   sty j
   lda RTCLOCK
   sta time
   jsr _std_print
   dta b(11),c'Time used: '
   dta b(129),a(time)
   dta b(6),c' ticks'
   dta b(128)
   dta b(0)
   jsr _std_print
   dta b(39),c'Press Q to quit, any other key for list'
   dta b(128)
   dta b(0)
   lda #key__none
   sta CHAR1
   jmp _lbl8
_lbl9:
_lbl8:
   lda CHAR1
   cmp #key__none
   jeq _lbl9
   jmp (10)
   icl 'atari.asm'
sieve:
   .ds 8192
