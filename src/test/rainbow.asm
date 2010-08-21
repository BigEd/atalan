VCOUNT equ 54283
WSYNC equ 54282
button__state__pressed equ 0
STRIG equ 644
COL__BK equ 53274
COL__PF2 equ 53272
RTCLOCK equ 20
_s0__c equ 132
_s0___7 equ 133
_s0___8 equ 135
_10 equ 137
   org $2e0
   dta a($2000)
   org $2000
   ldx _s0___7+0
   ldy _s0__c
   jmp _lbl1
_lbl3:
   lda VCOUNT
   asl
   tax
   lda #0
   rol
   sta _s0___7+1
   txa
   clc
   adc RTCLOCK
   sta _s0___8
   lda #0
   rol
   sta _s0___8+1
   lda _s0___8
   tay
   sta WSYNC
   sta COL__PF2
   sta COL__BK
_lbl1:
   lda STRIG
   sta _10
   cmp #button__state__pressed
   jeq _lbl2
   jmp _lbl3
_lbl2:
   jmp *
   icl 'atari.asm'
