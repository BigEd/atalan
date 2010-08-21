_arr equ 128
_TEMPW2 equ 52
timer equ 20
RANDOM equ 53770
CHBAS equ 756
VCOUNT equ 54283
button__state__pressed equ 0
button__state__not__pressed equ 1
STRIG equ 644
stick__state__left equ 11
stick__state__right equ 7
stick__state__up equ 14
stick__state__down equ 13
STICK equ 632
COLOR0 equ 708
SDLSTL equ 560
_scr equ 32768
root__HSx equ 22
root__HSy equ 10
root__SLength equ 7
root__SNAKE_HEAD_l equ 72
root__SNAKE_HEAD_r equ 69
root__SNAKE_HEAD_u equ 70
root__SNAKE_HEAD_d equ 71
root__SNAKE_BODY_nw equ 75
root__SNAKE_BODY_ws equ 77
root__SNAKE_BODY_se equ 78
root__SNAKE_BODY_en equ 76
root__SNAKE_BODY_v equ 79
root__SNAKE_BODY_h equ 80
root__SNAKE_TAIL_l equ 66
root__SNAKE_TAIL_r equ 68
root__SNAKE_TAIL_u equ 65
root__SNAKE_TAIL_d equ 67
root__FRUIT equ 16
root__WALL_CHR equ 73
root__RWALL_CHR equ 74
COL__BK equ 53274
RTCLOCK equ 20
Hx equ 130
Hy equ 131
Tx equ 132
Ty equ 133
HDir equ 134
Score equ 135
Level equ 137
LevDelay equ 138
_s0__yy equ 139
_s1__xx equ 140
drawSnakeBegin__x equ 141
drawSnakeBegin__y equ 142
drawSnakeBegin__len equ 143
drawSnakeBegin__xx equ 144
getNextHeadCoord__x equ 145
getNextHeadCoord__y equ 146
getNextTailCoord__ss equ 147
getNextTailCoord__x equ 148
getNextTailCoord__y equ 149
moveTail__ts equ 150
moveTail__xx equ 151
moveTail__yy equ 152
moveTail__ns equ 153
putNeck__dir1 equ 154
putNeck__neck1 equ 155
putNeck__dir2 equ 156
putNeck__neck2 equ 157
putNeck__body equ 158
eatFruit___12 equ 159
generateFruit__x equ 160
generateFruit___14 equ 161
generateFruit__y equ 162
generateFruit___16 equ 163
moveHead__gotFruit equ 164
moveHead__hs equ 165
moveHead__xx equ 166
moveHead__yy equ 167
moveHead__ns equ 168
moveHead__nh equ 169
drawSnakeAtRandomPosition__rl equ 170
drawSnakeAtRandomPosition___20 equ 171
drawSnakeAtRandomPosition__ry equ 172
drawSnakeAtRandomPosition___23 equ 173
drawSnakeAtRandomPosition__rx equ 174
drawSnakeAtRandomPosition___27 equ 175
drawSnakeAtRandomPosition___28 equ 176
_s8___30 equ 177
_s8___31 equ 178
changeDirection___33 equ 179
changeDirection___34 equ 180
changeDirection___35 equ 181
changeDirection___36 equ 182
_s9___37 equ 183
changeDirection__r equ 185
changeDirection__xx equ 186
changeDirection__yy equ 187
animateSnake__xx equ 188
animateSnake__yy equ 189
animateSnake__r equ 190
_s10__xx equ 191
_s11__yy equ 192
_s12__xx equ 193
grow equ 194
delayCntr equ 195
nDir equ 196
_s16__ss equ 197
_s18___45 equ 198
_47 equ 200
_48 equ 201
_49 equ 202
_50 equ 203
_51 equ 204
_52 equ 205
_53 equ 206
_54 equ 207
_55 equ 208
_56 equ 209
_57 equ 210
_58 equ 211
_59 equ 212
_60 equ 213
_61 equ 214
   org $2e0
   dta a($2000)
   org $2000
;(408) sdlstl = dl

   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
;(409) COLOR0(2) = 0

   lda #0
   sta COLOR0+2
;(410) COLOR0(5) = 0

   sta COLOR0+5
;(412) 

   lda #>fontFileName
   sta CHBAS
start__game:
;(415) 	initialize

   jsr initialize
;(417) 	drawArea ; draws the playfield and waits for the trigger to start

   jsr drawArea
;(418) 

   lda #root__HSx
   sta drawSnakeBegin__x
   lda #root__HSy
   sta drawSnakeBegin__y
   lda #root__SLength
   sta drawSnakeBegin__len
;(419) 	drawSnakeBegin(HSx HSy SLength)

   jsr drawSnakeBegin
;(420) 	generateFruit ; put the first fruit

   jsr generateFruit
;(422) 	grow:bool = 0

   lda #0
   sta grow
;(424) 	

loop:
;(427) 	if STRIG(0) = pressed

   lda STRIG
   sta _58
   cmp #button__state__pressed
   jne _lbl109
;(428) 		delayCntr = 2 ; if we push the trigger, the snake goes faster

   lda #2
   sta delayCntr
;(429) 	else

   jmp _lbl110
_lbl109:
;(430) 		delayCntr = LevDelay

   lda LevDelay
   sta delayCntr
;(431) 

_lbl110:
;(432) 	nDir = HDir

   lda HDir
   sta nDir
   ldy HDir
   jmp _lbl111
_lbl134:
;(437) 		timer = 0

   lda #0
   sta timer
   ldx _s16__ss
   jmp _lbl113
_lbl133:
;(440) 			ss:stick__state = STICK(0)

   lda STICK
   tax
   tya
   cpy #stick__state__right
   jne _lbl115
;(441) 			if HDir = right then ss = ss or 8

   txa
   ora #8
   tax
   jmp _lbl116
_lbl115:
   tya
   cpy #stick__state__up
   jne _lbl117
