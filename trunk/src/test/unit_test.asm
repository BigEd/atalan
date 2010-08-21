_TEMPW1 equ 50
_TEMPW2 equ 52
COLOR0 equ 708
x equ 131
y equ 132
z equ 133
_1000 equ 134
_1001 equ 135
p equ 136
o equ 138
r equ 140
   org $2e0
   dta a($2000)
   org $2000
   jsr _std_print_out
   dta b(12),c'Test started'
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(25),c'Byte unsigned arithmetics'
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(3),c'  ='
   dta b(0)
   lda #10
   sta x
   lda #3
   sta y
   lda #10
   cmp #10
   jeq _lbl1
   jmp fail
_lbl1:
   jsr _std_print_out
   dta b(2),c',+'
   dta b(0)
   lda x
   clc
   adc y
   sta z
   cmp #13
   jeq _lbl2
   jmp fail
_lbl2:
   lda z
   clc
   adc #2
   sta z
   cmp #15
   jeq _lbl3
   jmp fail
_lbl3:
   lda x
   clc
   adc #3
   sta z
   cmp #13
   jeq _lbl4
   jmp fail
_lbl4:
   lda x
   clc
   adc #4
   sta z
   cmp #14
   jeq _lbl5
   jmp fail
_lbl5:
   jsr _std_print_out
   dta b(2),c',-'
   dta b(0)
   lda x
   sec
   sbc y
   sta z
   cmp #7
   jeq _lbl6
   jmp fail
_lbl6:
   lda x
   sec
   sbc #4
   sta z
   cmp #6
   jeq _lbl7
   jmp fail
_lbl7:
   lda #15
   sec
   sbc x
   sta z
   cmp #5
   jeq _lbl8
   jmp fail
_lbl8:
   lda z
   sec
   sbc #2
   sta z
   cmp #3
   jeq _lbl9
   jmp fail
_lbl9:
   jsr _std_print_out
   dta b(2),c',*'
   dta b(0)
   lda x
   ldx y
   jsr _sys_mul8
   lda _TEMPW2
   sta z
   cmp #30
   jeq _lbl10
   jmp fail
_lbl10:
   lda x
   asl
   sta z
   cmp #20
   jeq _lbl11
   jmp fail
_lbl11:
   lda x
   ldx #3
   jsr _sys_mul8
   lda _TEMPW2
   sta z
   cmp #30
   jeq _lbl12
   jmp fail
_lbl12:
   jsr _std_print_out
   dta b(2),c',/'
   dta b(0)
   lda x
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #5
   jsr _sys_div8
   sta z
   lda z
   cmp #2
   jeq _lbl13
   jmp fail
_lbl13:
   lda x
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda y
   jsr _sys_div8
   sta z
   lda z
   cmp #3
   jeq _lbl14
   jmp fail
_lbl14:
   lda #99
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda y
   jsr _sys_div8
   sta z
   lda z
   cmp #33
   jeq _lbl15
   jmp fail
_lbl15:
   jsr _std_print_out
   dta b(4),c',mod'
   dta b(0)
   lda x
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #4
   jsr _sys_div8
   stx z
   txa
   cpx #2
   jeq _lbl16
   jmp fail
_lbl16:
   lda x
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda y
   jsr _sys_div8
   stx z
   txa
   cpx #1
   jeq _lbl17
   jmp fail
_lbl17:
   lda #100
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda y
   jsr _sys_div8
   stx z
   txa
   cpx #1
   jeq _lbl18
   jmp fail
_lbl18:
   lda #99
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda y
   jsr _sys_div8
   stx z
   txa
   cpx #0
   jeq _lbl19
   jmp fail
_lbl19:
   jsr _std_print_out
   dta b(4),c',and'
   dta b(0)
   lda x
   and y
   sta z
   cmp #2
   jeq _lbl20
   jmp fail
_lbl20:
   lda x
   and #8
   sta z
   cmp #8
   jeq _lbl21
   jmp fail
_lbl21:
   lda #2
   and x
   sta z
   cmp #2
   jeq _lbl22
   jmp fail
_lbl22:
   jsr _std_print_out
   dta b(3),c',or'
   dta b(0)
   lda x
   ora y
   sta z
   cmp #11
   jeq _lbl23
   jmp fail
_lbl23:
   lda x
   ora #131
   sta z
   cmp #139
   jeq _lbl24
   jmp fail
_lbl24:
   lda #65
   ora x
   sta z
   cmp #75
   jeq _lbl25
   jmp fail
_lbl25:
   jsr _std_print_out
   dta b(4),c',xor'
   dta b(0)
   lda x
   eor y
   sta z
   cmp #9
   jeq _lbl26
   jmp fail
_lbl26:
   lda x
   eor #131
   sta z
   cmp #137
   jeq _lbl27
   jmp fail
