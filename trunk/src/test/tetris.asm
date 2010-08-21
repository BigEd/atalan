_arr equ 128
timer equ 20
RANDOM equ 53770
CHBAS equ 756
button__state__pressed equ 0
STRIG equ 644
stick__state__left equ 11
stick__state__right equ 7
stick__state__down equ 13
STICK equ 632
COLOR0 equ 708
SDLSTL equ 560
s equ 32768
LM equ 15
RM equ 26
TM equ 1
BM equ 22
NEXT_LEFT equ 29
NEXT_TOP equ 5
WALL_CHR equ 69
RWALL_CHR equ 70
x equ 132
y equ 133
ti equ 134
c equ 135
nx equ 136
ny equ 137
steps equ 138
action__delay equ 139
collision equ 140
j equ 141
k equ 142
down__lock equ 143
tile equ 144
next__tile equ 145
ntile equ 146
score equ 147
draw__tile__tile equ 149
draw__tile__x equ 150
draw__tile__y equ 151
_s1___7 equ 152
_s1___8 equ 153
erase__tile__tile equ 154
erase__tile__x equ 155
erase__tile__y equ 156
_s3___10 equ 157
_s3___11 equ 158
test__collision__tile equ 159
test__collision__x equ 160
test__collision__y equ 161
test__collision__coll equ 162
_s5___14 equ 163
_s5___15 equ 164
_s6__y1 equ 165
_s8__y1 equ 166
_s9__x equ 167
_20 equ 168
_23 equ 169
_24 equ 170
_25 equ 171
_26 equ 172
_27 equ 173
_28 equ 174
_29 equ 175
_30 equ 176
_31 equ 177
_32 equ 178
_33 equ 179
   org $2e0
   dta a($2000)
   org $2000
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
   lda #0
   sta COLOR0+2
   sta COLOR0+5
   lda #>tetris__font
   sta CHBAS
start__game:
   ldy #0
   lda s_lo,y
   clc
   adc #0
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(13),c'ATALAN TETRIS'
   dta b(0)
   ldy #3
   lda s_lo,y
   clc
   adc #NEXT_LEFT
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(4),c'NEXT'
   dta b(0)
   ldy #1
   lda s_lo,y
   clc
   adc #NEXT_LEFT
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(13),c'SCORE: 0     '
   dta b(0)
   lda #1
   sta _s6__y1
_lbl20:
   lda #16
   sta x
_lbl19:
   ldy _s6__y1
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda #0
   sta (_arr),y
   inc x
   lda x
   cmp #26
   jne _lbl19
   inc _s6__y1
   lda _s6__y1
   cmp #21
   jne _lbl20
   lda #TM
   sta _s8__y1
_lbl22:
   ldy _s8__y1
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy #LM
   lda #WALL_CHR
   sta (_arr),y
   ldy _s8__y1
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy #RM
   lda #RWALL_CHR
   sta (_arr),y
   inc _s8__y1
   lda _s8__y1
   cmp #23
   jne _lbl22
   lda #LM
   sta _s9__x
_lbl24:
   ldy #BM
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy _s9__x
   lda #WALL_CHR
   sta (_arr),y
   inc _s9__x
   lda _s9__x
   cmp #27
   jne _lbl24
   lda #0
   sta action__delay
   sta down__lock
   lda #19
   sta x
   lda #1
   sta y
   lda RANDOM
   and #7
   sta tile
   lda RANDOM
   and #7
   sta next__tile
   lda #0
   sta score
   sta score+1
   lda next__tile
   sta draw__tile__tile
   lda #NEXT_LEFT
   sta draw__tile__x
   lda #NEXT_TOP
   sta draw__tile__y
   jsr draw__tile
loop:
   lda #20
   sta steps
   lda STICK
   sta _24
   cmp #stick__state__down
   jne _lbl25
   lda down__lock
   cmp #0
   jne _lbl26
   lda #1
   sta steps
_lbl26:
_lbl25:
   jmp _lbl27
_lbl43:
   lda tile
   sta draw__tile__tile
   lda x
   sta draw__tile__x
   lda y
   sta draw__tile__y
   jsr draw__tile
   lda x
   sta nx
   lda tile
   sta ntile
   lda STICK
   sta _25
   cmp #stick__state__down
   jeq _lbl29
   lda #0
   sta down__lock
