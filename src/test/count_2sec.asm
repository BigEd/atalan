timer equ 20
count equ 128
   org $2e0
   dta a($2000)
   org $2000
;(9) 

   jsr _std_print_out
   dta b(21),c'Counting 2 seconds...'
;(10) "Counting 2 seconds..."

   dta b(128)
   dta b(0)
;(12) count = 0

   lda #0
   sta count
   sta count+1
   sta count+2
;(13) timer = 0

   sta timer
   tax
   tay
   jmp _lbl1
_lbl3:
;(17) 

   inx
   jne _lbl4
   iny
   jne _lbl4
   inc count+2
_lbl4:
_lbl1:
;(15) until timer = 100

   lda timer
   cmp #100
   jeq _lbl2
   jmp _lbl3
_lbl2:
   sty count+1
   stx count
   jsr _std_print_out
   dta b(11),c'Counted to '
   dta b(131),a(count)
   dta b(1),c'.'
;(18) "Counted to [count]."

   dta b(128)
   dta b(0)
;(20) 
   jmp *
   icl 'atari.asm'