_lbl27:
   lda #67
   eor x
   sta z
   cmp #73
   jeq _lbl28
   jmp fail
_lbl28:
   jsr _std_print_out
   dta b(4),c',not'
   dta b(128)
   dta b(0)
   lda x
   eor #$FF
   sta z
   cmp #245
   jeq _lbl29
   jmp fail
_lbl29:
   jsr _std_print_out
   dta b(19),c'Operator precedence'
   dta b(128)
   dta b(0)
   lda #3
   sta x
   lda #4
   sta y
   lda #6
   sta _1000
   clc
   lda #10
   sta z
   cmp #10
   jeq _lbl30
   jmp fail
_lbl30:
   lda x
   asl
   sta _1000
   lda y
   clc
   adc _1000
   sta z
   cmp #10
   jeq _lbl31
   jmp fail
_lbl31:
   lda x
   asl
   sta _1000
   lda y
   asl
   sta _1001
   lda _1000
   clc
   adc _1001
   sta z
   cmp #14
   jeq _lbl32
   jmp fail
_lbl32:
   jsr _std_print_out
   dta b(11),c'Parentheses'
   dta b(128)
   dta b(0)
   lda x
   clc
   adc y
   sta _1000
   asl
   sta z
   cmp #14
   jeq _lbl33
   jmp fail
_lbl33:
   jsr _std_print_out
   dta b(27),c'2-byte unsigned arithmetics'
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(3),c'  ='
   dta b(0)
   lda #16
   sta p
   lda #39
   sta p+1
   lda #43
   sta o
   lda #0
   sta o+1
   lda #72
   sta x
   lda #16
   cmp #16
   jne _lbl84
   lda #39
   cmp #39
   jeq _lbl34
_lbl84:
   jmp fail
_lbl34:
   lda o
   cmp #43
   jne _lbl85
   lda o+1
   cmp #0
   jeq _lbl35
_lbl85:
   jmp fail
_lbl35:
   lda p
   sta o
   lda p+1
   sta o+1
   lda p
   cmp #16
   jne _lbl86
   lda p+1
   cmp #39
   jeq _lbl36
_lbl86:
   jmp fail
_lbl36:
   lda x
   sta o
   lda #0
   sta o+1
   lda x
   cmp #72
   jne _lbl87
   lda #0
   cmp #0
   jeq _lbl37
_lbl87:
   jmp fail
_lbl37:
   lda #223
   sta o
   lda #2
   sta o+1
   lda #223
   cmp #223
   jne _lbl88
   lda #2
   cmp #2
   jeq _lbl38
_lbl88:
   jmp fail
_lbl38:
   jsr _std_print_out
   dta b(2),c',+'
   dta b(0)
   lda p
   clc
   adc o+0
   sta r
   lda p+1
   adc o+1
   sta r+1
   lda r
   cmp #239
   jne _lbl89
   lda r+1
   cmp #41
   jeq _lbl39
_lbl89:
   jmp fail
_lbl39:
   lda p
   clc
   adc #163
   sta r
   lda p+1
   adc #20
   sta r+1
   lda r
   cmp #179
   jne _lbl90
   lda r+1
   cmp #59
   jeq _lbl40
_lbl90:
   jmp fail
_lbl40:
   lda p
   clc
   adc #140
   sta r
   lda p+1
   adc #24
   sta r+1
   lda r
   cmp #156
   jne _lbl91
   lda r+1
   cmp #63
   jeq _lbl41
_lbl91:
   jmp fail
_lbl41:
   jsr _std_print_out
   dta b(2),c',-'
   dta b(0)
   lda p
   sec
   sbc o+0
   sta r
   lda p+1
   sbc o+1
   sta r+1
   lda r
   cmp #49
   jne _lbl92
   lda r+1
   cmp #36
   jeq _lbl42
_lbl92:
   jmp fail
_lbl42:
   lda p
   sec
   sbc #136
   sta r
   lda p+1
   sbc #19
   sta r+1
   lda r
   cmp #136
   jne _lbl93
   lda r+1
   cmp #19
   jeq _lbl43
_lbl93:
   jmp fail
_lbl43:
   lda #171
   sec
   sbc p+0
   sta r
   lda #97
   sbc p+1
   sta r+1
   lda r
   cmp #155
   jne _lbl94
   lda r+1
   cmp #58
   jeq _lbl44
_lbl94:
   jmp fail
_lbl44:
   jsr _std_print_out
   dta b(2),c',*'
   dta b(128)
   dta b(0)
   lda #250
   sta x
   cmp #250
   jeq _lbl45
   jmp fail
_lbl45:
   lda x
   asl
   sta r
   lda #0
   rol
   sta r+1
   lda r
   cmp #244
   jne _lbl95
   lda r+1
   cmp #1
   jeq _lbl46
_lbl95:
   jmp fail