_lbl29:
   lda action__delay
   cmp #0
   jcc _lbl30
   jeq _lbl30
   dec action__delay
   jmp _lbl31
_lbl30:
   lda #6
   sta action__delay
   lda STICK
   sta _26
   cmp #stick__state__right
   jne _lbl32
   inc nx
   jmp _lbl33
_lbl32:
   lda STICK
   sta _27
   cmp #stick__state__left
   jne _lbl34
   dec nx
   jmp _lbl33
_lbl34:
   lda STRIG
   sta _28
   cmp #button__state__pressed
   jne _lbl35
   ldx tile
   lda rotation,x
   sta ntile
   jmp _lbl33
_lbl35:
   lda #0
   sta action__delay
_lbl33:
_lbl31:
   lda #0
   sta timer
   jmp _lbl36
_lbl38:
_lbl36:
   lda timer
   cmp #1
   jcs _lbl37
   jmp _lbl38
_lbl37:
   lda tile
   sta erase__tile__tile
   lda x
   sta erase__tile__x
   lda y
   sta erase__tile__y
   jsr erase__tile
   lda ntile
   sta test__collision__tile
   lda nx
   sta test__collision__x
   lda y
   sta test__collision__y
   jsr test__collision
   lda test__collision__coll
   cmp #0
   jne _lbl39
   lda nx
   sta x
   lda ntile
   sta tile
_lbl39:
   dec steps
   lda STICK
   sta _29
   cmp #stick__state__down
   jne _lbl40
   lda down__lock
   cmp #0
   jne _lbl41
   lda steps
   cmp #5
   jcc _lbl42
   jeq _lbl42
   lda #0
   sta steps
_lbl42:
_lbl41:
_lbl40:
_lbl27:
   lda steps
   cmp #0
   jcc _lbl28
   jeq _lbl28
   jmp _lbl43
_lbl28:
   lda tile
   sta test__collision__tile
   lda x
   sta test__collision__x
   lda y
   clc
   adc #1
   sta _20
   sta test__collision__y
   jsr test__collision
   lda test__collision__coll
   cmp #0
   jne _lbl44
   inc y
   jmp _lbl45
_lbl44:
   lda tile
   sta draw__tile__tile
   lda x
   sta draw__tile__x
   lda y
   sta draw__tile__y
   jsr draw__tile
   lda #21
   sta y
   sta ny
   jmp _lbl46
_lbl59:
   lda #0
   sta collision
   lda #16
   sta x
   ldx #0
_lbl49:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda (_arr),y
   sta _30
   cmp #0
   jne _lbl48
   lda #1
   tax
_lbl48:
   inc x
   lda x
   cmp #26
   jne _lbl49
   stx collision
   txa
   cpx #0
   jeq _lbl50
   lda #16
   sta x
_lbl52:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda (_arr),y
   tax
   ldy ny
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   txa
   sta (_arr),y
   inc x
   lda x
   cmp #26
   jne _lbl52
   stx _31
   dec ny
   dec y
   jmp _lbl53
_lbl50:
   inc score+0
   jne _lbl65
   inc score+1
_lbl65:
   dec y
   lda #16
   sta x
_lbl58:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda (_arr),y
   tax
   cmp #64
   jne _lbl55
   lda #67
   tax
_lbl55:
   txa
   cpx #65
   jne _lbl56
   lda #66
   tax
_lbl56:
   txa
   cpx #68
   jne _lbl57
   lda #67
   tax
_lbl57:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   txa
   sta (_arr),y
   inc x
   lda x
   cmp #26
   jne _lbl58
   stx c
_lbl53:
_lbl46:
   lda y
   cmp #1
   jcc _lbl47
   jmp _lbl59
_lbl47:
   ldy #1
   lda s_lo,y
   clc
   adc #29
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(7),c'SCORE: '
   dta b(130),a(score)
   dta b(0)
   lda STICK
   sta _32
   cmp #stick__state__down
   jne _lbl60
   lda #1
   sta down__lock
_lbl60:
   lda next__tile
   sta tile
   sta erase__tile__tile
   lda #NEXT_LEFT
   sta erase__tile__x
   lda #NEXT_TOP
   sta erase__tile__y
   jsr erase__tile
   lda RANDOM
   and #7
   sta next__tile
   sta draw__tile__tile
   lda #NEXT_LEFT
   sta draw__tile__x
   lda #NEXT_TOP
   sta draw__tile__y
   jsr draw__tile
   lda #19
   sta x
   lda #1
   sta y
   lda tile
   sta test__collision__tile
   lda #19
   sta test__collision__x
   lda #1
   sta test__collision__y
   jsr test__collision
   lda test__collision__coll
   cmp #0
   jeq _lbl61
   jmp game__over
