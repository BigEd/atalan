roz_y equ 128
roz_x equ 129
yy equ 130
xx equ 131
   org $2e0
   dta a($2000)
   org $2000
;(2) roz_y = 6

   lda #6
   sta roz_y
;(3) roz_x = 8

   lda #8
   sta roz_x
;(5) yy = 0

   lda #0
   sta yy
;(6) xx = 0

   sta xx
   jmp _lbl1
_lbl6:
;(9)       xx = 0

   lda #0
   sta xx
   tax
   ldy yy
   jmp _lbl3
_lbl5:
;(11)                 inc xx

   inx
;(13) 

   iny
_lbl3:
;(10)       while xx < roz_x

   txa
   cpx roz_x
   jcs _lbl4
   jmp _lbl5
_lbl4:
   sty yy
   stx xx
_lbl1:
;(8) while yy < roz_y

   lda yy
   cmp roz_y
   jcs _lbl2
   jmp _lbl6
_lbl2:
   jsr _std_print_out
   dta b(17),c'xx after loops = '
   dta b(129),a(xx)
;(14) "xx after loops = [xx]"

   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(17),c'yy after loops = '
   dta b(129),a(yy)
;(15) "yy after loops = [yy]"

   dta b(128)
   dta b(0)
;(17) 
   jmp *
   icl 'atari.asm'