_lbl46:
   lda p
   asl
   sta r
   lda p+1
   rol
   sta r+1
   lda r
   cmp #32
   jne _lbl96
   lda r+1
   cmp #78
   jeq _lbl47
_lbl96:
   jmp fail
_lbl47:
   lda #100
   sta x
   lda #5
   sta y
   lda #100
   ldx #5
   jsr _sys_mul8
   lda _TEMPW2
   sta r
   lda _TEMPW2+1
   sta r+1
   lda _TEMPW2
   cmp #244
   jne _lbl97
   lda _TEMPW2+1
   cmp #1
   jeq _lbl48
_lbl97:
   jmp fail
_lbl48:
   lda x
   ldx #7
   jsr _sys_mul8
   lda _TEMPW2
   sta r
   lda _TEMPW2+1
   sta r+1
   lda _TEMPW2
   cmp #188
   jne _lbl98
   lda _TEMPW2+1
   cmp #2
   jeq _lbl49
_lbl98:
   jmp fail
_lbl49:
   lda x
   ldx #13
   jsr _sys_mul8
   lda _TEMPW2
   sta r
   lda _TEMPW2+1
   sta r+1
   lda _TEMPW2
   cmp #20
   jne _lbl99
   lda _TEMPW2+1
   cmp #5
   jeq _lbl50
_lbl99:
   jmp fail
_lbl50:
   jsr _std_print_out
   dta b(6),c'Blocks'
   dta b(128)
   dta b(0)
   lda #1
   sta x
   lda #5
   sta z
   lda #1
   cmp #1
   jne _lbl51
   lda #5
_lbl51:
   lda z
   cmp #5
   jne _lbl52
   lda #6
   sta z
   lda #2
   sta x
_lbl52:
   lda x
   cmp #2
   jne _lbl53
   lda #7
   sta z
_lbl53:
   lda z
   cmp #7
   jeq _lbl54
   jmp fail
_lbl54:
   lda z
   cmp #7
   jne _lbl55
   lda x
   cmp #2
   jne _lbl56
   lda #3
   sta x
_lbl56:
_lbl55:
   lda x
   cmp #3
   jeq _lbl57
   jmp fail
_lbl57:
   jsr _std_print_out
   dta b(12),c'If Then Else'
   dta b(128)
   dta b(0)
   lda #1
   sta x
   cmp #1
   jne _lbl58
   lda #2
   sta z
   jmp _lbl59
_lbl58:
   lda #3
   sta z
_lbl59:
   lda z
   cmp #2
   jeq _lbl60
   jmp fail
_lbl60:
   lda #2
   sta x
   cmp #1
   jne _lbl61
   sta z
   jmp _lbl62
_lbl61:
   lda #3
   sta z
_lbl62:
   lda z
   cmp #3
   jeq _lbl63
   jmp fail
_lbl63:
   lda #0
   sta z
   lda #1
   sta x
   cmp #1
   jne _lbl64
   lda #2
   sta z
   jmp _lbl65
_lbl64:
   lda x
   cmp #2
   jne _lbl66
   lda #3
   sta z
   jmp _lbl65
_lbl66:
   lda x
   cmp #3
   jne _lbl67
   lda #4
   sta z
   jmp _lbl65
_lbl67:
   lda #5
   sta z
_lbl65:
   lda z
   cmp #2
   jeq _lbl68
   jmp fail
_lbl68:
   lda #2
   sta x
   cmp #1
   jne _lbl69
   sta z
   jmp _lbl70
_lbl69:
   lda x
   cmp #2
   jne _lbl71
   lda #3
   sta z
   jmp _lbl70
_lbl71:
   lda x
   cmp #3
   jne _lbl72
   lda #4
   sta z
   jmp _lbl70
_lbl72:
   lda #5
   sta z
_lbl70:
   lda z
   cmp #3
   jeq _lbl73
   jmp fail
_lbl73:
   lda #3
   sta x
   cmp #1
   jne _lbl74
   lda #2
   sta z
   jmp _lbl75
_lbl74:
   lda x
   cmp #2
   jne _lbl76
   lda #3
   sta z
   jmp _lbl75
_lbl76:
   lda x
   cmp #3
   jne _lbl77
   lda #4
   sta z
   jmp _lbl75
_lbl77:
   lda #5
   sta z
_lbl75:
   lda z
   cmp #4
   jeq _lbl78
   jmp fail
_lbl78:
   lda #4
   sta x
   cmp #1
   jne _lbl79
   lda #2
   sta z
   jmp _lbl80
_lbl79:
   lda x
   cmp #2
   jne _lbl81
   lda #3
   sta z
   jmp _lbl80
_lbl81:
   lda x
   cmp #3
   jne _lbl82
   lda #4
   sta z
   jmp _lbl80
_lbl82:
   lda #5
   sta z
_lbl80:
   lda z
   cmp #5
   jeq _lbl83
   jmp fail
_lbl83:
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
