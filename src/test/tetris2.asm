inc___2000 equ 128
_arr equ 129
SDLSTL equ 560
timer equ 20
RANDOM equ 53770
CHBAS equ 756
LMARGN equ 82
RMARGN equ 83
VCOUNT equ 54283
WSYNC equ 54282
button__state__pressed equ 0
button__state__not__pressed equ 1
STRIG equ 644
stick__state__left equ 11
stick__state__right equ 7
stick__state__down equ 13
STICK equ 632
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
LM equ 15
RM equ 26
BM equ 22
NEXT_LEFT equ 29
NEXT_TOP equ 3
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
draw__tile__tile equ 147
draw__tile__x equ 148
draw__tile__y equ 149
draw__tile___1 equ 150
erase__tile__tile equ 151
erase__tile__x equ 152
erase__tile__y equ 153
erase__tile___2002 equ 154
erase__tile___3 equ 155
erase__tile___4 equ 156
test__collision__tile equ 157
test__collision__x equ 158
test__collision__y equ 159
test__collision___5 equ 160
_99 equ 161
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
   lda #1
   sta y
_lbl18:
   lda #16
   sta x
_lbl20:
   ldy y
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
   jne _lbl20
   inc y
   lda y
   cmp #23
   jne _lbl18
   lda #1
   sta y
_lbl22:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy #LM
   lda #WALL_CHR
   sta (_arr),y
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy #RM
   lda #RWALL_CHR
   sta (_arr),y
   inc y
   lda y
   cmp #23
   jne _lbl22
   lda #16
   sta x
_lbl24:
   ldy #BM
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda #WALL_CHR
   sta (_arr),y
   inc x
   lda x
   cmp #26
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
   sta _99
   lda _99
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
_lbl28:
   lda tile
   sta draw__tile__tile
   lda x
   sta draw__tile__x
   lda y
   sta draw__tile__y
   jsr draw__tile
   lda x
   sta nx
   lda y
   sta ny
   lda tile
   sta ntile
   lda STICK
   sta _99
   lda _99
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
   sta _99
   lda _99
   cmp #stick__state__right
   jne _lbl32
   inc nx
   jmp _lbl33
_lbl32:
   lda STICK
   sta _99
   lda _99
   cmp #stick__state__left
   jne _lbl34
   dec nx
   jmp _lbl33
_lbl34:
   lda STRIG
   sta _99
   lda _99
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
_lbl37:
_lbl36:
   lda timer
   cmp #1
   jcc _lbl37
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
   lda collision
   cmp #0
   jne _lbl38
   lda nx
   sta x
   lda ntile
   sta tile
_lbl38:
   dec steps
   lda STICK
   sta _99
   lda _99
   cmp #stick__state__down
   jne _lbl39
   lda down__lock
   cmp #0
   jne _lbl40
   lda steps
   cmp #5
   jcc _lbl41
   jeq _lbl41
   lda #0
   sta steps
_lbl41:
_lbl40:
_lbl39:
_lbl27:
   lda steps
   cmp #0
   jeq _lbl61
   jcs _lbl28
_lbl61:
   lda tile
   sta test__collision__tile
   lda x
   sta test__collision__x
   lda y
   clc
   adc #1
   sta erase__tile___4
   sta test__collision__y
   jsr test__collision
   lda collision
   cmp #0
   jne _lbl42
   inc y
   jmp _lbl43
_lbl42:
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
   jmp _lbl44
_lbl45:
   lda #0
   sta collision
   lda #16
   sta x
_lbl47:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda (_arr),y
   tax
   txa
   cpx #0
   jne _lbl46
   lda #1
   sta collision
_lbl46:
   inc x
   lda x
   cmp #26
   jne _lbl47
   stx _99
   lda collision
   cmp #0
   jeq _lbl48
   lda #16
   sta x
_lbl50:
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
   jne _lbl50
   stx _99
   dec ny
   dec y
   jmp _lbl51
_lbl48:
   dec y
   lda #16
   sta x
_lbl53:
   ldy y
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy x
   lda (_arr),y
   tax
   txa
   cpx #64
   jne _lbl54
   lda #67
   tax
_lbl54:
   txa
   cpx #65
   jne _lbl55
   lda #66
   tax
_lbl55:
   txa
   cpx #68
   jne _lbl56
   lda #67
   tax
_lbl56:
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
   jne _lbl53
   stx c
_lbl51:
_lbl44:
   lda y
   cmp #1
   jcs _lbl45
   lda STICK
   sta _99
   lda _99
   cmp #stick__state__down
   jne _lbl57
   lda #1
   sta down__lock
_lbl57:
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
   lda collision
   cmp #0
   jeq _lbl58
   jmp game__over
_lbl58:
_lbl43:
   jmp loop
game__over:
   jmp _lbl59
_lbl60:
_lbl59:
   lda STRIG
   tax
   txa
   cpx #button__state__not__pressed
   jeq _lbl60
   stx _99
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
s:
   .ds 40*24
draw__tile .proc
   lda #0
   sta ti
   lda #0
   sta j
_lbl2:
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
   jeq _lbl5
   lda draw__tile__x
   clc
   adc k
   sta inc___2000
   lda draw__tile__y
   clc
   adc j
   sta draw__tile___1
   ldy draw__tile___1
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy inc___2000
   lda c
   sta (_arr),y
_lbl5:
   inc ti
   inc k
   lda k
   cmp #4
   jne _lbl4
   inc j
   lda j
   cmp #4
   jne _lbl2
   rts
.endp
erase__tile .proc
   lda #0
   sta ti
   lda #0
   sta j
_lbl7:
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
   jeq _lbl10
   lda erase__tile__x
   clc
   adc k
   sta erase__tile___2002
   lda erase__tile__y
   clc
   adc j
   sta erase__tile___3
   ldy erase__tile___3
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy erase__tile___2002
   lda #0
   sta (_arr),y
_lbl10:
   inc ti
   inc k
   lda k
   cmp #4
   jne _lbl9
   inc j
   lda j
   cmp #4
   jne _lbl7
   rts
.endp
test__collision .proc
   lda #0
   sta collision
   lda #0
   sta ti
   lda #0
   sta j
_lbl12:
   lda #0
   sta k
_lbl14:
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
   jeq _lbl15
   lda test__collision__x
   clc
   adc k
   sta erase__tile___4
   lda test__collision__y
   clc
   adc j
   sta test__collision___5
   ldy test__collision___5
   lda s_lo,y
   sta _arr
   lda s_hi,y
   sta _arr+1
   ldy erase__tile___4
   lda (_arr),y
   sta _99
   lda _99
   cmp #0
   jeq _lbl16
   lda #1
   sta collision
_lbl16:
_lbl15:
   inc k
   lda k
   cmp #4
   jne _lbl14
   inc j
   lda j
   cmp #4
   jne _lbl12
   rts
.endp
