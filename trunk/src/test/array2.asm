_arr equ 128
_0 equ 130
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
w equ 131
h equ 132
w1 equ 133
h1 equ 134
_99 equ 135
   org $2e0
   dta a($2000)
   org $2000
   lda #0
   sta LMARGN
   jsr _std_print
   dta b(128)
   dta b(0)
   sta h
_lbl8:
   lda #0
   sta w
_lbl10:
   ldy w
   lda p_lo,y
   sta _arr
   lda p_hi,y
   sta _arr+1
   ldy h
   lda #0
   sta (_arr),y
   inc w
   lda w
   cmp #40
   jne _lbl10
   inc h
   lda h
   cmp #24
   jne _lbl8
   lda #1
   sta h1
_lbl12:
   lda #1
   sta w1
_lbl14:
   lda RANDOM
   cmp #100
   jcs _lbl15
   ldy w1
   lda p_lo,y
   sta _arr
   lda p_hi,y
   sta _arr+1
   ldy h1
   lda #1
   sta (_arr),y
_lbl15:
   inc w1
   lda w1
   cmp #39
   jne _lbl14
   inc h1
   lda h1
   cmp #23
   jne _lbl12
   jsr print
   lda #1
   sta h1
_lbl17:
   lda #1
   sta w1
_lbl19:
   lda h1
   clc
   adc #1
   sta _0
   ldy w1
   lda p_lo,y
   sta _arr
   lda p_hi,y
   sta _arr+1
   ldy _0
   lda (_arr),y
   tax
   cmp #1
   jne _lbl20
   lda h1
   clc
   adc #1
   sta _0
   ldy h1
   lda #1
   sta (_arr),y
_lbl20:
   inc w1
   lda w1
   cmp #39
   jne _lbl19
   inc h1
   lda h1
   cmp #23
   jne _lbl17
   stx _99
   jsr print
   jmp *
   icl 'atari.asm'
p:
   .ds 40*24
p_lo:  :40 dta l(p + #*24)
p_hi:  :40 dta h(p + #*24)
print .proc
   lda #1
   sta h1
_lbl2:
   lda #1
   sta w1
_lbl4:
   ldy w1
   lda p_lo,y
   sta _arr
   lda p_hi,y
   sta _arr+1
   ldy h1
   lda (_arr),y
   sta _99
   lda _99
   cmp #1
   jne _lbl5
   jsr _std_print
   dta b(1),c'*'
   dta b(0)
   jmp _lbl6
_lbl5:
   jsr _std_print
   dta b(1),c' '
   dta b(0)
_lbl6:
   inc w1
   lda w1
   cmp #39
   jne _lbl4
   jsr _std_print
   dta b(128)
   dta b(0)
   inc h1
   lda h1
   cmp #23
   jne _lbl2
   rts
.endp
