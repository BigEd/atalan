inc___0 equ 128
RANDOM equ 53770
x equ 132
   org $2e0
   dta a($2000)
   org $2000
   lda RANDOM
   and #63
   sta inc___0
   asl
   sta x
   jsr _std_print_out
   dta b(129),a(x)
   dta b(128)
   dta b(0)
   jmp *
   icl 'atari.asm'
