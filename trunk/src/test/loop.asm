sum equ 128
_s0__i equ 129
sum2 equ 130
_s1__i equ 131
_s2__i equ 132
r equ 133
s equ 134
_s4___7 equ 135
_s4__i equ 136
double_range__s equ 137
double_range__a equ 138
double_range__b equ 139
_s5__i equ 140
   org $2e0
   dta a($2000)
   org $2000
;(12) sum = 0

   lda #0
   sta sum
;(14) 

   lda #5
   sta _s0__i
_lbl2:
;(13) for i : 5..10 sum = sum + i

   lda sum
   clc
   adc _s0__i
   sta sum
   inc _s0__i
   lda _s0__i
   cmp #11
   jne _lbl2
;(15) sum2 = 5 + 6 + 7 + 8 + 9 + 10

   lda #45
   sta sum2
;(16) 

   jsr _std_print_out
   dta b(6),c'Range '
   dta b(129),a(sum)
;(17) "Range [sum]"  ;45

   dta b(128)
   dta b(0)
;(21) sum = 0

   lda #0
   sta sum
;(23) 

   lda #5
   sta _s1__i
_lbl4:
;(22) for i:range sum = sum + i

   lda sum
   clc
   adc _s1__i
   sta sum
   inc _s1__i
   lda _s1__i
   cmp #11
   jne _lbl4
   jsr _std_print_out
   dta b(5),c'Type '
   dta b(129),a(sum)
;(24) "Type [sum]"

   dta b(128)
   dta b(0)
;(28) sum = 0

   lda #0
   sta sum
;(30) 

   lda #5
   sta _s2__i
_lbl6:
;(29) for i:range sum = sum + i

   lda sum
   clc
   adc _s2__i
   sta sum
   inc _s2__i
   lda _s2__i
   cmp #11
   jne _lbl6
   jsr _std_print_out
   dta b(4),c'Var '
   dta b(129),a(sum)
;(31) "Var [sum]"

   dta b(128)
   dta b(0)
;(36) sum = 0

   lda #0
   sta sum
;(38) 

   lda #5
   sta r
_lbl8:
;(37) for r sum = sum + r

   lda sum
   clc
   adc r
   sta sum
   inc r
   lda r
   cmp #11
   jne _lbl8
   jsr _std_print_out
   dta b(11),c'Global var '
   dta b(129),a(sum)
;(39) "Global var [sum]"

   dta b(128)
   dta b(0)
;(43) in s:0..100 = 5

   lda #5
   sta s
;(45) sum = 0

   lda #0
   sta sum
   lda s
   clc
   adc #5
   sta _s4___7
;(47) 

   lda s
   sta _s4__i
_lbl10:
;(46) for i:s..s+5 sum = sum + i

   lda sum
   clc
   adc _s4__i
   sta sum
   inc _s4__i
   lda _s4__i
   cmp _s4___7
   jcc _lbl10
   jeq _lbl10
   jsr _std_print_out
   dta b(15),c'Variable range '
   dta b(129),a(sum)
;(48) "Variable range [sum]"

   dta b(128)
   dta b(0)
;(59) sum = 0

   lda #0
   sta sum
   lda #5
   sta double_range__s
   jsr double_range
;(61) 

   lda double_range__a
   sta _s5__i
_lbl12:
;(60) for i:double_range(5) sum = sum + i

   lda sum
   clc
   adc _s5__i
   sta sum
   inc _s5__i
   lda _s5__i
   cmp double_range__b
   jcc _lbl12
   jeq _lbl12
   jsr _std_print_out
   dta b(15),c'Function range '
   dta b(129),a(sum)
;(62) "Function range [sum]"

   dta b(128)
   dta b(0)
;(65) 
   jmp *
   icl 'atari.asm'
double_range .proc
;(56) 	a = s

   lda double_range__s
   sta double_range__a
;(57) 	b = s * 2

   lda double_range__s
   asl
   sta double_range__b
   rts
.endp