;(442) 			else if HDir = up then ss = ss or 1

   txa
   ora #1
   tax
   jmp _lbl116
_lbl117:
   tya
   cpy #stick__state__left
   jne _lbl118
;(443) 			else if HDir = left then ss = ss or 4

   txa
   ora #4
   tax
   jmp _lbl116
_lbl118:
   tya
   cpy #stick__state__down
   jne _lbl119
;(444) 			else if HDir = down then ss = ss or 2

   txa
   ora #2
   tax
;(445) 				

_lbl119:
_lbl116:
;(446) 			if ss = right

   txa
   cpx #stick__state__right
   jne _lbl120
   tya
   cpy #stick__state__up
   jne _lbl121
;(447) 				if HDir = up then nDir = ss

   txa
   stx nDir
_lbl121:
   tya
   cpy #stick__state__down
   jne _lbl122
;(448) 				if HDir = down then nDir = ss

   txa
   stx nDir
_lbl122:
   jmp _lbl123
_lbl120:
;(449) 			else if ss = left

   txa
   cpx #stick__state__left
   jne _lbl124
   tya
   cpy #stick__state__up
   jne _lbl125
;(450) 				if HDir = up then nDir = ss

   txa
   stx nDir
_lbl125:
   tya
   cpy #stick__state__down
   jne _lbl126
;(451) 				if HDir = down then nDir = ss

   txa
   stx nDir
_lbl126:
   jmp _lbl123
_lbl124:
;(452) 			else if ss = up

   txa
   cpx #stick__state__up
   jne _lbl127
   tya
   cpy #stick__state__left
   jne _lbl128
;(453) 				if HDir = left then nDir = ss

   txa
   stx nDir
_lbl128:
   tya
   cpy #stick__state__right
   jne _lbl129
;(454) 				if HDir = right then nDir = ss

   txa
   stx nDir
_lbl129:
   jmp _lbl123
_lbl127:
;(455) 			else if ss = down

   txa
   cpx #stick__state__down
   jne _lbl130
   tya
   cpy #stick__state__left
   jne _lbl131
;(456) 				if HDir = left then nDir = ss

   txa
   stx nDir
_lbl131:
   tya
   cpy #stick__state__right
   jne _lbl132
;(457) 				if HDir = right then nDir = ss

   txa
   stx nDir
;(458) 		

_lbl132:
_lbl130:
_lbl123:
_lbl113:
;(438) 		while timer < 1

   lda timer
   cmp #1
   jcs _lbl114
   jmp _lbl133
_lbl114:
   stx _s16__ss
;(460) 		

   dec delayCntr
_lbl111:
;(434) 	while delayCntr > 0	; delay loop

   lda delayCntr
   cmp #0
   jcc _lbl112
   jeq _lbl112
   jmp _lbl134
_lbl112:
;(461) 	HDir = nDir ; new direction

   lda nDir
   sta HDir
;(465) 	if grow = 0 

   lda grow
   cmp #0
   jne _lbl135
;(466) 		moveTail ; only if the snake hasn__t eaten a fruit before

   jsr moveTail
;(467) 	else 

   jmp _lbl136
_lbl135:
;(468) 		grow = 0

   lda #0
   sta grow
;(469) 	

_lbl136:
   jsr moveHead
;(470) 	if moveHead = 1

   lda moveHead__gotFruit
   cmp #1
   jne _lbl137
;(471) 		eatFruit

   jsr eatFruit
;(472) 		generateFruit

   jsr generateFruit
;(473) 		grow = 1

   lda #1
   sta grow
;(474) 

_lbl137:
;(475) goto loop

   jmp loop
;(479) 

game__over:
;(481) 

   ldy #24
   lda _scr_lo,y
   clc
   adc #11
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(15),c'-+ GAME OVER +-'
;(482) _scr(11,24) = "-+ GAME OVER +-"

   dta b(0)
;(485) 

   ldx _59
   jmp _lbl138
_lbl140:
_lbl138:
;(484) until STRIG(0) = not__pressed

   lda STRIG
   tax
   cmp #button__state__not__pressed
   jeq _lbl139
   jmp _lbl140
_lbl139:
   stx _59
;(488) 

   ldx _s18___45+0
   ldy _s18___45+1
   jmp _lbl141
_lbl143:
   lda VCOUNT
   asl
   tax
   lda #0
   rol
   tay
;(487) 	COL__BK = VCOUNT * 2 + RTCLOCK	; do the Atari rainbow

   txa
   clc
   adc RTCLOCK
   sta COL__BK
   lda #0
   rol
   sta COL__BK+1
_lbl141:
;(486) until STRIG(0) = pressed

   lda STRIG
   sta _60
   cmp #button__state__pressed
   jeq _lbl142
   jmp _lbl143
_lbl142:
   sty _s18___45+1
   stx _s18___45
;(490) 

   ldx _61
   jmp _lbl144
_lbl146:
_lbl144:
;(489) until STRIG(0) = not__pressed

   lda STRIG
   tax
   cmp #button__state__not__pressed
   jeq _lbl145
   jmp _lbl146
_lbl145:
   stx _61
;(491) goto start__game

   jmp start__game
;(492) 
   jmp *
   icl 'atari.asm'
dl:
;(17) ;Display list 

   dta b(112)
   dta b(112)
   dta b(66)
   dta a(_scr)
   dta b(16)
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
   dta b(16)
   dta b(2)
   dta b(65)
;(18) const dl:array = 2 times $70, $42, _scr, $10, 23 times $02, $10, $02, $41, dl

   dta a(dl)
   .align 1024
fontFileName:
;(22) ;Font file

   ins 'snake.fnt'