_lbl61:
_lbl45:
   jmp loop
game__over:
   ldy #10
   lda s_lo,y
   clc
   adc #19
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(4),c'GAME'
   dta b(0)
   ldy #11
   lda s_lo,y
   clc
   adc #19
   sta _arr
   lda s_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(4),c'OVER'
   dta b(0)
   ldx _33
   jmp _lbl62
_lbl64:
_lbl62:
   lda STRIG
   tax
   cmp #button__state__pressed
   jeq _lbl63
   jmp _lbl64
_lbl63:
   stx _33
   jmp start__game
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
   .align 1024
tetris__font:
   ins 'tetris.fnt'
tiles:
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(64)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(68)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(68)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(68)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(68)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(67)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(68)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(67)
   dta b(67)
   dta b(0)
   dta b(66)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(65)
   dta b(67)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(68)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(66)
   dta b(67)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
rotation:
   dta b(7)
   dta b(1)
   dta b(10)
   dta b(11)
   dta b(14)
   dta b(17)
   dta b(18)
   dta b(8)
   dta b(9)
   dta b(0)
   dta b(2)
   dta b(12)
   dta b(13)
   dta b(3)
   dta b(15)
   dta b(16)
   dta b(4)
   dta b(5)
   dta b(6)
s_lo:  :24 dta l(s + #*40)
s_hi:  :24 dta h(s + #*40)
tiles_lo:  :19 dta l(tiles + #*16)
tiles_hi:  :19 dta h(tiles + #*16)
draw__tile .proc
   lda #0
   sta ti
   lda #0
   sta j
_lbl5:
   lda #0
   sta k
_lbl4:
   ldy draw__tile__tile
   lda tiles_lo,y
   sta _arr
   lda tiles_hi,y
   sta _arr+1
   ldy ti
   lda (_arr),y
   sta c
   lda c
   cmp #0
   jeq _lbl3
   lda draw__tile__x
   clc
   adc k
   sta _s1___7
   lda draw__tile__y
   clc
   adc j
   sta _s1___8
   ldy _s1___8
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy _s1___7
   lda c
   sta (_arr),y
_lbl3:
   inc ti
   inc k
   lda k
   cmp #4
   jne _lbl4
   inc j
   lda j
   cmp #4
   jne _lbl5
   rts
.endp
erase__tile .proc
   lda #0
   sta ti
   lda #0
   sta j
_lbl10:
   lda #0
   sta k
_lbl9:
   ldy erase__tile__tile
   lda tiles_lo,y
   sta _arr
   lda tiles_hi,y
   sta _arr+1
   ldy ti
   lda (_arr),y
   sta c
   lda c
   cmp #0
   jeq _lbl8
   lda erase__tile__x
   clc
   adc k
   sta _s3___10
   lda erase__tile__y
   clc
   adc j
   sta _s3___11
   ldy _s3___11
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy _s3___10
   lda #0
   sta (_arr),y
_lbl8:
   inc ti
   inc k
   lda k
   cmp #4
   jne _lbl9
   inc j
   lda j
   cmp #4
   jne _lbl10
   rts
.endp
test__collision .proc
   lda #0
   sta test__collision__coll
   lda #0
   sta ti
   lda #0
   sta j
_lbl16:
   lda #0
   sta k
_lbl15:
   ldy test__collision__tile
   lda tiles_lo,y
   sta _arr
   lda tiles_hi,y
   sta _arr+1
   ldy ti
   lda (_arr),y
   sta c
   inc ti
   lda c
   cmp #0
   jeq _lbl13
   lda test__collision__x
   clc
   adc k
   sta _s5___14
   lda test__collision__y
   clc
   adc j
   sta _s5___15
   ldy _s5___15
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy _s5___14
   lda (_arr),y
   sta _23
   lda _23
   cmp #0
   jeq _lbl14
   lda #1
   sta test__collision__coll
_lbl14:
_lbl13:
   inc k
   lda k
   cmp #4
   jne _lbl15
   inc j
   lda j
   cmp #4
   jne _lbl16
   rts
.endp
