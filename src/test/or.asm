RANDOM equ 53770
COLOR0 equ 708
x equ 132
y equ 133
   org $2e0
   dta a($2000)
   org $2000
   jsr _std_print_out
   dta b(17),c'Logical operators'
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(5),c'   Or'
   dta b(0)
   lda #0
   sta x
   cmp #1
   jcs _lbl1
   jmp _lbl2
_lbl1:
   lda x
   cmp #38
   jcc _lbl3
   jeq _lbl3
   jmp _lbl2
_lbl3:
   lda x
   cmp #10
   jne _lbl4
_lbl2:
   lda #40
   sta x
_lbl4:
   lda x
   cmp #40
   jeq _lbl5
   jmp fail
_lbl5:
   lda #50
   sta x
   cmp #1
   jcs _lbl6
   jmp _lbl7
_lbl6:
   lda x
   cmp #38
   jcc _lbl8
   jeq _lbl8
   jmp _lbl7
_lbl8:
   lda x
   cmp #10
   jne _lbl9
_lbl7:
   lda #40
   sta x
_lbl9:
   lda x
   cmp #40
   jeq _lbl10
   jmp fail
_lbl10:
   lda #10
   sta x
   cmp #1
   jcs _lbl11
   jmp _lbl12
_lbl11:
   lda x
   cmp #38
   jcc _lbl13
   jeq _lbl13
   jmp _lbl12
_lbl13:
   lda x
   cmp #10
   jne _lbl14
_lbl12:
   lda #40
   sta x
_lbl14:
   lda x
   cmp #40
   jeq _lbl15
   jmp fail
_lbl15:
   lda #20
   sta x
   cmp #1
   jcs _lbl16
   jmp _lbl17
_lbl16:
   lda x
   cmp #38
   jcc _lbl18
   jeq _lbl18
   jmp _lbl17
_lbl18:
   lda x
   cmp #10
   jne _lbl19
_lbl17:
   lda #40
   sta x
_lbl19:
   lda x
   cmp #20
   jeq _lbl20
   jmp fail
_lbl20:
   jsr _std_print_out
   dta b(4),c',And'
   dta b(0)
   lda #5
   sta x
   lda #7
   sta y
   lda x
   cmp #5
   jne _lbl21
   lda y
   cmp #7
   jne _lbl21
   lda #100
   sta x
_lbl21:
   lda x
   cmp #100
   jeq _lbl22
   jmp fail
_lbl22:
   lda #5
   sta x
   lda #3
   sta y
   lda x
   cmp #5
   jne _lbl23
   lda y
   cmp #7
   jne _lbl23
   lda #100
   sta x
_lbl23:
   lda x
   cmp #5
   jeq _lbl24
   jmp fail
_lbl24:
   jsr _std_print_out
   dta b(4),c',mix'
   dta b(0)
   lda #5
   sta x
   lda #3
   sta y
   lda x
   cmp #5
   jne _lbl25
   jmp _lbl26
_lbl25:
   lda x
   cmp #6
   jne _lbl27
   lda y
   cmp #2
   jne _lbl27
_lbl26:
   lda #100
   sta x
_lbl27:
   lda x
   cmp #100
   jeq _lbl28
   jmp fail
_lbl28:
   lda #6
   sta x
   lda #2
   sta y
   lda x
   cmp #5
   jne _lbl29
   jmp _lbl30
_lbl29:
   lda x
   cmp #6
   jne _lbl31
   lda y
   cmp #2
   jne _lbl31
_lbl30:
   lda #100
   sta x
_lbl31:
   lda x
   cmp #100
   jeq _lbl32
   jmp fail
_lbl32:
   lda #6
   sta x
   lda #3
   sta y
   lda x
   cmp #5
   jne _lbl33
   jmp _lbl34
_lbl33:
   lda x
   cmp #6
   jne _lbl35
   lda y
   cmp #2
   jne _lbl35
_lbl34:
   lda #100
   sta x
_lbl35:
   lda x
   cmp #100
   jne _lbl36
   jmp fail
_lbl36:
   lda #6
   sta x
   lda #3
   sta y
   lda x
   cmp #5
   jne _lbl37
   jmp _lbl38
_lbl37:
   lda x
   cmp #6
   jne _lbl39
   lda y
   cmp #2
   jne _lbl39
_lbl38:
   lda #100
   sta x
_lbl39:
   lda x
   cmp #100
   jne _lbl40
   jmp fail
_lbl40:
   jsr _std_print_out
   dta b(6),c',While'
   dta b(0)
   lda #0
   sta x
   jmp _lbl41
_lbl45:
   lda RANDOM
   sta x
_lbl41:
   lda x
   cmp #38
   jcs _lbl42
   jmp _lbl43
_lbl42:
   lda x
   cmp #50
   jcc _lbl44
   jeq _lbl44
_lbl43:
   jmp _lbl45
_lbl44:
   lda x
   cmp #38
   jcs _lbl46
   jmp _lbl47
_lbl46:
   lda x
   cmp #50
   jcc _lbl48
   jeq _lbl48
_lbl47:
   jmp fail
_lbl48:
   jsr _std_print_out
   dta b(6),c',Until'
   dta b(128)
   dta b(0)
   lda #3
   sta x
   jmp _lbl49
_lbl51:
   lda RANDOM
   sta x
_lbl49:
   lda x
   cmp #10
   jeq _lbl55
   jcs _lbl50
_lbl55:
   lda x
   cmp #20
   jcc _lbl50
   jmp _lbl51
_lbl50:
   jsr _std_print_out
   dta b(129),a(x)
   dta b(128)
   dta b(0)
   lda x
   cmp #38
   jcs _lbl52
   jmp _lbl53
_lbl52:
   lda x
   cmp #50
   jcc _lbl54
   jeq _lbl54
_lbl53:
   jmp fail
_lbl54:
   jsr _std_print_out
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(10),c'*** OK ***'
   dta b(128)
   dta b(0)
   jmp done
fail:
   jsr _std_print_out
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(16),c'*** Failure! ***'
   dta b(128)
   dta b(0)
   lda #37
   sta COLOR0+4
done:
   jmp *
   icl 'atari.asm'
