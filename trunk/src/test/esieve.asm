_arr equ 128
key__Q equ 47
key__none equ 255
CH equ 764
RTCLOCK equ 20
k equ 130
j equ 132
time equ 134
_s1__i equ 135
_5 equ 136
_6 equ 137
   org $2e0
   dta a($2000)
   org $2000
;(15) 

   jsr _std_print_out
   dta b(19),c'Computing primes...'
;(16) "Computing primes..."

   dta b(128)
   dta b(0)
;(20) 

   lda #0
   sta k
   sta k+1
   tay
_lbl2:
;(19) 	sieve(k) = 1

   lda #<sieve
   sta _arr
   lda #>sieve
   clc
   adc k+1
   sta _arr+1
   lda #1
   sta (_arr),y
   iny
   jne _lbl14
   inc k+1
_lbl14:
   lda k+1
   cmp #31
   jcc _lbl2
   jne _lbl15
   tya
   cpy #255
   jcc _lbl2
_lbl15:
   sty k
;(21) RTCLOCK = 0

   lda #0
   sta RTCLOCK
   lda #2
   sta _s1__i
_lbl7:
;(23) for i:2..SQRT_COUNT where sieve(i) = 1

   ldx _s1__i
   lda sieve,x
   sta _5
   cmp #1
   jne _lbl3
;(24) 		j = i * 2

   lda _s1__i
   asl
   sta j
   lda #0
   rol
   sta j+1
;(28) 

   ldy j+0
   jmp _lbl4
_lbl6:
;(26) 			sieve(j) = 0

   lda #<sieve
   sta _arr
   lda #>sieve
   clc
   adc j+1
   sta _arr+1
   lda #0
   sta (_arr),y
;(27) 			j = j + i

   tya
   clc
   adc _s1__i
   tay
   jcc _lbl16
   inc j+1
_lbl16:
_lbl4:
;(25) 		while j<=8191

   lda j+1
   cmp #31
   jeq _lbl17
   jcs _lbl5
_lbl17:
   tya
   cpy #255
   jeq _loc2
   jcs _lbl5
_loc2:
   jmp _lbl6
_lbl5:
   sty j
_lbl3:
   inc _s1__i
   lda _s1__i
   cmp #92
   jne _lbl7
;(29) time = RTCLOCK

   lda RTCLOCK
   sta time
;(30)  

   jsr _std_print_out
   dta b(11),c'Time used: '
   dta b(129),a(time)
   dta b(6),c' ticks'
;(31) "Time used: [time] ticks"

   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(39),c'Press Q to quit, any other key for list'
;(32) "Press Q to quit, any other key for list"

   dta b(128)
   dta b(0)
;(34) CH = none

   lda #key__none
   sta CH
;(36) 

   jmp _lbl8
_lbl10:
_lbl8:
;(35) until CH <> none

   lda CH
   cmp #key__none
   jne _lbl9
   jmp _lbl10
_lbl9:
;(39) 	

   lda #0
   sta k
   sta k+1
   jmp _lbl11
_lbl13:
   lda #<sieve
   sta _arr
   lda #>sieve
   clc
   adc k+1
   sta _arr+1
   ldy k+0
   lda (_arr),y
   sta _6
   cmp #1
   jne _lbl11
   jsr _std_print_out
   dta b(130),a(k)
;(38) 	 "[k]"

   dta b(128)
   dta b(0)
_lbl11:
;(37) for k where sieve(k) = 1 until CH = Q

   lda CH
   cmp #key__Q
   jeq _lbl12
   inc k+0
   jne _lbl18
   inc k+1
_lbl18:
   lda k+1
   cmp #31
   jcc _lbl13
   jne _lbl19
   lda k
   cmp #255
   jcc _lbl13
_lbl19:
_lbl12:
;(40) CH = none

;(41) 
   jmp *
   icl 'atari.asm'
sieve:
   .ds 8192
