_arr equ 128
_0 equ 131
RANDOM equ 53770
CHBAS equ 756
LMARGN equ 82
RMARGN equ 83
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
SDLSTL equ 560
LM equ 15
RM equ 25
BM equ 22
x equ 132
y equ 133
j equ 134
k equ 135
_1 equ 136
   org $2e0
   dta a($2000)
   org $2000
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
   lda #1
   sta y
_lbl2:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy #LM
   lda #5
   sta (_arr),y
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy #RM
   lda #5
   sta (_arr),y
   inc y
   lda y
   cmp #23
   jne _lbl2
   lda #15
   sta x
_lbl4:
   ldy #BM
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda #5
   sta (_arr),y
   inc x
   lda x
   cmp #26
   jne _lbl4
   lda #19
   sta x
   lda #1
   sta y
   lda #0
   sta j
   ldx #19
_lbl6:
   lda #0
   sta k
_lbl8:
   txa
   clc
   adc k
   sta _0
   lda y
   clc
   adc j
   sta _1
   tay
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy _0
   lda #3
   sta (_arr),y
   inc k
   lda k
   cmp #4
   jne _lbl8
   inc j
   lda j
   cmp #4
   jne _lbl6
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
s:
   .ds 40*24
s_lo:  :24 dta l(s + #*40)
s_hi:  :24 dta h(s + #*40)