_scr_lo:  :25 dta l(_scr + #*40)
_scr_hi:  :25 dta h(_scr + #*40)
initialize .proc
;(79) 	Score = 0

   lda #0
   sta Score
   lda #0
   sta Score+1
;(80) 	Level = 1

   lda #1
   sta Level
;(81) 	LevDelay = 10

   lda #10
   sta LevDelay
   rts
.endp
writeCredits .proc
;(84) writeCredits:proc =

   ldy #7
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'+--------------------------------+'
;(85) 	_scr(3,7)   = "+--------------------------------+"

   dta b(0)
   ldy #8
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'!                                !'
;(86) 	_scr(3,8)   = "!                                !"

   dta b(0)
   ldy #9
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'!       PROGRAMMED BY JKR        !'
;(87) 	_scr(3,9)   = "!       PROGRAMMED BY JKR        !"

   dta b(0)
   ldy #10
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'!                                !'
;(88) 	_scr(3,10)  = "!                                !"

   dta b(0)
   ldy #11
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'! IN ATALAN PROGRAMMING LANGUAGE !'
;(89) 	_scr(3,11)  = "! IN ATALAN PROGRAMMING LANGUAGE !"

   dta b(0)
   ldy #12
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'!                                !'
;(90) 	_scr(3,12)  = "!                                !"

   dta b(0)
   ldy #13
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'!     KATOWICE, AUGUST 2010      !'
;(91) 	_scr(3,13)  = "!     KATOWICE, AUGUST 2010      !"

   dta b(0)
   ldy #14
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'!                                !'
;(92) 	_scr(3,14)  = "!                                !"

   dta b(0)
   ldy #15
   lda _scr_lo,y
   clc
   adc #3
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(34),c'+--------------------------------+'
;(93) 	_scr(3,15)  = "+--------------------------------+"

   dta b(0)
   rts
.endp
erasePlayfield .proc
;(99) 	

   lda #2
   sta _s0__yy
_lbl4:
;(98) 

   lda #1
   sta _s1__xx
_lbl3:
;(97) 	for yy:2..22 for xx:1..38 _scr(xx, yy) = 0

   ldy _s0__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy _s1__xx
   lda #0
   sta (_arr),y
   inc _s1__xx
   lda _s1__xx
   cmp #39
   jne _lbl3
   inc _s0__yy
   lda _s0__yy
   cmp #23
   jne _lbl4
   rts
.endp
drawSnakeBegin .proc
;(101) 	Hx = x

   lda drawSnakeBegin__x
   sta Hx
;(102) 	Hy = y

   lda drawSnakeBegin__y
   sta Hy
;(103) 	Tx = x - len

   lda drawSnakeBegin__x
   sec
   sbc drawSnakeBegin__len
   sta Tx
;(104) 	Ty = y

   lda drawSnakeBegin__y
   sta Ty
;(106) 	xx = Tx

   lda Tx
   sta drawSnakeBegin__xx
   jmp _lbl5
_lbl7:
;(108) 		_scr(xx, Hy) = SNAKE_BODY_h

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy drawSnakeBegin__xx
   lda #root__SNAKE_BODY_h
   sta (_arr),y
;(110) 

   inc drawSnakeBegin__xx
_lbl5:
;(107) 	until xx > Hx

   lda drawSnakeBegin__xx
   cmp Hx
   jeq _lbl147
   jcs _lbl6
_lbl147:
   jmp _lbl7
_lbl6:
;(111) 	_scr(Hx, Hy) = SNAKE_HEAD_r

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda #root__SNAKE_HEAD_r
   sta (_arr),y
;(112) 	_scr(Tx, Ty) = SNAKE_TAIL_r

   ldy Ty
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Tx
   lda #root__SNAKE_TAIL_r
   sta (_arr),y
;(114) 	HDir = right

   lda #stick__state__right
   sta HDir
   rts
.endp
getNextHeadCoord .proc
;(119) 	if HDir = right

   lda HDir
   cmp #stick__state__right
   jne _lbl8
;(120) 		x = Hx + 1

   lda Hx
   clc
   adc #1
   sta getNextHeadCoord__x
;(121) 		y = Hy

   lda Hy
   sta getNextHeadCoord__y
   jmp _lbl9
_lbl8:
;(122) 	else if HDir = up

   lda HDir
   cmp #stick__state__up
   jne _lbl10
;(123) 		x = Hx 

   lda Hx
   sta getNextHeadCoord__x
;(124) 		y = Hy - 1

   lda Hy
   sec
   sbc #1
   sta getNextHeadCoord__y
   jmp _lbl9
_lbl10:
;(125) 	else if HDir = left

   lda HDir
   cmp #stick__state__left
   jne _lbl11
;(126) 		x = Hx - 1

   lda Hx
   sec
   sbc #1
   sta getNextHeadCoord__x
;(127) 		y = Hy

   lda Hy
   sta getNextHeadCoord__y
;(128) 	else

   jmp _lbl9
_lbl11:
;(129) 		x = Hx

   lda Hx
   sta getNextHeadCoord__x
;(130) 		y = Hy + 1

   lda Hy
   clc
   adc #1
   sta getNextHeadCoord__y
;(132) 		

_lbl9:
   rts
.endp
getNextTailCoord .proc
;(135) 	if ss = SNAKE_TAIL_r

   lda getNextTailCoord__ss
   cmp #root__SNAKE_TAIL_r
   jne _lbl12
;(136) 		x = Tx + 1

   lda Tx
   clc
   adc #1
   sta getNextTailCoord__x
;(137) 		y = Ty

   lda Ty
   sta getNextTailCoord__y
   jmp _lbl13
_lbl12:
;(138) 	else if ss = SNAKE_TAIL_l

   lda getNextTailCoord__ss
   cmp #root__SNAKE_TAIL_l
   jne _lbl14
;(139) 		x = Tx - 1

   lda Tx
   sec
   sbc #1
   sta getNextTailCoord__x
;(140) 		y = Ty

   lda Ty
   sta getNextTailCoord__y
   jmp _lbl13
_lbl14:
;(141) 	else if ss = SNAKE_TAIL_u

   lda getNextTailCoord__ss
   cmp #root__SNAKE_TAIL_u
   jne _lbl15
;(142) 		x = Tx

   lda Tx
   sta getNextTailCoord__x
;(143) 		y = Ty - 1

   lda Ty
   sec
   sbc #1
   sta getNextTailCoord__y
   jmp _lbl13
_lbl15:
;(144) 	else if ss = SNAKE_TAIL_d

   lda getNextTailCoord__ss
   cmp #root__SNAKE_TAIL_d
   jne _lbl16
;(145) 		x = Tx

   lda Tx
   sta getNextTailCoord__x
;(146) 		y = Ty + 1

   lda Ty
   clc
   adc #1
   sta getNextTailCoord__y
;(148) 		

_lbl16:
_lbl13:
   rts
.endp
moveTail .proc
;(150) 	ts = _scr(Tx,Ty) ; current __tail__ character

   ldy Ty
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Tx
   lda (_arr),y
   sta moveTail__ts
;(151) 	xx,yy = getNextTailCoord ts

   lda moveTail__ts
   sta getNextTailCoord__ss
   jsr getNextTailCoord
   lda getNextTailCoord__x
   sta moveTail__xx
   lda getNextTailCoord__y
   sta moveTail__yy
;(152) 	ns = _scr(xx,yy) ; snake__s character next to the __tail__

   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda (_arr),y
   sta moveTail__ns
;(154) 	_scr(Tx,Ty) = 0 ; erase the tail

   ldy Ty
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Tx
   lda #0
   sta (_arr),y
;(157) 	if ns = SNAKE_BODY_nw 

   lda moveTail__ns
   cmp #root__SNAKE_BODY_nw
   jne _lbl17
   lda moveTail__ts
   cmp #root__SNAKE_TAIL_r
   jne _lbl18
   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_u
   sta (_arr),y
   jmp _lbl19
_lbl18:
;(158) 		if ts = SNAKE_TAIL_r then _scr(xx,yy) = SNAKE_TAIL_u else _scr(xx,yy) = SNAKE_TAIL_l

   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_l
   sta (_arr),y
;(159) 			

_lbl19:
   jmp _lbl20
_lbl17:
;(160) 	else if ns = SNAKE_BODY_ws 

   lda moveTail__ns
   cmp #root__SNAKE_BODY_ws
   jne _lbl21
   lda moveTail__ts
   cmp #root__SNAKE_TAIL_r
   jne _lbl22
   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_d
   sta (_arr),y
   jmp _lbl23
_lbl22:
;(161) 		if ts = SNAKE_TAIL_r _scr(xx,yy) = SNAKE_TAIL_d else _scr(xx,yy) = SNAKE_TAIL_l

   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_l
   sta (_arr),y
;(162) 			

_lbl23:
   jmp _lbl20
_lbl21:
;(163) 	else if ns = SNAKE_BODY_se

   lda moveTail__ns
   cmp #root__SNAKE_BODY_se
   jne _lbl24
   lda moveTail__ts
   cmp #root__SNAKE_TAIL_u
   jne _lbl25
   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_r
   sta (_arr),y
   jmp _lbl26
_lbl25:
;(164) 		if ts = SNAKE_TAIL_u _scr(xx,yy) = SNAKE_TAIL_r else _scr(xx,yy) = SNAKE_TAIL_d

   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_d
   sta (_arr),y
;(165) 			

_lbl26:
   jmp _lbl20
_lbl24:
;(166) 	else if ns = SNAKE_BODY_en

   lda moveTail__ns
   cmp #root__SNAKE_BODY_en
   jne _lbl27
   lda moveTail__ts
   cmp #root__SNAKE_TAIL_l
   jne _lbl28
   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_u
   sta (_arr),y
   jmp _lbl29
_lbl28:
;(167) 		if ts = SNAKE_TAIL_l _scr(xx,yy) = SNAKE_TAIL_u else _scr(xx,yy) = SNAKE_TAIL_r

   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda #root__SNAKE_TAIL_r
   sta (_arr),y
;(168) 

_lbl29:
;(169) 	else

   jmp _lbl20
_lbl27:
;(170) 		_scr(xx,yy) = ts

   ldy moveTail__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveTail__xx
   lda moveTail__ts
   sta (_arr),y
;(171) 		

_lbl20:
;(172) 	Tx = xx

   lda moveTail__xx
   sta Tx
;(173) 	Ty = yy

   lda moveTail__yy
   sta Ty
   rts
.endp
putNeck .proc
;(180) 	if HDir = dir1

   lda HDir
   cmp putNeck__dir1
   jne _lbl30
;(181) 		_scr(Hx,Hy) = neck1  

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda putNeck__neck1
   sta (_arr),y
   jmp _lbl31
_lbl30:
;(182) 	else if HDir = dir2

   lda HDir
   cmp putNeck__dir2
   jne _lbl32
;(183) 		_scr(Hx,Hy) = neck2

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda putNeck__neck2
   sta (_arr),y
;(184) 	else 

   jmp _lbl31
_lbl32:
;(185) 		_scr(Hx,Hy) = body

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda putNeck__body
   sta (_arr),y
;(187) 		

_lbl31:
   rts
.endp
increaseLevel .proc
;(189) 	inc(Level)

   inc Level
;(190) 	dec(LevDelay)

   dec LevDelay
   rts
.endp
eatFruit .proc
;(194) 	Score = Score + 10 * Level

   lda Level
   ldx Level
   jsr _sys_mul8
   lda _TEMPW2
   sta eatFruit___12
   lda Score
   clc
   adc eatFruit___12
   sta Score
   jcc _lbl148
   inc Score+1
_lbl148:
;(195) 			

   lda Score
   cmp #150
   jne _lbl33
   lda Score+1
   cmp #0
   jne _lbl33
_lbl149:
;(196) 	if Score = 150 increaseLevel ; 15 * 10

   jsr increaseLevel
_lbl33:
   lda Score
   cmp #194
   jne _lbl34
   lda Score+1
   cmp #1
   jne _lbl34
_lbl150:
;(197) 	if Score = 450 increaseLevel ; + 15 * 20

   jsr increaseLevel
_lbl34:
   lda Score
   cmp #132
   jne _lbl35
   lda Score+1
   cmp #3
   jne _lbl35
_lbl151:
;(198) 	if Score = 900 increaseLevel ; + 15 * 30 

   jsr increaseLevel
_lbl35:
   lda Score
   cmp #220
   jne _lbl36
   lda Score+1
   cmp #5
   jne _lbl36
_lbl152:
;(199) 	if Score = 1500 increaseLevel ; + 15 * 40

   jsr increaseLevel
_lbl36:
   lda Score
   cmp #196
   jne _lbl37
   lda Score+1
   cmp #9
   jne _lbl37
_lbl153:
;(200) 	if Score = 2500 increaseLevel ; + 20 * 50

   jsr increaseLevel
_lbl37:
   lda Score
   cmp #116
   jne _lbl38
   lda Score+1
   cmp #14
   jne _lbl38
_lbl154:
;(201) 	if Score = 3700 increaseLevel ; + 20 * 60

   jsr increaseLevel
_lbl38:
   lda Score
   cmp #168
   jne _lbl39
   lda Score+1
   cmp #22
   jne _lbl39
_lbl155:
;(202) 	if Score = 5800 increaseLevel ; + 30 * 70

   jsr increaseLevel
_lbl39:
   lda Score
   cmp #40
   jne _lbl40
   lda Score+1
   cmp #35
   jne _lbl40
_lbl156:
;(203) 	if Score = 9000 increaseLevel ; + 40 * 80

   jsr increaseLevel
;(204) 		

_lbl40:
;(206) ;	s:Score = Score  	; for some reason __"... SCORE: [Score]"__ doesn__t work

   ldy #24
   lda _scr_lo,y
   clc
   adc #0
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(8),c' LEVEL: '
   dta b(129),a(Level)
   dta b(26),c'                   SCORE: '
   dta b(130),a(Score)
;(207) 	_scr(0,24) = " LEVEL: [Level]                   SCORE: [Score]" 

   dta b(0)
   rts
.endp
generateFruit .proc
;(211) 	; generate random coordinates inside the playing area: 1..38, 2..22

   lda RANDOM
   and #63
   sta generateFruit___14
;(212) 	x = (RANDOM and 63) + 1

   lda generateFruit___14
   clc
   adc #1
   sta generateFruit__x
   lda RANDOM
   and #31
   sta generateFruit___16
;(213) 	y = (RANDOM and 31) + 2

   lda generateFruit___16
   clc
   adc #2
   sta generateFruit__y
   jmp _lbl41
_lbl43:
;(214) 	while x > 38 x = x - 38

   lda generateFruit__x
   sec
   sbc #38
   sta generateFruit__x
_lbl41:
   lda generateFruit__x
   cmp #38
   jcc _lbl42
   jeq _lbl42
   jmp _lbl43
_lbl42:
;(216) 	; if this position is occupied, find a free one by increasing (and wrapping) x, then y

   jmp _lbl44
_lbl46:
;(215) 	while y > 22 y = y - 22

   lda generateFruit__y
   sec
   sbc #22
   sta generateFruit__y
_lbl44:
   lda generateFruit__y
   cmp #22
   jcc _lbl45
   jeq _lbl45
   jmp _lbl46
_lbl45:
   jmp _lbl47
_lbl53:
;(218) 		if x <> 38

   lda generateFruit__x
   cmp #38
   jeq _lbl49
;(219) 			inc x

   inc generateFruit__x
;(220) 		else

   jmp _lbl50
_lbl49:
;(221) 			x = 1

   lda #1
   sta generateFruit__x
   lda generateFruit__y
   cmp #22
   jne _lbl51
;(223) 				y = 2

   lda #2
   sta generateFruit__y
;(224) 			else 

   jmp _lbl52
_lbl51:
;(226) 	; put the fruit on the screen				

   inc generateFruit__y
_lbl52:
_lbl50:
_lbl47:
;(217) 	while _scr(x,y) <> 0

   ldy generateFruit__y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy generateFruit__x
   lda (_arr),y
   sta _47
   lda _47
   cmp #0
   jeq _lbl48
   jmp _lbl53
_lbl48:
;(227) 	_scr(x,y) = FRUIT

   ldy generateFruit__y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy generateFruit__x
   lda #root__FRUIT
   sta (_arr),y
   rts
.endp
moveHead .proc
;(231) 	gotFruit = 0

   lda #0
   sta moveHead__gotFruit
;(233) 	hs = _scr(Hx,Hy) ; current __head__ character

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda (_arr),y
   sta moveHead__hs
;(234) 	xx,yy = getNextHeadCoord

   jsr getNextHeadCoord
   lda getNextHeadCoord__x
   sta moveHead__xx
   lda getNextHeadCoord__y
   sta moveHead__yy
;(235) 	ns = _scr(xx,yy) ; character in front of the __head__

   ldy moveHead__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveHead__xx
   lda (_arr),y
   sta moveHead__ns
;(238) 	if HDir = right	 

   lda HDir
   cmp #stick__state__right
   jne _lbl54
;(239) 		nh = SNAKE_HEAD_r

   lda #root__SNAKE_HEAD_r
   sta moveHead__nh
   jmp _lbl55
_lbl54:
;(240) 	else if HDir = up

   lda HDir
   cmp #stick__state__up
   jne _lbl56
;(241) 		nh = SNAKE_HEAD_u

   lda #root__SNAKE_HEAD_u
   sta moveHead__nh
   jmp _lbl55
_lbl56:
;(242) 	else if HDir = left

   lda HDir
   cmp #stick__state__left
   jne _lbl57
;(243) 		nh = SNAKE_HEAD_l

   lda #root__SNAKE_HEAD_l
   sta moveHead__nh
;(244) 	else

   jmp _lbl55
_lbl57:
;(245) 		nh = SNAKE_HEAD_d

   lda #root__SNAKE_HEAD_d
   sta moveHead__nh
;(247) 	; turn the head

_lbl55:
;(248) 	_scr(Hx,Hy) = nh

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda moveHead__nh
   sta (_arr),y
;(251) 	if ns = FRUIT

   lda moveHead__ns
   cmp #root__FRUIT
   jne _lbl58
;(252) 		gotFruit = 1 

   lda #1
   sta moveHead__gotFruit
   jmp _lbl59
_lbl58:
;(253) 	else if ns <> 0 

   lda moveHead__ns
   cmp #0
   jeq _lbl60
;(254) 		goto game__over

   jmp game__over
;(256) 	; find and put the character next to the head

_lbl60:
_lbl59:
;(257) 	if hs = SNAKE_HEAD_r 

   lda moveHead__hs
   cmp #root__SNAKE_HEAD_r
   jne _lbl61
   lda #stick__state__up
   sta putNeck__dir1
   lda #root__SNAKE_BODY_nw
   sta putNeck__neck1
   lda #stick__state__down
   sta putNeck__dir2
   lda #root__SNAKE_BODY_ws
   sta putNeck__neck2
   lda #root__SNAKE_BODY_h
   sta putNeck__body
;(258) 		putNeck(up SNAKE_BODY_nw down SNAKE_BODY_ws SNAKE_BODY_h)

   jsr putNeck
;(259) 			

_lbl61:
;(260) 	if hs = SNAKE_HEAD_u 

   lda moveHead__hs
   cmp #root__SNAKE_HEAD_u
   jne _lbl62
   lda #stick__state__left
   sta putNeck__dir1
   lda #root__SNAKE_BODY_ws
   sta putNeck__neck1
   lda #stick__state__right
   sta putNeck__dir2
   lda #root__SNAKE_BODY_se
   sta putNeck__neck2
   lda #root__SNAKE_BODY_v
   sta putNeck__body
;(261) 		putNeck(left SNAKE_BODY_ws right SNAKE_BODY_se SNAKE_BODY_v)

   jsr putNeck
;(262) 		

_lbl62:
;(263) 	if hs = SNAKE_HEAD_l 

   lda moveHead__hs
   cmp #root__SNAKE_HEAD_l
   jne _lbl63
   lda #stick__state__up
   sta putNeck__dir1
   lda #root__SNAKE_BODY_en
   sta putNeck__neck1
   lda #stick__state__down
   sta putNeck__dir2
   lda #root__SNAKE_BODY_se
   sta putNeck__neck2
   lda #root__SNAKE_BODY_h
   sta putNeck__body
;(264) 		putNeck(up SNAKE_BODY_en down SNAKE_BODY_se SNAKE_BODY_h)

   jsr putNeck
;(265) 		

_lbl63:
;(266) 	if hs = SNAKE_HEAD_d 

   lda moveHead__hs
   cmp #root__SNAKE_HEAD_d
   jne _lbl64
   lda #stick__state__left
   sta putNeck__dir1
   lda #root__SNAKE_BODY_nw
   sta putNeck__neck1
   lda #stick__state__right
   sta putNeck__dir2
   lda #root__SNAKE_BODY_en
   sta putNeck__neck2
   lda #root__SNAKE_BODY_v
   sta putNeck__body
;(267) 		putNeck(left SNAKE_BODY_nw right SNAKE_BODY_en SNAKE_BODY_v)

   jsr putNeck
;(269) 	; put the head character in the new position

_lbl64:
;(270) 	_scr(xx,yy) = nh

   ldy moveHead__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy moveHead__xx
   lda moveHead__nh
   sta (_arr),y
;(271) 	Hx = xx

   lda moveHead__xx
   sta Hx
;(272) 	Hy = yy

   lda moveHead__yy
   sta Hy
   rts
.endp
drawSnakeAtRandomPosition .proc
;(277) drawSnakeAtRandomPosition:proc =

   lda RANDOM
   and #15
   sta drawSnakeAtRandomPosition___20
;(278) 	rl:6..21 = (RANDOM and 15) + 6			;rl = 6..15+6

   lda drawSnakeAtRandomPosition___20
   clc
   adc #6
   sta drawSnakeAtRandomPosition__rl
   jmp _lbl65
_lbl67:
;(280) 		rl = rl - 9

   lda drawSnakeAtRandomPosition__rl
   sec
   sbc #9
   sta drawSnakeAtRandomPosition__rl
_lbl65:
;(279) 	while rl > 15 

   lda drawSnakeAtRandomPosition__rl
   cmp #15
   jcc _lbl66
   jeq _lbl66
   jmp _lbl67
_lbl66:
   lda RANDOM
   and #15
   sta drawSnakeAtRandomPosition___23
;(281) 	ry:Hy = (RANDOM and 15) + 2

   lda drawSnakeAtRandomPosition___23
   clc
   adc #2
   sta drawSnakeAtRandomPosition__ry
   jmp _lbl68
_lbl70:
;(283) 		ry = ry - 11

   lda drawSnakeAtRandomPosition__ry
   sec
   sbc #11
   sta drawSnakeAtRandomPosition__ry
_lbl68:
;(282) 	while ry > 13 

   lda drawSnakeAtRandomPosition__ry
   cmp #13
   jcc _lbl69
   jeq _lbl69
   jmp _lbl70
_lbl69:
   lda drawSnakeAtRandomPosition__ry
   cmp #6
   jcc _lbl71
   jeq _lbl71
;(284) 	if ry > 6 then ry = ry + 9

   lda drawSnakeAtRandomPosition__ry
   clc
   adc #9
   sta drawSnakeAtRandomPosition__ry
_lbl71:
   lda RANDOM
   and #63
   sta drawSnakeAtRandomPosition___27
   lda drawSnakeAtRandomPosition__rl
   clc
   adc drawSnakeAtRandomPosition__rl
   sta drawSnakeAtRandomPosition___28
;(285) 	rx:Hx = (RANDOM and 63) + (1 + rl)

   lda drawSnakeAtRandomPosition___27
   clc
   adc drawSnakeAtRandomPosition___28
   sta drawSnakeAtRandomPosition__rx
;(288) 		

   jmp _lbl72
_lbl74:
   lda drawSnakeAtRandomPosition__rl
   clc
   adc drawSnakeAtRandomPosition__rl
   sta _s8___30
   lda drawSnakeAtRandomPosition__rx
   clc
   adc _s8___30
   sta _s8___31
;(287) 		rx = rx + (1 + rl) - 38

   lda _s8___31
   sec
   sbc #38
   sta drawSnakeAtRandomPosition__rx
_lbl72:
;(286) 	while rx > 38 

   lda drawSnakeAtRandomPosition__rx
   cmp #38
   jcc _lbl73
   jeq _lbl73
   jmp _lbl74
_lbl73:
   lda drawSnakeAtRandomPosition__rx
   sta drawSnakeBegin__x
   lda drawSnakeAtRandomPosition__ry
   sta drawSnakeBegin__y
   lda drawSnakeAtRandomPosition__rl
   sta drawSnakeBegin__len
;(289) 	drawSnakeBegin(rx ry rl)

   jsr drawSnakeBegin
   rts
.endp
changeDirection .proc
;(292) changeDirection:proc =

   lda Hx
   clc
   adc #1
   sta changeDirection___33
;(293) 	if _scr(Hx+1,Hy) <> 0

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy changeDirection___33
   lda (_arr),y
   sta _48
   lda _48
   cmp #0
   jeq _lbl75
   lda Hx
   sec
   sbc #1
   sta changeDirection___34
;(294) 		if _scr(Hx-1,Hy) <> 0

   ldy Hy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy changeDirection___34
   lda (_arr),y
   sta _49
   lda _49
   cmp #0
   jeq _lbl76
   lda Hy
   clc
   adc #1
   sta changeDirection___35
;(295) 			if _scr(Hx,Hy+1) <> 0

   ldy changeDirection___35
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda (_arr),y
   sta _50
   lda _50
   cmp #0
   jeq _lbl77
   lda Hy
   sec
   sbc #1
   sta changeDirection___36
;(296) 				if _scr(Hx,Hy-1) <> 0

   ldy changeDirection___36
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy Hx
   lda (_arr),y
   sta _51
   lda _51
   cmp #0
   jeq _lbl78
;(299) 					timer = 0

   lda #0
   sta timer
   jmp _lbl79
_lbl82:
   lda VCOUNT
   asl
   sta _s9___37
   lda #0
   rol
   sta _s9___37+1
;(301) 						COL__BK = VCOUNT * 2 + RTCLOCK	; do the Atari rainbow

   lda _s9___37
   clc
   adc RTCLOCK
   sta COL__BK
   lda #0
   rol
   sta COL__BK+1
   lda STRIG
   sta _52
   lda _52
   cmp #button__state__pressed
   jne _lbl81
;(302) 						if STRIG(0) = pressed  goto ex1

   jmp ex1
;(303) 

_lbl81:
_lbl79:
;(300) 					while timer < 100

   lda timer
   cmp #100
   jcs _lbl80
   jmp _lbl82
_lbl80:
;(304) 					erasePlayfield

   jsr erasePlayfield
;(305) 					writeCredits

   jsr writeCredits
;(306) 					drawSnakeAtRandomPosition

   jsr drawSnakeAtRandomPosition
;(307) 					goto ex1

   jmp ex1
;(308) 

_lbl78:
_lbl77:
_lbl76:
_lbl75:
;(309) 	r = RANDOM 

   lda RANDOM
   sta changeDirection__r
   lda changeDirection__r
   cmp #128
   jcc _lbl83
;(310) 	if r >= 128 goto turn__right

   jmp turn__right
;(311) 	

_lbl83:
turn__left:
;(313) 	if HDir = right	

   lda HDir
   cmp #stick__state__right
   jne _lbl84
;(314) 		HDir = up

   lda #stick__state__up
   sta HDir
   jmp _lbl85
_lbl84:
;(315) 	else if HDir = up

   lda HDir
   cmp #stick__state__up
   jne _lbl86
;(316) 		HDir = left

   lda #stick__state__left
   sta HDir
   jmp _lbl85
_lbl86:
;(317) 	else if HDir = left

   lda HDir
   cmp #stick__state__left
   jne _lbl87
;(318) 		HDir = down

   lda #stick__state__down
   sta HDir
;(319) 	else

   jmp _lbl85
_lbl87:
;(320) 		HDir = right

   lda #stick__state__right
   sta HDir
;(321) 	

_lbl85:
;(322) 	xx,yy = getNextHeadCoord

   jsr getNextHeadCoord
   lda getNextHeadCoord__x
   sta changeDirection__xx
   lda getNextHeadCoord__y
   sta changeDirection__yy
;(323) 	if _scr(xx,yy) <> 0 

   ldy changeDirection__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy changeDirection__xx
   lda (_arr),y
   sta _53
   lda _53
   cmp #0
   jeq _lbl88
;(324) 		goto turn__left

   jmp turn__left
_lbl88:
;(325) 	moveHead

   jsr moveHead
;(326) 	goto ex1

   jmp ex1
;(328) 	

turn__right:
;(330) 	if HDir = right	

   lda HDir
   cmp #stick__state__right
   jne _lbl89
;(331) 		HDir = down

   lda #stick__state__down
   sta HDir
   jmp _lbl90
_lbl89:
;(332) 	else if HDir = up

   lda HDir
   cmp #stick__state__up
   jne _lbl91
;(333) 		HDir = right

   lda #stick__state__right
   sta HDir
   jmp _lbl90
_lbl91:
;(334) 	else if HDir = left

   lda HDir
   cmp #stick__state__left
   jne _lbl92
;(335) 		HDir = up

   lda #stick__state__up
   sta HDir
;(336) 	else

   jmp _lbl90
_lbl92:
;(337) 		HDir = left

   lda #stick__state__left
   sta HDir
;(338) 

_lbl90:
;(339) 	xx,yy = getNextHeadCoord

   jsr getNextHeadCoord
   lda getNextHeadCoord__x
   sta changeDirection__xx
   lda getNextHeadCoord__y
   sta changeDirection__yy
;(340) 	if _scr(xx,yy) <> 0 

   ldy changeDirection__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy changeDirection__xx
   lda (_arr),y
   sta _54
   lda _54
   cmp #0
   jeq _lbl93
;(341) 		goto turn__right

   jmp turn__right
_lbl93:
;(342) 	moveHead

   jsr moveHead
;(343) 	

ex1:
   rts
.endp
animateSnake .proc
;(348) 	if timer >= 12

   lda timer
   cmp #12
   jcc _lbl94
;(350) 		moveTail

   jsr moveTail
;(352) 		xx,yy = getNextHeadCoord

   jsr getNextHeadCoord
   lda getNextHeadCoord__x
   sta animateSnake__xx
   lda getNextHeadCoord__y
   sta animateSnake__yy
;(353) 		if _scr(xx,yy) <> 0

   ldy animateSnake__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy animateSnake__xx
   lda (_arr),y
   sta _55
   lda _55
   cmp #0
   jeq _lbl95
;(354) 			changeDirection

   jsr changeDirection
;(355) 			goto ex2

   jmp ex2
;(356) 

_lbl95:
   lda RANDOM
   and #15
   sta animateSnake__r
;(358) 		if r < 3 

   lda animateSnake__r
   cmp #3
   jcs _lbl96
;(359) 			changeDirection

   jsr changeDirection
;(360) 			goto ex2

   jmp ex2
;(361) 			

_lbl96:
;(362) 		moveHead

   jsr moveHead
;(363) 		

ex2:
;(366) 		timer = 0

   lda #0
   sta timer
;(369) 		

_lbl94:
   rts
.endp
drawArea .proc
;(370) drawArea:proc =

   ldy #0
   lda _scr_lo,y
   clc
   adc #0
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(40),c'       .-+  ATALAN THE SNAKE  +-.   V0.1'
;(371) 	_scr(0,0)  = "       .-+  ATALAN THE SNAKE  +-.   V0.1"

   dta b(0)
;(376) 		

   lda #0
   sta _s10__xx
_lbl98:
;(374) 		_scr(xx, 1) = WALL_CHR

   ldy #1
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy _s10__xx
   lda #root__WALL_CHR
   sta (_arr),y
;(375) 		_scr(xx, 23) = WALL_CHR

   ldy #23
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy _s10__xx
   lda #root__WALL_CHR
   sta (_arr),y
   inc _s10__xx
   lda _s10__xx
   cmp #40
   jne _lbl98
   lda #2
   sta _s11__yy
_lbl102:
;(378) 		_scr(0, yy) = WALL_CHR

   ldy _s11__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy #0
   lda #root__WALL_CHR
   sta (_arr),y
;(379) 		_scr(39, yy) = RWALL_CHR

   ldy _s11__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy #39
   lda #root__RWALL_CHR
   sta (_arr),y
;(382) 	

   lda #1
   sta _s12__xx
_lbl101:
;(381) 			_scr(xx, yy) = 0

   ldy _s11__yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy _s12__xx
   lda #0
   sta (_arr),y
   inc _s12__xx
   lda _s12__xx
   cmp #39
   jne _lbl101
   inc _s11__yy
   lda _s11__yy
   cmp #23
   jne _lbl102
;(383) 	_scr(39, 1) = RWALL_CHR

   ldy #1
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy #39
   lda #root__RWALL_CHR
   sta (_arr),y
;(384) 	_scr(39, 23) = RWALL_CHR

   ldy #23
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy #39
   lda #root__RWALL_CHR
   sta (_arr),y
;(386) 	writeCredits

   jsr writeCredits
;(387) 	

   ldy #24
   lda _scr_lo,y
   clc
   adc #0
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(40),c'     -+  PRESS TRIGGER TO START  +-     '
;(388) 	_scr(0,24) = "     -+  PRESS TRIGGER TO START  +-     "

   dta b(0)
;(390) 	timer = 0

   lda #0
   sta timer
;(391) 	drawSnakeAtRandomPosition

   jsr drawSnakeAtRandomPosition
;(396) 

   jmp _lbl103
_lbl105:
;(395) 		animateSnake

   jsr animateSnake
_lbl103:
;(394) 	until STRIG(0) = pressed

   lda STRIG
   sta _56
   lda _56
   cmp #button__state__pressed
   jeq _lbl104
   jmp _lbl105
_lbl104:
;(397) 	erasePlayfield

   jsr erasePlayfield
;(398) 

   ldy #24
   lda _scr_lo,y
   clc
   adc #0
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(40),c' LEVEL: 1                   SCORE: 0    '
;(399) 	_scr(0,24) = " LEVEL: 1                   SCORE: 0    "

   dta b(0)
;(407) ;Initialize graphics.

   jmp _lbl106
_lbl108:
_lbl106:
;(402) 	until STRIG(0) = not__pressed

   lda STRIG
   sta _57
   lda _57
   cmp #button__state__not__pressed
   jeq _lbl107
   jmp _lbl108
_lbl107:
   rts
.endp
