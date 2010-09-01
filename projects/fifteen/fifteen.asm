_arr equ 128
_TEMPW1 equ 50
_TEMPW2 equ 52
_TEMPL1 equ 112
timer equ 20
RANDOM equ 53770
WSYNC equ 54282
STICK equ 632
DMACTL equ 559
COLOR0 equ 708
COLPF equ 53270
PMBASE equ 54279
player_size equ 53256
player_x equ 53248
player_color equ 704
player_col2 equ 53266
missile_size equ 53260
missile_x equ 53252
GRACTL equ 53277
GTICTLS equ 623
AUDF1 equ 53760
AUDC1 equ 53761
key__none equ 255
CH equ 764
SDLSTL equ 560
NMIEN equ 54286
VDSLST equ 512
VVBLKD equ 548
mem equ 0
screen equ 41136
vcnt equ 1555
pmg_mem equ 39936
textbuf equ 40960
besttime equ 40976
playfield equ 40304
root__D_UP equ 0
root__D_RIGHT equ 1
root__D_DOWN equ 2
root__D_LEFT equ 3
root__T_EMPTY equ 15
root__XOFF equ 1
root__YOFF equ 8
i equ 1536
j equ 1537
k equ 1538
l equ 1539
xbtmp equ 1540
ybtmp equ 1541
xctmp equ 1542
yctmp equ 1544
ctmp1 equ 1546
bptr equ 1548
btmp1 equ 1550
btmp2 equ 1551
CONSOL equ 53279
mus_init equ 38656
mus_play equ 38659
mus_silence equ 38665
mus_setpokey equ 38668
dir equ 1552
play equ 1553
piccnt equ 1554
tsec equ 1556
tdsec equ 1557
tmin equ 1558
tdmin equ 1559
moves equ 1560
bmoves equ 1562
currcol equ 1564
currentbuf equ 1565
drawmainscreen__tmp equ 1567
sleep__time equ 130
copyblock__srcbm equ 131
copyblock__xbm equ 133
copyblock__ybm equ 134
copyblock__wbm equ 136
copyblock__xscr equ 137
copyblock__yscr equ 138
copyblock__xsize equ 139
copyblock__ysize equ 140
_s2__j equ 141
text__xt equ 145
text__yt equ 146
text__len equ 147
text__ii equ 148
_s3___12 equ 149
copytile__xbm equ 151
copytile__ybm equ 152
copytile__xscr equ 153
copytile__yscr equ 154
copytile__xsize equ 155
copytile__ysize equ 156
copytile___13 equ 157
eraserect__xscr equ 158
eraserect__yscr equ 159
eraserect__width equ 160
eraserect__height equ 161
findempty__x equ 162
findempty__y equ 163
gettile__tile equ 164
gettile__tx equ 165
gettile__ty equ 166
_s9___20 equ 167
_s9___22 equ 168
_s9__tile equ 169
movetile__xs equ 170
movetile__ys equ 171
movetile__m equ 172
movetile__show equ 173
movetile__tile equ 174
movetile__xoffset equ 175
movetile__yoffset equ 176
movetile___24 equ 177
movetile___26 equ 178
movetile__z equ 179
_s10___28 equ 180
_s10___30 equ 181
_s10___34 equ 182
_s10___43 equ 184
_s10___49 equ 185
_s10___51 equ 187
_s10___52 equ 188
_s10___54 equ 189
_s10___55 equ 190
changepicture__c equ 191
countgoodtiles__res equ 192
_s14__tile equ 193
_s16__tile equ 194
setboardcolor__col equ 195
changecolor__col equ 196
drawmainscreen___73 equ 197
_s19__i equ 199
_s19___75 equ 200
_s19___77 equ 202
_s19___79 equ 204
shuffletiles__c equ 206
shuffletiles__x equ 207
shuffletiles__y equ 208
shuffletiles__cnt equ 209
_s21__i equ 210
_s22__i equ 211
_s23__i equ 212
_s24__i equ 213
drawbesttime__tcnt equ 214
initscreen___95 equ 215
initscreen___96 equ 217
titlescreen__ctmp equ 219
game__cnt equ 220
game__ss equ 221
game__x equ 222
game__y equ 223
_98 equ 224
_99 equ 225
_100 equ 226
_101 equ 227
_102 equ 228
_103 equ 229
   org $2e0
   dta a($2000)
   org $2000
;### fifteen.atl(156) piccnt@$612:byte= RANDOM and 3
   lda RANDOM
   and #3
   sta piccnt
;### fifteen.atl(723) initscreen
   jsr initscreen
;### fifteen.atl(724) besttime(0)=0
   lda #0
   sta besttime
;### fifteen.atl(725) bmoves=999
   lda #231
   sta bmoves
   lda #3
   sta bmoves+1
;### fifteen.atl(731) 	game
   jmp _lbl138
;### fifteen.atl(731) 	game
_lbl139:
;### fifteen.atl(727) 	titlescreen
   jsr titlescreen
;### fifteen.atl(728) 	game
   jsr game
;### fifteen.atl(731) 	game
_lbl138:
;### fifteen.atl(731) 	game
   jmp _lbl139
   jmp *
   icl 'atari.asm'
dl:
;### fifteen.atl(27) const dl:array of byte = ( 2 times $70, $4f,  screen ,
   dta b(112)
   dta b(112)
   dta b(79)
   dta a(screen)
;### fifteen.atl(28) 	32 times $f,
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(29) 	$8f,
   dta b(143)
;### fifteen.atl(30) 	18 times $f
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(31) 	$8f,
   dta b(143)
;### fifteen.atl(32) 	12 times $f ,
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(33) 	$8f,
   dta b(143)
;### fifteen.atl(34) 	29 times $f ,
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(35) 	$8f
   dta b(143)
;### fifteen.atl(36) 	$f $f
   dta b(15)
   dta b(15)
;### fifteen.atl(37) 	$4f , 0, $B0 ,
   dta b(79)
   dta b(0)
   dta b(176)
;### fifteen.atl(38) 	40  times $f,
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(39) 	$8f
   dta b(143)
;### fifteen.atl(40) 	60  times $f,
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(41) 	$4f screen
   dta b(79)
   dta a(screen)
;### fifteen.atl(42) 	7 times $f
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(43) 	$41, dl
   dta b(65)
   dta a(dl)
;### fifteen.atl(44) 	)
   dta a(dl)
buf1:
;### fifteen.atl(128) const buf1: array(23, 191) = file "einstein-tongue.pbm"
   ins 'einstein-tongue.pbm'
;### fifteen.atl(128) const buf1: array(23, 191) = file "einstein-tongue.pbm"
   dta b(8)
buf2:
;### fifteen.atl(129) const buf2: array(23, 191) = file "miner.pbm"
   ins 'miner.pbm'
;### fifteen.atl(129) const buf2: array(23, 191) = file "miner.pbm"
   dta b(8)
buf3:
;### fifteen.atl(130) const buf3: array(23, 191) = file "johnromero.pbm"
   ins 'johnromero.pbm'
;### fifteen.atl(130) const buf3: array(23, 191) = file "johnromero.pbm"
   dta b(8)
buf4:
;### fifteen.atl(131) const buf4: array(23, 191) = file "clinteastwood.pbm"
   ins 'clinteastwood.pbm'
;### fifteen.atl(131) const buf4: array(23, 191) = file "clinteastwood.pbm"
   dta b(8)
leafh:
;### fifteen.atl(132) const leafh: array(47) = file "leafhoriz.pbm"
   ins 'leafhoriz.pbm'
;### fifteen.atl(132) const leafh: array(47) = file "leafhoriz.pbm"
   dta b(8)
leafv:
;### fifteen.atl(133) const leafv: array(47) = file "leafvert.pbm"
   ins 'leafvert.pbm'
;### fifteen.atl(133) const leafv: array(47) = file "leafvert.pbm"
   dta b(8)
mains:
;### fifteen.atl(134) const mains: array(2800) = file "15.pbm"
   ins '15.pbm'
;### fifteen.atl(134) const mains: array(2800) = file "15.pbm"
   dta b(8)
fonts:
;### fifteen.atl(135) const fonts: array(1024) = file "STENCIL.FNT"
   ins 'STENCIL.FNT'
;### fifteen.atl(136) const fonts: array(1024) = file "STENCIL.FNT"
   dta b(8)
screen_lo:  :200 dta l(screen + #*40)
screen_hi:  :200 dta h(screen + #*40)
textbuf_lo:  :1 dta l(textbuf + #*15)
textbuf_hi:  :1 dta h(textbuf + #*15)
besttime_lo:  :1 dta l(besttime + #*15)
besttime_hi:  :1 dta h(besttime + #*15)
playfield_lo:  :4 dta l(playfield + #*4)
playfield_hi:  :4 dta h(playfield + #*4)
buf1_lo:  :192 dta l(buf1 + #*24)
buf1_hi:  :192 dta h(buf1 + #*24)
buf2_lo:  :192 dta l(buf2 + #*24)
buf2_hi:  :192 dta h(buf2 + #*24)
buf3_lo:  :192 dta l(buf3 + #*24)
buf3_hi:  :192 dta h(buf3 + #*24)
buf4_lo:  :192 dta l(buf4 + #*24)
buf4_hi:  :192 dta h(buf4 + #*24)
set_col .proc
   pha
   txa
   pha
   tya
   pha
;### fifteen.atl(49) 	if vcnt = 0
   lda vcnt
   cmp #0
   jne _lbl1
;### fifteen.atl(50) 		inc vcnt
   inc vcnt
;### fifteen.atl(52) 		player_col2(3) = $9e
   sta WSYNC
   lda #158
   sta player_col2+3
;### fifteen.atl(53) 		COLPF(3) = $96
   lda #150
   sta COLPF+3
;### fifteen.atl(54) 		player_x(3)=180
   lda #180
   sta player_x+3
;### fifteen.atl(55) 		missile_x(2)=166
   lda #166
   sta missile_x+2
;### fifteen.atl(56) 		missile_x(3)=172
   lda #172
   sta missile_x+3
;### fifteen.atl(58) 		player_col2(3) = $8e
   sta WSYNC
   lda #142
   sta player_col2+3
;### fifteen.atl(59) 		COLPF(3) = $86
   lda #134
   sta COLPF+3
;### fifteen.atl(61) 		player_col2(3) = $5e
   sta WSYNC
   lda #94
   sta player_col2+3
;### fifteen.atl(62) 		COLPF(3) = $56
   lda #86
   sta COLPF+3
;### fifteen.atl(64) 		player_col2(3) = $4e
   sta WSYNC
   lda #78
   sta player_col2+3
;### fifteen.atl(65) 		COLPF(3) = $46
   lda #70
   sta COLPF+3
;### fifteen.atl(66) 	else if vcnt = 1
   jmp _lbl2
_lbl1:
   lda vcnt
   cmp #1
   jne _lbl3
;### fifteen.atl(68) 		COLPF(3) = $4e
   sta WSYNC
   lda #78
   sta COLPF+3
;### fifteen.atl(69) 		player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(70) 		missile_x(2)=192
   lda #192
   sta missile_x+2
;### fifteen.atl(71) 		missile_x(3)=200
   lda #200
   sta missile_x+3
;### fifteen.atl(72) 		inc vcnt
   inc vcnt
;### fifteen.atl(73) 	else if vcnt = 2
   jmp _lbl2
_lbl3:
   lda vcnt
   cmp #2
   jne _lbl4
;### fifteen.atl(75) 		player_col2(3) = $4c
   sta WSYNC
   lda #76
   sta player_col2+3
;### fifteen.atl(77) 		player_col2(3) = $4a
   sta WSYNC
   lda #74
   sta player_col2+3
;### fifteen.atl(79) 		player_col2(3) = $48
   sta WSYNC
   lda #72
   sta player_col2+3
;### fifteen.atl(81) 		player_col2(3) = $46
   sta WSYNC
   lda #70
   sta player_col2+3
;### fifteen.atl(83) 		player_col2(3) = $44
   sta WSYNC
   lda #68
   sta player_col2+3
;### fifteen.atl(85) 		player_col2(3) = $42
   sta WSYNC
   lda #66
   sta player_col2+3
;### fifteen.atl(86) 		inc vcnt
   inc vcnt
;### fifteen.atl(87) 	else if vcnt = 3
   jmp _lbl2
_lbl4:
   lda vcnt
   cmp #3
   jne _lbl5
;### fifteen.atl(89) 		player_col2(3) = $56
   sta WSYNC
   lda #86
   sta player_col2+3
;### fifteen.atl(90) 		COLPF(3) = $56
   lda #86
   sta COLPF+3
;### fifteen.atl(92) 		COLPF(3) = $84
   sta WSYNC
   lda #132
   sta COLPF+3
;### fifteen.atl(93) 		player_col2(3) = $86
   lda #134
   sta player_col2+3
;### fifteen.atl(95) 		COLPF(3) = $94
   sta WSYNC
   lda #148
   sta COLPF+3
;### fifteen.atl(96) 		player_col2(3) = $96
   lda #150
   sta player_col2+3
;### fifteen.atl(97) 		inc vcnt
   inc vcnt
;### fifteen.atl(98) 	else if vcnt = 4
   jmp _lbl2
_lbl5:
   lda vcnt
   cmp #4
   jne _lbl6
;### fifteen.atl(100) 		COLPF(3) = $9a
   sta WSYNC
   lda #154
   sta COLPF+3
;### fifteen.atl(101) 		player_col2(3) = $9c
   lda #156
   sta player_col2+3
;### fifteen.atl(102) 		inc vcnt
   inc vcnt
;### fifteen.atl(103) 		inc vcnt
_lbl6:
;### fifteen.atl(103) 		inc vcnt
_lbl2:
   pla
   tay
   pla
   tax
   pla
   rti
.endp
sleep .proc
;### fifteen.atl(169) 	tmp = timer
   lda timer
   sta drawmainscreen__tmp
;### fifteen.atl(170) 	tmp = tmp + time
   lda drawmainscreen__tmp
   clc
   adc sleep__time
   sta drawmainscreen__tmp
;### fifteen.atl(173) 	while timer<>tmp
   jmp _lbl7
;### fifteen.atl(173) 	while timer<>tmp
_lbl9:
;### fifteen.atl(173) 	while timer<>tmp
_lbl7:
;### fifteen.atl(171) 	while timer<>tmp
   lda timer
   cmp drawmainscreen__tmp
   jeq _lbl8
;### fifteen.atl(173) 	while timer<>tmp
   jmp _lbl9
;### fifteen.atl(173) 	while timer<>tmp
_lbl8:
   rts
.endp
copyblock .proc
;### fifteen.atl(175) 	bptr(0)=srcbm(0)
   lda copyblock__srcbm
   sta bptr
;### fifteen.atl(176) 	bptr(1)=srcbm(1)
   lda copyblock__srcbm+1
   sta bptr+1
;### fifteen.atl(177) 	i=0
   lda #0
   sta i
;### fifteen.atl(197) 		inc i
   jmp _lbl10
;### fifteen.atl(197) 		inc i
_lbl14:
;### fifteen.atl(180) 		xbtmp=i
   lda i
   sta xbtmp
;### fifteen.atl(181) 		xbtmp=xbtmp+xscr
   lda xbtmp
   clc
   adc copyblock__xscr
   sta xbtmp
;### fifteen.atl(182) 		ybtmp=yscr
   lda copyblock__yscr
   sta ybtmp
;### fifteen.atl(184) 		xctmp=xbm+i
   lda copyblock__xbm
   clc
   adc i
   sta xctmp
   lda #0
   rol
   sta xctmp+1
;### fifteen.atl(185) 		yctmp=ybm
   lda copyblock__ybm
   sta yctmp
   lda copyblock__ybm+1
   sta yctmp+1
;### fifteen.atl(187) 		ctmp1 = bptr
   lda bptr
   sta ctmp1
   lda bptr+1
   sta ctmp1+1
;### fifteen.atl(188) 		ctmp1= ctmp1+ xctmp
   lda ctmp1
   clc
   adc xctmp+0
   sta ctmp1
   lda ctmp1+1
   adc xctmp+1
   sta ctmp1+1
;### fifteen.atl(189) 		ctmp1= ctmp1+ yctmp
   lda ctmp1
   clc
   adc yctmp+0
   sta ctmp1
   lda ctmp1+1
   adc yctmp+1
   sta ctmp1+1
;### fifteen.atl(195) 		inc i
   lda #1
   sta _s2__j
_lbl13:
;### fifteen.atl(191) 			screen(xbtmp,ybtmp)=mem(ctmp1)
   lda #<mem
   sta _arr
   lda #>mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda (_arr),y
   sta _98
   ldy ybtmp
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy xbtmp
   lda _98
   sta (_arr),y
;### fifteen.atl(192) 			ctmp1 = ctmp1 + wbm
   lda ctmp1
   clc
   adc copyblock__wbm
   sta ctmp1
   jcc _lbl140
   inc ctmp1+1
_lbl140:
;### fifteen.atl(193) 			inc ybtmp
   inc ybtmp
   inc _s2__j
   lda _s2__j
   cmp copyblock__ysize
   jcc _lbl13
   jeq _lbl13
   inc i
;### fifteen.atl(197) 		inc i
_lbl10:
;### fifteen.atl(178) 	while i<xsize 
   lda i
   cmp copyblock__xsize
   jcs _lbl11
;### fifteen.atl(197) 		inc i
   jmp _lbl14
;### fifteen.atl(197) 		inc i
_lbl11:
   rts
.endp
text .proc
;### fifteen.atl(199) 	ii:byte=0
   lda #0
   sta text__ii
;### fifteen.atl(207) 		inc ii
   jmp _lbl15
;### fifteen.atl(207) 		inc ii
_lbl17:
;### fifteen.atl(201) 		ctmp1=textbuf(ii)
   ldx text__ii
   lda textbuf,x
   sta _99
   lda _99
   sta ctmp1
   lda #0
   sta ctmp1+1
;### fifteen.atl(202) 		ctmp1=ctmp1*8
   lda ctmp1
   sta _TEMPW1
   lda ctmp1+1
   sta _TEMPW1+1
   lda #8
   sta _TEMPW2
   lda #0
   sta _TEMPW2+1
   jsr _sys_mul16
   lda _TEMPL1
   sta ctmp1
   lda _TEMPL1+1
   sta ctmp1+1
;### fifteen.atl(203) 		btmp1= xt+ii
   lda text__xt
   clc
   adc text__ii
   sta btmp1
;### fifteen.atl(204) 		copyblock fonts 0 ctmp1 1 btmp1 yt 1 8
   lda #<fonts
   sta _s3___12
   lda #>fonts
   sta _s3___12+1
   lda _s3___12
   sta copyblock__srcbm
   lda _s3___12+1
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   lda ctmp1
   sta copyblock__ybm
   lda ctmp1+1
   sta copyblock__ybm+1
   lda #1
   sta copyblock__wbm
   lda btmp1
   sta copyblock__xscr
   lda text__yt
   sta copyblock__yscr
   lda #1
   sta copyblock__xsize
   lda #8
   sta copyblock__ysize
   jsr copyblock
;### fifteen.atl(205) 		inc ii
   inc text__ii
;### fifteen.atl(207) 		inc ii
_lbl15:
;### fifteen.atl(200) 	while ii<len
   lda text__ii
   cmp text__len
   jcs _lbl16
;### fifteen.atl(207) 		inc ii
   jmp _lbl17
;### fifteen.atl(207) 		inc ii
_lbl16:
   rts
.endp
copytile .proc
;### fifteen.atl(209) 	ctmp1=ybm
   lda copytile__ybm
   sta ctmp1
   lda #0
   sta ctmp1+1
;### fifteen.atl(210) 	ctmp1=ctmp1*48*24
   lda ctmp1
   sta _TEMPW1
   lda ctmp1+1
   sta _TEMPW1+1
   lda #48
   sta _TEMPW2
   lda #0
   sta _TEMPW2+1
   jsr _sys_mul16
   lda _TEMPL1
   sta copytile___13
   lda _TEMPL1+1
   sta copytile___13+1
   lda copytile___13
   sta _TEMPW1
   lda copytile___13+1
   sta _TEMPW1+1
   lda #24
   sta _TEMPW2
   lda #0
   sta _TEMPW2+1
   jsr _sys_mul16
   lda _TEMPL1
   sta ctmp1
   lda _TEMPL1+1
   sta ctmp1+1
;### fifteen.atl(211) 	xbm=xbm*6
   lda copytile__xbm
   asl
   clc
   adc copytile__xbm
   sta copytile__xbm
   asl copytile__xbm
;### fifteen.atl(212) 	copyblock currentbuf xbm ctmp1 24 xscr yscr xsize ysize
   lda currentbuf
   sta copyblock__srcbm
   lda currentbuf+1
   sta copyblock__srcbm+1
   lda copytile__xbm
   sta copyblock__xbm
   lda ctmp1
   sta copyblock__ybm
   lda ctmp1+1
   sta copyblock__ybm+1
   lda #24
   sta copyblock__wbm
   lda copytile__xscr
   sta copyblock__xscr
   lda copytile__yscr
   sta copyblock__yscr
   lda copytile__xsize
   sta copyblock__xsize
   lda copytile__ysize
   sta copyblock__ysize
   jsr copyblock
   rts
.endp
eraserect .proc
;### fifteen.atl(215) 	i=0
   lda #0
   sta i
;### fifteen.atl(225) 		inc i
   jmp _lbl18
;### fifteen.atl(225) 		inc i
_lbl23:
;### fifteen.atl(217) 		xbtmp=i+xscr
   lda i
   clc
   adc eraserect__xscr
   sta xbtmp
;### fifteen.atl(218) 		ybtmp=yscr
   lda eraserect__yscr
   sta ybtmp
;### fifteen.atl(219) 		j=0
   lda #0
   sta j
;### fifteen.atl(224) 		inc i
   jmp _lbl20
_lbl22:
;### fifteen.atl(221) 			screen(xbtmp,ybtmp)=$ff
   ldy ybtmp
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy xbtmp
   lda #255
   sta (_arr),y
;### fifteen.atl(222) 			inc ybtmp
   inc ybtmp
;### fifteen.atl(223) 			inc j
   inc j
_lbl20:
;### fifteen.atl(220) 		while j<height
   lda j
   cmp eraserect__height
   jcs _lbl21
   jmp _lbl22
_lbl21:
   inc i
;### fifteen.atl(225) 		inc i
_lbl18:
;### fifteen.atl(216) 	while i<width
   lda i
   cmp eraserect__width
   jcs _lbl19
;### fifteen.atl(225) 		inc i
   jmp _lbl23
;### fifteen.atl(225) 		inc i
_lbl19:
   rts
.endp
findempty .proc
;### fifteen.atl(227) 	x=0
   lda #0
   sta findempty__x
;### fifteen.atl(228) 	y=0
   lda #0
   sta findempty__y
;### fifteen.atl(235) 				y=l
   lda #0
   sta k
;### fifteen.atl(235) 				y=l
_lbl28:
;### fifteen.atl(235) 				y=l
   lda #0
   sta l
;### fifteen.atl(235) 				y=l
_lbl27:
;### fifteen.atl(231) 			if playfield(k,l) = T_EMPTY
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _100
   lda _100
   cmp #root__T_EMPTY
   jne _lbl26
;### fifteen.atl(232) 				x=k
   lda k
   sta findempty__x
;### fifteen.atl(233) 				y=l
   lda l
   sta findempty__y
;### fifteen.atl(235) 				y=l
_lbl26:
;### fifteen.atl(235) 				y=l
   inc l
;### fifteen.atl(235) 				y=l
   lda l
   cmp #4
   jne _lbl27
;### fifteen.atl(235) 				y=l
   inc k
;### fifteen.atl(235) 				y=l
   lda k
   cmp #4
   jne _lbl28
   rts
.endp
gettile .proc
;### fifteen.atl(237) 	ty = tile and 3
   lda gettile__tile
   and #3
   sta gettile__ty
;### fifteen.atl(238) 	tx = tile / 4
   lda gettile__tile
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #4
   jsr _sys_div8
   sta gettile__tx
;### fifteen.atl(239) 	tx = tx and 3
   lda gettile__tx
   and #3
   sta gettile__tx
   rts
.endp
showboard .proc
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   lda #0
   sta k
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
_lbl34:
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   lda #0
   sta l
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
_lbl33:
;### fifteen.atl(244) 			xbtmp = 6*k +XOFF
   lda k
   asl
   clc
   adc k
   sta _s9___20
   asl _s9___20
   lda _s9___20
   clc
   adc #root__XOFF
   sta xbtmp
;### fifteen.atl(245) 			ybtmp = 48*l +YOFF
   lda l
   asl
   clc
   adc l
   sta _s9___22
   asl _s9___22
   asl _s9___22
   asl _s9___22
   asl _s9___22
   lda _s9___22
   clc
   adc #root__YOFF
   sta ybtmp
;### fifteen.atl(246) 			tile = playfield(k,l)
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _s9__tile
;### fifteen.atl(247) 			if tile <> T_EMPTY
   lda _s9__tile
   cmp #root__T_EMPTY
   jeq _lbl31
;### fifteen.atl(248) 				i,j=gettile tile
   lda _s9__tile
   sta gettile__tile
   jsr gettile
   lda gettile__tx
   sta i
   lda gettile__ty
   sta j
;### fifteen.atl(249) 				copytile i j xbtmp ybtmp 6 48
   lda i
   sta copytile__xbm
   lda j
   sta copytile__ybm
   lda xbtmp
   sta copytile__xscr
   lda ybtmp
   sta copytile__yscr
   lda #6
   sta copytile__xsize
   lda #48
   sta copytile__ysize
   jsr copytile
;### fifteen.atl(250) 			else
   jmp _lbl32
_lbl31:
;### fifteen.atl(251) 				eraserect  xbtmp ybtmp 6 48
   lda xbtmp
   sta eraserect__xscr
   lda ybtmp
   sta eraserect__yscr
   lda #6
   sta eraserect__width
   lda #48
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
_lbl32:
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   inc l
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   lda l
   cmp #4
   jne _lbl33
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   inc k
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   lda k
   cmp #4
   jne _lbl34
   rts
.endp
movetile .proc
;### fifteen.atl(258) 	xoffset=1
   lda #1
   sta movetile__xoffset
;### fifteen.atl(259) 	yoffset=1
   lda #1
   sta movetile__yoffset
;### fifteen.atl(260) 	AUDC1=$22
   lda #34
   sta AUDC1
;### fifteen.atl(261) 	if m = D_UP
   lda movetile__m
   cmp #root__D_UP
   jne _lbl35
;### fifteen.atl(262) 		if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl36
;### fifteen.atl(263) 			yoffset=2
   lda #2
   sta movetile__yoffset
;### fifteen.atl(265) 	if m = D_RIGHT
_lbl36:
_lbl35:
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl37
;### fifteen.atl(266) 		if xs>0
   lda movetile__xs
   cmp #0
   jcc _lbl38
   jeq _lbl38
;### fifteen.atl(267) 			xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(269) 	if m = D_DOWN
_lbl38:
_lbl37:
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl39
;### fifteen.atl(270) 		if ys>0
   lda movetile__ys
   cmp #0
   jcc _lbl40
   jeq _lbl40
;### fifteen.atl(271) 			yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(273) 	if m = D_LEFT
_lbl40:
_lbl39:
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl41
;### fifteen.atl(274) 		if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl42
;### fifteen.atl(275) 			xoffset=2
   lda #2
   sta movetile__xoffset
;### fifteen.atl(277) 	if xoffset <>1 or yoffset <>1
_lbl42:
_lbl41:
   lda movetile__xoffset
   cmp #1
   jeq _lbl43
   jmp _lbl44
_lbl43:
   lda movetile__yoffset
   cmp #1
   jeq _lbl45
_lbl44:
;### fifteen.atl(279) 		xbtmp=xs-1+xoffset
   lda movetile__xs
   sec
   sbc #1
   sta movetile___24
   lda movetile___24
   clc
   adc movetile__xoffset
   sta xbtmp
;### fifteen.atl(280) 		ybtmp=ys-1+yoffset
   lda movetile__ys
   sec
   sbc #1
   sta movetile___26
   lda movetile___26
   clc
   adc movetile__yoffset
   sta ybtmp
;### fifteen.atl(281) 		playfield(xs,ys)=playfield(xbtmp,ybtmp)
   ldy ybtmp
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy xbtmp
   lda (_arr),y
   sta _101
   ldy movetile__ys
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy movetile__xs
   lda _101
   sta (_arr),y
;### fifteen.atl(282) 		playfield(xbtmp,ybtmp)=T_EMPTY
   ldy ybtmp
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy xbtmp
   lda #root__T_EMPTY
   sta (_arr),y
;### fifteen.atl(283) 		inc moves
   inc moves+0
   jne _lbl141
   inc moves+1
_lbl141:
;### fifteen.atl(284) 		if moves > 999 moves=999
   lda moves+1
   cmp #3
   jcc _lbl46
   jne _lbl142
   lda moves
   cmp #231
   jcc _lbl46
_lbl142:
   lda #231
   sta moves
   lda #3
   sta moves+1
;### fifteen.atl(286) 	xoffset=0
_lbl46:
_lbl45:
   lda #0
   sta movetile__xoffset
;### fifteen.atl(287) 	yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(288) 	if show >0
   lda movetile__show
   cmp #0
   jcc _lbl47
   jeq _lbl47
;### fifteen.atl(289) 		z=0
   lda #0
   sta movetile__z
;### fifteen.atl(290) 		if show=2 z=5
   lda movetile__show
   cmp #2
   jne _lbl48
   lda #5
   sta movetile__z
;### fifteen.atl(291) 		while z<6
_lbl48:
;### fifteen.atl(338) 	AUDC1=0
   jmp _lbl49
_lbl61:
;### fifteen.atl(293) 			xbtmp=xs*6+XOFF
   lda movetile__xs
   asl
   clc
   adc movetile__xs
   sta _s10___28
   asl _s10___28
   lda _s10___28
   clc
   adc #root__XOFF
   sta xbtmp
;### fifteen.atl(294) 			ybtmp=ys*48+YOFF
   lda movetile__ys
   ldx #48
   jsr _sys_mul8
   lda _TEMPW2
   sta _s10___30
   lda _TEMPW2+1
   sta _s10___30+1
   lda _s10___30
   clc
   adc #root__YOFF
   sta ybtmp
   lda #0
   adc #0
   sta ybtmp+1
;### fifteen.atl(295) 			AUDF1=z+xs
   lda movetile__z
   clc
   adc movetile__xs
   sta AUDF1
;### fifteen.atl(296) 			if m = D_LEFT
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl51
;### fifteen.atl(297) 				if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl52
;### fifteen.atl(298) 					xoffset=5-z
   lda #5
   sec
   sbc movetile__z
   sta movetile__xoffset
;### fifteen.atl(299) 					yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(300) 					btmp1=xbtmp+xoffset+6
   lda xbtmp
   clc
   adc movetile__xoffset
   sta _s10___34
   lda #0
   rol
   sta _s10___34+1
   lda _s10___34
   clc
   adc #6
   sta btmp1
   lda #0
   adc #0
   sta btmp1+1
;### fifteen.atl(301) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(302) 					eraserect btmp1 btmp2 1 48
   lda btmp1
   sta eraserect__xscr
   lda btmp2
   sta eraserect__yscr
   lda #1
   sta eraserect__width
   lda #48
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(304) 			if m = D_DOWN
_lbl52:
_lbl51:
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl53
;### fifteen.atl(305) 				if ys>0
   lda movetile__ys
   cmp #0
   jcc _lbl54
   jeq _lbl54
;### fifteen.atl(306) 					yoffset=z-5
   lda movetile__z
   sec
   sbc #5
   sta movetile__yoffset
;### fifteen.atl(307) 					yoffset = yoffset * 8
   lda movetile__yoffset
   asl
   asl
   asl
   sta movetile__yoffset
;### fifteen.atl(308) 					xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(309) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(310) 					btmp2=ybtmp-8
   lda ybtmp
   sec
   sbc #8
   sta btmp2
;### fifteen.atl(311) 					btmp2=btmp2+yoffset
   lda btmp2
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(312) 					eraserect btmp1 btmp2 6 8
   lda btmp1
   sta eraserect__xscr
   lda btmp2
   sta eraserect__yscr
   lda #6
   sta eraserect__width
   lda #8
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(313) 			if m = D_RIGHT
_lbl54:
_lbl53:
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl55
;### fifteen.atl(314) 				if xs>0
   lda movetile__xs
   cmp #0
   jcc _lbl56
   jeq _lbl56
;### fifteen.atl(315) 					xoffset=z-5
   lda movetile__z
   sec
   sbc #5
   sta movetile__xoffset
;### fifteen.atl(316) 					yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(317) 					btmp1=xbtmp-1+xoffset
   lda xbtmp
   sec
   sbc #1
   sta _s10___43
   lda _s10___43
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(318) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(319) 					eraserect btmp1 btmp2 1 48
   lda btmp1
   sta eraserect__xscr
   lda btmp2
   sta eraserect__yscr
   lda #1
   sta eraserect__width
   lda #48
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(321) 			if m = D_UP
_lbl56:
_lbl55:
   lda movetile__m
   cmp #root__D_UP
   jne _lbl57
;### fifteen.atl(322) 				if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl58
;### fifteen.atl(323) 					yoffset=5-z
   lda #5
   sec
   sbc movetile__z
   sta movetile__yoffset
;### fifteen.atl(324) 					yoffset = yoffset * 8
   lda movetile__yoffset
   asl
   asl
   asl
   sta movetile__yoffset
;### fifteen.atl(325) 					xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(326) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(327) 					btmp2=ybtmp+yoffset+48 
   lda ybtmp
   clc
   adc movetile__yoffset
   sta _s10___49
   lda #0
   rol
   sta _s10___49+1
   lda _s10___49
   clc
   adc #48
   sta btmp2
   lda #0
   adc #0
   sta btmp2+1
;### fifteen.atl(328) 					eraserect btmp1 btmp2 6 8
   lda btmp1
   sta eraserect__xscr
   lda btmp2
   sta eraserect__yscr
   lda #6
   sta eraserect__width
   lda #8
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(330) 			tile = playfield(xs,ys)
_lbl58:
_lbl57:
   ldy movetile__ys
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy movetile__xs
   lda (_arr),y
   sta movetile__tile
;### fifteen.atl(331) 			if tile <> T_EMPTY
   lda movetile__tile
   cmp #root__T_EMPTY
   jeq _lbl59
;### fifteen.atl(332) 				if show >0
   lda movetile__show
   cmp #0
   jcc _lbl60
   jeq _lbl60
;### fifteen.atl(333) 					k,l = gettile tile
   lda movetile__tile
   sta gettile__tile
   jsr gettile
   lda gettile__tx
   sta k
   lda gettile__ty
   sta l
;### fifteen.atl(334) 					btmp1=xs*6+XOFF+xoffset
   lda movetile__xs
   asl
   clc
   adc movetile__xs
   sta _s10___51
   asl _s10___51
   lda _s10___51
   clc
   adc #root__XOFF
   sta _s10___52
   lda _s10___52
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(335) 					btmp2=ys*48+YOFF+yoffset
   lda movetile__ys
   ldx #48
   jsr _sys_mul8
   lda _TEMPW2
   sta _s10___54
   lda _TEMPW2+1
   sta _s10___54+1
   lda _s10___54
   clc
   adc #root__YOFF
   sta _s10___55
   lda #0
   adc #0
   sta _s10___55+1
   lda _s10___55
   clc
   adc movetile__yoffset
   sta btmp2
   lda #0
   adc #0
   sta btmp2+1
;### fifteen.atl(336) 					copytile k l btmp1 btmp2 6 48
   lda k
   sta copytile__xbm
   lda l
   sta copytile__ybm
   lda btmp1
   sta copytile__xscr
   lda btmp2
   sta copytile__yscr
   lda #6
   sta copytile__xsize
   lda #48
   sta copytile__ysize
   jsr copytile
;### fifteen.atl(337) 			inc z
_lbl60:
_lbl59:
   inc movetile__z
_lbl49:
   lda movetile__z
   cmp #6
   jcs _lbl50
   jmp _lbl61
_lbl50:
_lbl47:
   lda #0
   sta AUDC1
   rts
.endp
clrscr .proc
;### fifteen.atl(342) 	i=0
   lda #0
   sta i
;### fifteen.atl(350) 		inc i
   jmp _lbl62
;### fifteen.atl(350) 		inc i
_lbl67:
;### fifteen.atl(344) 		j=0
   lda #0
   sta j
;### fifteen.atl(348) 		inc i
   jmp _lbl64
_lbl66:
;### fifteen.atl(346) 			screen(i,j)=255
   ldy j
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy i
   lda #255
   sta (_arr),y
;### fifteen.atl(347) 			inc j
   inc j
_lbl64:
;### fifteen.atl(345) 		while j<200
   lda j
   cmp #200
   jcs _lbl65
   jmp _lbl66
_lbl65:
   inc i
;### fifteen.atl(350) 		inc i
_lbl62:
;### fifteen.atl(343) 	while i<40
   lda i
   cmp #40
   jcs _lbl63
;### fifteen.atl(350) 		inc i
   jmp _lbl67
;### fifteen.atl(350) 		inc i
_lbl63:
   rts
.endp
changepicture .proc
;### fifteen.atl(352) 	if piccnt=0
   lda piccnt
   cmp #0
   jne _lbl68
;### fifteen.atl(353) 		currentbuf=buf1
   lda #<buf1
   sta currentbuf
   lda #>buf1
   sta currentbuf+1
;### fifteen.atl(354) 		c=$1c
   lda #28
   sta changepicture__c
;### fifteen.atl(355) 	if piccnt=1
_lbl68:
   lda piccnt
   cmp #1
   jne _lbl69
;### fifteen.atl(356) 		currentbuf=buf2
   lda #<buf2
   sta currentbuf
   lda #>buf2
   sta currentbuf+1
;### fifteen.atl(357) 		c=$0e
   lda #14
   sta changepicture__c
;### fifteen.atl(359) 	if piccnt=2
_lbl69:
   lda piccnt
   cmp #2
   jne _lbl70
;### fifteen.atl(360) 		currentbuf=buf3
   lda #<buf3
   sta currentbuf
   lda #>buf3
   sta currentbuf+1
;### fifteen.atl(361) 		c=$3a
   lda #58
   sta changepicture__c
;### fifteen.atl(362) 	if piccnt=3
_lbl70:
   lda piccnt
   cmp #3
   jne _lbl71
;### fifteen.atl(363) 		currentbuf=buf4
   lda #<buf4
   sta currentbuf
   lda #>buf4
   sta currentbuf+1
;### fifteen.atl(364) 		c=$7a
   lda #122
   sta changepicture__c
;### fifteen.atl(366) 	inc piccnt
_lbl71:
   inc piccnt
;### fifteen.atl(367) 	if piccnt>=4 piccnt=0
   lda piccnt
   cmp #4
   jcc _lbl72
   lda #0
   sta piccnt
;### fifteen.atl(370) 	if piccnt>=4 piccnt=0
_lbl72:
   rts
.endp
countgoodtiles .proc
;### fifteen.atl(372) 	res=0
   lda #0
   sta countgoodtiles__res
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
   lda #0
   sta k
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
_lbl77:
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
   lda #0
   sta l
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
_lbl76:
;### fifteen.atl(375) 			tile=k*4
   lda k
   asl
   asl
   sta _s14__tile
;### fifteen.atl(376) 			tile=tile+l
   lda _s14__tile
   clc
   adc l
   sta _s14__tile
;### fifteen.atl(377) 			if playfield(k,l) = tile inc res
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _102
   lda _102
   cmp _s14__tile
   jne _lbl75
   inc countgoodtiles__res
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
_lbl75:
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
   inc l
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
   lda l
   cmp #4
   jne _lbl76
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
   inc k
;### fifteen.atl(378) 			if playfield(k,l) = tile inc res
   lda k
   cmp #4
   jne _lbl77
   rts
.endp
inittiles .proc
;### fifteen.atl(385) 			playfield(k,l)=tile
   lda #0
   sta k
;### fifteen.atl(385) 			playfield(k,l)=tile
_lbl81:
;### fifteen.atl(385) 			playfield(k,l)=tile
   lda #0
   sta l
;### fifteen.atl(385) 			playfield(k,l)=tile
_lbl80:
;### fifteen.atl(382) 			tile=k*4
   lda k
   asl
   asl
   sta _s16__tile
;### fifteen.atl(383) 			tile=tile+l
   lda _s16__tile
   clc
   adc l
   sta _s16__tile
;### fifteen.atl(384) 			playfield(k,l)=tile
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda _s16__tile
   sta (_arr),y
;### fifteen.atl(385) 			playfield(k,l)=tile
   inc l
;### fifteen.atl(385) 			playfield(k,l)=tile
   lda l
   cmp #4
   jne _lbl80
;### fifteen.atl(385) 			playfield(k,l)=tile
   inc k
;### fifteen.atl(385) 			playfield(k,l)=tile
   lda k
   cmp #4
   jne _lbl81
   rts
.endp
setboardcolor .proc
;### fifteen.atl(387) 	player_color(0)=col
   lda setboardcolor__col
   sta player_color
;### fifteen.atl(388) 	player_color(1)=col
   lda setboardcolor__col
   sta player_color+1
;### fifteen.atl(389) 	player_color(2)=col
   lda setboardcolor__col
   sta player_color+2
   rts
.endp
changecolor .proc
;### fifteen.atl(393) 	btmp1=col and $f
   lda changecolor__col
   and #15
   sta btmp1
;### fifteen.atl(394) 	btmp2=col and $f0
   lda changecolor__col
   and #240
   sta btmp2
;### fifteen.atl(396) 	xbtmp=currcol and $f
   lda currcol
   and #15
   sta xbtmp
;### fifteen.atl(397) 	ybtmp=currcol and $f0
   lda currcol
   and #240
   sta ybtmp
;### fifteen.atl(399) 	if xbtmp < btmp1 ; fade in
   lda xbtmp
   cmp btmp1
   jcs _lbl82
;### fifteen.atl(406) 	else ; fade out
   jmp _lbl83
_lbl85:
;### fifteen.atl(401) 			xbtmp = xbtmp + 2
   lda xbtmp
   clc
   adc #2
   sta xbtmp
;### fifteen.atl(402) 			currcol = btmp2
   lda btmp2
   sta currcol
;### fifteen.atl(403) 			currcol = currcol + xbtmp
   lda currcol
   clc
   adc xbtmp
   sta currcol
;### fifteen.atl(404) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(405) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
_lbl83:
;### fifteen.atl(400) 		while btmp1 > xbtmp
   lda btmp1
   cmp xbtmp
   jcc _lbl84
   jeq _lbl84
   jmp _lbl85
_lbl84:
   jmp _lbl86
_lbl82:
;### fifteen.atl(414) 			sleep 1
   jmp _lbl87
;### fifteen.atl(414) 			sleep 1
_lbl89:
;### fifteen.atl(408) 			xbtmp = xbtmp -2
   lda xbtmp
   sec
   sbc #2
   sta xbtmp
;### fifteen.atl(409) 			currcol = ybtmp
   lda ybtmp
   sta currcol
;### fifteen.atl(410) 			currcol = currcol + xbtmp
   lda currcol
   clc
   adc xbtmp
   sta currcol
;### fifteen.atl(411) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(412) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(414) 			sleep 1
_lbl87:
;### fifteen.atl(407) 		while btmp1 < xbtmp
   lda btmp1
   cmp xbtmp
   jcs _lbl88
;### fifteen.atl(414) 			sleep 1
   jmp _lbl89
;### fifteen.atl(414) 			sleep 1
_lbl88:
;### fifteen.atl(414) 			sleep 1
_lbl86:
   rts
.endp
drawmainscreen .proc
;### fifteen.atl(416) 	copyblock mains 0 0 14 26 0 14 200
   lda #<mains
   sta drawmainscreen___73
   lda #>mains
   sta drawmainscreen___73+1
   lda drawmainscreen___73
   sta copyblock__srcbm
   lda drawmainscreen___73+1
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   lda #0
   sta copyblock__ybm
   lda #0
   sta copyblock__ybm+1
   lda #14
   sta copyblock__wbm
   lda #26
   sta copyblock__xscr
   lda #0
   sta copyblock__yscr
   lda #14
   sta copyblock__xsize
   lda #200
   sta copyblock__ysize
   jsr copyblock
;### fifteen.atl(417) 	j=0
   lda #0
   sta j
;### fifteen.atl(428) 	tmp:byte = changepicture
   lda #0
   sta _s19__i
_lbl91:
;### fifteen.atl(419) 		xbtmp=i*6
   lda _s19__i
   asl
   clc
   adc _s19__i
   sta xbtmp
   asl xbtmp
;### fifteen.atl(420) 		inc xbtmp
   inc xbtmp
;### fifteen.atl(421) 		copyblock leafh 0 0 6 xbtmp 0 6 8
   lda #<leafh
   sta _s19___75
   lda #>leafh
   sta _s19___75+1
   lda _s19___75
   sta copyblock__srcbm
   lda _s19___75+1
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   lda #0
   sta copyblock__ybm
   lda #0
   sta copyblock__ybm+1
   lda #6
   sta copyblock__wbm
   lda xbtmp
   sta copyblock__xscr
   lda #0
   sta copyblock__yscr
   lda #6
   sta copyblock__xsize
   lda #8
   sta copyblock__ysize
   jsr copyblock
;### fifteen.atl(422) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(423) 		copyblock leafv 0 0 1 0 xbtmp 1 48
   lda #<leafv
   sta _s19___77
   lda #>leafv
   sta _s19___77+1
   lda _s19___77
   sta copyblock__srcbm
   lda _s19___77+1
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   lda #0
   sta copyblock__ybm
   lda #0
   sta copyblock__ybm+1
   lda #1
   sta copyblock__wbm
   lda #0
   sta copyblock__xscr
   lda xbtmp
   sta copyblock__yscr
   lda #1
   sta copyblock__xsize
   lda #48
   sta copyblock__ysize
   jsr copyblock
;### fifteen.atl(424) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(425) 		copyblock leafv 0 0 1 25 xbtmp 1 48
   lda #<leafv
   sta _s19___79
   lda #>leafv
   sta _s19___79+1
   lda _s19___79
   sta copyblock__srcbm
   lda _s19___79+1
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   lda #0
   sta copyblock__ybm
   lda #0
   sta copyblock__ybm+1
   lda #1
   sta copyblock__wbm
   lda #25
   sta copyblock__xscr
   lda xbtmp
   sta copyblock__yscr
   lda #1
   sta copyblock__xsize
   lda #48
   sta copyblock__ysize
   jsr copyblock
;### fifteen.atl(426) 		j=j+48
   lda j
   clc
   adc #48
   sta j
   inc _s19__i
   lda _s19__i
   cmp #4
   jne _lbl91
   jsr changepicture
   lda changepicture__c
   sta drawmainscreen__tmp
;### fifteen.atl(429) 	showboard
   jsr showboard
;### fifteen.atl(430) 	currcol=0
   lda #0
   sta currcol
;### fifteen.atl(431) 	changecolor tmp
   lda drawmainscreen__tmp
   sta changecolor__col
   jsr changecolor
   rts
.endp
shuffletiles .proc
;### fifteen.atl(434) 	c:byte=0
   lda #0
   sta shuffletiles__c
;### fifteen.atl(435) 	loop@
loop:
;### fifteen.atl(436) 	dir=RANDOM and 3
   lda RANDOM
   and #3
   sta dir
;### fifteen.atl(437) 	x,y=findempty
   jsr findempty
   lda findempty__x
   sta shuffletiles__x
   lda findempty__y
   sta shuffletiles__y
;### fifteen.atl(438) 	movetile x y dir 2
   lda shuffletiles__x
   sta movetile__xs
   lda shuffletiles__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #2
   sta movetile__show
   jsr movetile
;### fifteen.atl(439) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta shuffletiles__cnt
;### fifteen.atl(440) 	if c<50 inc c
   lda shuffletiles__c
   cmp #50
   jcs _lbl92
   inc shuffletiles__c
;### fifteen.atl(441) 	if cnt>2 or c<50 goto loop
_lbl92:
   lda shuffletiles__cnt
   cmp #2
   jcc _lbl93
   jeq _lbl93
   jmp _lbl94
_lbl93:
   lda shuffletiles__c
   cmp #50
   jcs _lbl95
_lbl94:
   jmp loop
;### fifteen.atl(443) 	if cnt>2 or c<50 goto loop
_lbl95:
   rts
.endp
cycle .proc
;### fifteen.atl(445) 	if play = 1
   lda play
   cmp #1
   jne _lbl96
;### fifteen.atl(446) 		mus_play
   jsr mus_play
;### fifteen.atl(447) 	vcnt=0
_lbl96:
   lda #0
   sta vcnt
   jmp $e462
.endp
fillpmg .proc
;### fifteen.atl(450) 	ctmp1 = $180
   lda #128
   sta ctmp1
   lda #1
   sta ctmp1+1
;### fifteen.atl(456) 	for i:16..111
   jmp _lbl97
_lbl99:
;### fifteen.atl(453) 		pmg_mem(i)=$0
   lda #0
   ldx i
   sta pmg_mem,x
;### fifteen.atl(454) 		inc ctmp1
   inc ctmp1+0
   jne _lbl143
   inc ctmp1+1
_lbl143:
_lbl97:
;### fifteen.atl(452) 	while ctmp1<=$3ff
   lda ctmp1+1
   cmp #3
   jeq _lbl144
   jcs _lbl98
_lbl144:
   lda ctmp1
   cmp #255
   jeq _loc2
   jcs _lbl98
_loc2:
   jmp _lbl99
_lbl98:
;### fifteen.atl(466) 	for i:12..115
   lda #16
   sta _s21__i
_lbl101:
;### fifteen.atl(457) 		ctmp1=$200
   lda #0
   sta ctmp1
   lda #2
   sta ctmp1+1
;### fifteen.atl(458) 		ctmp1 = ctmp1+i
   lda ctmp1
   clc
   adc _s21__i
   sta ctmp1
   jcc _lbl145
   inc ctmp1+1
_lbl145:
;### fifteen.atl(459) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(460) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl146
   inc ctmp1+1
_lbl146:
;### fifteen.atl(461) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(462) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl147
   inc ctmp1+1
_lbl147:
;### fifteen.atl(463) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(464) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl148
   inc ctmp1+1
_lbl148:
   inc _s21__i
   lda _s21__i
   cmp #112
   jne _lbl101
;### fifteen.atl(473) 	for i:75..79
   lda #12
   sta _s22__i
_lbl103:
;### fifteen.atl(467) 		ctmp1=$180
   lda #128
   sta ctmp1
   lda #1
   sta ctmp1+1
;### fifteen.atl(468) 		ctmp1=ctmp1 + i
   lda ctmp1
   clc
   adc _s22__i
   sta ctmp1
   jcc _lbl149
   inc ctmp1+1
_lbl149:
;### fifteen.atl(469) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(470) 		ctmp1=ctmp1+ $200
   lda ctmp1
   clc
   adc #0
   sta ctmp1
   lda ctmp1+1
   adc #2
   sta ctmp1+1
;### fifteen.atl(471) 		pmg_mem(ctmp1)=$fe
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #254
   sta (_arr),y
   inc _s22__i
   lda _s22__i
   cmp #116
   jne _lbl103
;### fifteen.atl(477) 		pmg_mem(ctmp1)=$00
   lda #75
   sta _s23__i
;### fifteen.atl(477) 		pmg_mem(ctmp1)=$00
_lbl105:
;### fifteen.atl(474) 		ctmp1=$380
   lda #128
   sta ctmp1
   lda #3
   sta ctmp1+1
;### fifteen.atl(475) 		ctmp1= ctmp1+ i
   lda ctmp1
   clc
   adc _s23__i
   sta ctmp1
   jcc _lbl150
   inc ctmp1+1
_lbl150:
;### fifteen.atl(476) 		pmg_mem(ctmp1)=$00
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(477) 		pmg_mem(ctmp1)=$00
   inc _s23__i
;### fifteen.atl(477) 		pmg_mem(ctmp1)=$00
   lda _s23__i
   cmp #80
   jne _lbl105
   rts
.endp
windowpmg .proc
;### fifteen.atl(491) 		pmg_mem(ctmp1)=$3f
   lda #0
   sta _s24__i
;### fifteen.atl(491) 		pmg_mem(ctmp1)=$3f
_lbl107:
;### fifteen.atl(484) 		ctmp1=$237
   lda #55
   sta ctmp1
   lda #2
   sta ctmp1+1
;### fifteen.atl(485) 		ctmp1 = ctmp1 +i
   lda ctmp1
   clc
   adc _s24__i
   sta ctmp1
   jcc _lbl151
   inc ctmp1+1
_lbl151:
;### fifteen.atl(486) 		pmg_mem(ctmp1)=$fc
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #252
   sta (_arr),y
;### fifteen.atl(487) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl152
   inc ctmp1+1
_lbl152:
;### fifteen.atl(488) 		pmg_mem(ctmp1)=$0
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(489) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl153
   inc ctmp1+1
_lbl153:
;### fifteen.atl(490) 		pmg_mem(ctmp1)=$3f
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #63
   sta (_arr),y
;### fifteen.atl(491) 		pmg_mem(ctmp1)=$3f
   inc _s24__i
;### fifteen.atl(491) 		pmg_mem(ctmp1)=$3f
   lda _s24__i
   cmp #18
   jne _lbl107
   rts
.endp
setpmg .proc
;### fifteen.atl(494) 	fillpmg
   jsr fillpmg
;### fifteen.atl(496) 	PMBASE= $9c
   lda #156
   sta PMBASE
;### fifteen.atl(497) 	GRACTL=3
   lda #3
   sta GRACTL
;### fifteen.atl(499) 	GTICTLS=$18
   lda #24
   sta GTICTLS
;### fifteen.atl(501) 	player_x(0)=52
   lda #52
   sta player_x
;### fifteen.atl(502) 	player_x(1)=84
   lda #84
   sta player_x+1
;### fifteen.atl(503) 	player_x(2)=116
   lda #116
   sta player_x+2
;### fifteen.atl(504) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(505) 	player_size(0)=3
   lda #3
   sta player_size
;### fifteen.atl(506) 	player_size(1)=3
   lda #3
   sta player_size+1
;### fifteen.atl(507) 	player_size(2)=3
   lda #3
   sta player_size+2
;### fifteen.atl(508) 	player_size(3)=3
   lda #3
   sta player_size+3
;### fifteen.atl(509) 	missile_x(0)=152
   lda #152
   sta missile_x
;### fifteen.atl(510) 	missile_x(1)=160
   lda #160
   sta missile_x+1
;### fifteen.atl(511) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(512) 	missile_x(2)=192
   lda #192
   sta missile_x+2
;### fifteen.atl(513) 	missile_x(3)=200
   lda #200
   sta missile_x+3
;### fifteen.atl(514) 	player_color(3)=$98
   lda #152
   sta player_color+3
;### fifteen.atl(515) 	COLOR0(3)=$96
   lda #150
   sta COLOR0+3
;### fifteen.atl(516) 	missile_size=$ff
   lda #255
   sta missile_size
;### fifteen.atl(517) 	DMACTL =  $2e
   lda #46
   sta DMACTL
   rts
.endp
start_timer .proc
;### fifteen.atl(523) 	timer=0
   lda #0
   sta timer
;### fifteen.atl(524) 	tsec=0
   lda #0
   sta tsec
;### fifteen.atl(525) 	tdsec=0
   lda #0
   sta tdsec
;### fifteen.atl(526) 	tmin=0
   lda #0
   sta tmin
;### fifteen.atl(527) 	tdmin=0
   lda #0
   sta tdmin
   rts
.endp
update_timer .proc
;### fifteen.atl(531) 	if timer>49
   lda timer
   cmp #49
   jcc _lbl108
   jeq _lbl108
;### fifteen.atl(532) 		timer = timer -50
   lda timer
   sec
   sbc #50
   sta timer
;### fifteen.atl(533) 		inc tsec
   inc tsec
;### fifteen.atl(535) 	if tsec=10
_lbl108:
   lda tsec
   cmp #10
   jne _lbl109
;### fifteen.atl(536) 		tsec=0
   lda #0
   sta tsec
;### fifteen.atl(537) 		inc tdsec
   inc tdsec
;### fifteen.atl(539) 	if tdsec=6
_lbl109:
   lda tdsec
   cmp #6
   jne _lbl110
;### fifteen.atl(540) 		tdsec=0
   lda #0
   sta tdsec
;### fifteen.atl(541) 		inc tmin
   inc tmin
;### fifteen.atl(543) 	if tmin=10
_lbl110:
   lda tmin
   cmp #10
   jne _lbl111
;### fifteen.atl(544) 		tmin=0
   lda #0
   sta tmin
;### fifteen.atl(545) 		inc tdmin
   inc tdmin
;### fifteen.atl(547) 	if tdmin=6
_lbl111:
   lda tdmin
   cmp #6
   jne _lbl112
;### fifteen.atl(548) 		tdmin=5
   lda #5
   sta tdmin
;### fifteen.atl(549) 		tmin=9
   lda #9
   sta tmin
;### fifteen.atl(550) 		tdsec=5
   lda #5
   sta tdsec
;### fifteen.atl(551) 		tsec=9
   lda #9
   sta tsec
;### fifteen.atl(552) 		tsec=9
_lbl112:
   rts
.endp
drawbesttime .proc
;### fifteen.atl(554) 	if besttime(0,0) <> 0
   ldy #0
   lda besttime_lo,y
   sta _arr
   lda besttime_hi,y
   sta _arr+1
   ldy #0
   lda (_arr),y
   sta _103
   lda _103
   cmp #0
   jeq _lbl113
;### fifteen.atl(556) 		tcnt=0
   lda #0
   sta drawbesttime__tcnt
;### fifteen.atl(560) 		text 26 181 14
   jmp _lbl114
_lbl116:
;### fifteen.atl(558) 			textbuf(tcnt)=besttime(tcnt)
   ldx drawbesttime__tcnt
   lda besttime,x
   ldx drawbesttime__tcnt
   sta textbuf,x
;### fifteen.atl(559) 			inc tcnt
   inc drawbesttime__tcnt
_lbl114:
;### fifteen.atl(557) 		while tcnt<14
   lda drawbesttime__tcnt
   cmp #14
   jcs _lbl115
   jmp _lbl116
_lbl115:
   lda #26
   sta text__xt
   lda #181
   sta text__yt
   lda #14
   sta text__len
   jsr text
;### fifteen.atl(562) 		textbuf(0,0)="IN [bmoves] MOVES!   "
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(3),c'IN '
   dta b(130),a(bmoves)
   dta b(10),c' MOVES!   '
   dta b(0)
;### fifteen.atl(563) 		text 27 192 13
   lda #27
   sta text__xt
   lda #192
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(565) 		text 27 192 13
_lbl113:
   rts
.endp
initscreen .proc
;### fifteen.atl(568) 	DMACTL=0
   lda #0
   sta DMACTL
;### fifteen.atl(569) 	sdlstl = dl
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
;### fifteen.atl(570) 	currentbuf=buf1
   lda #<buf1
   sta currentbuf
   lda #>buf1
   sta currentbuf+1
;### fifteen.atl(575) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(577) 	setpmg
   jsr setpmg
;### fifteen.atl(578) 	play=0
   lda #0
   sta play
;### fifteen.atl(579) 	on__vbi cycle
   lda #<cycle
   sta initscreen___95
   lda #>cycle
   sta initscreen___95+1
   lda initscreen___95
   sta VVBLKD
   lda initscreen___95+1
   sta VVBLKD+1
;### fifteen.atl(580) 	on__dli set_col
   lda #<set_col
   sta initscreen___96
   lda #>set_col
   sta initscreen___96+1
   lda initscreen___96
   sta VDSLST
   lda initscreen___96+1
   sta VDSLST+1
   lda #192
   sta NMIEN
   rts
.endp
titlescreen .proc
;### fifteen.atl(583) 	DMACTL=0
   lda #0
   sta DMACTL
;### fifteen.atl(584) 	clrscr
   jsr clrscr
;### fifteen.atl(585) 	piccnt = RANDOM and 3
   lda RANDOM
   and #3
   sta piccnt
;### fifteen.atl(586) 	inittiles
   jsr inittiles
;### fifteen.atl(587) 	drawmainscreen
   jsr drawmainscreen
;### fifteen.atl(588) 	fillpmg
   jsr fillpmg
;### fifteen.atl(589) 	DMACTL =  $2e
   lda #46
   sta DMACTL
;### fifteen.atl(591) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(592) 	CH=62
   lda #62
   sta CH
;### fifteen.atl(594) 	COLOR0(1) = $c0
   lda #192
   sta COLOR0+1
;### fifteen.atl(595) 	COLOR0(2) = $c8
   lda #200
   sta COLOR0+2
;### fifteen.atl(596) 	COLOR0(4) = $c0
   lda #192
   sta COLOR0+4
;### fifteen.atl(598) 	drawbesttime
   jsr drawbesttime
;### fifteen.atl(600) 	loop@
loop:
;### fifteen.atl(601) 	if CH = 28  ; ESC 
   lda CH
   cmp #28
   jne _lbl117
;### fifteen.atl(602) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(603) 		ctmp=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta titlescreen__ctmp
;### fifteen.atl(604) 		if ctmp<16
   lda titlescreen__ctmp
   cmp #16
   jcs _lbl118
;### fifteen.atl(605) 			ctmp=currcol
   lda currcol
   sta titlescreen__ctmp
;### fifteen.atl(606) 			changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(607) 			inittiles
   jsr inittiles
;### fifteen.atl(608) 			showboard
   jsr showboard
;### fifteen.atl(609) 			changecolor ctmp
   lda titlescreen__ctmp
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(611) 	if CH=62
_lbl118:
_lbl117:
   lda CH
   cmp #62
   jne _lbl119
;### fifteen.atl(612) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(613) 		if play = 1
   lda play
   cmp #1
   jne _lbl120
;### fifteen.atl(614) 			mus_silence
   jsr mus_silence
;### fifteen.atl(615) 			play = 0
   lda #0
   sta play
;### fifteen.atl(616) 		else
   jmp _lbl121
_lbl120:
;### fifteen.atl(617) 			_x= $0
   ldx #0
;### fifteen.atl(618) 			_y= $9a
   ldy #154
;### fifteen.atl(619) 			mus_init
   jsr mus_init
;### fifteen.atl(620) 			play=1
   lda #1
   sta play
;### fifteen.atl(622) 	if CONSOL = 6
_lbl121:
_lbl119:
   lda CONSOL
   cmp #6
   jne _lbl122
;### fifteen.atl(623) 		shuffletiles
   jsr shuffletiles
;### fifteen.atl(624) 		showboard
   jsr showboard
;### fifteen.atl(625) 		goto loop
   jmp loop
;### fifteen.atl(627) 	if CONSOL = 5
_lbl122:
   lda CONSOL
   cmp #5
   jne _lbl123
;### fifteen.atl(628) 		changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(629) 		btmp1 = changepicture
   jsr changepicture
   lda changepicture__c
   sta btmp1
;### fifteen.atl(630) 		showboard
   jsr showboard
;### fifteen.atl(631) 		changecolor btmp1
   lda btmp1
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(632) 		goto loop
   jmp loop
;### fifteen.atl(635) 	if CONSOL <> 3
_lbl123:
   lda CONSOL
   cmp #3
   jeq _lbl124
;### fifteen.atl(636) 		goto loop
   jmp loop
;### fifteen.atl(637) 		goto loop
_lbl124:
   rts
.endp
game .proc
;### fifteen.atl(641) 	showboard
   jsr showboard
;### fifteen.atl(642) 	play=0
   lda #0
   sta play
;### fifteen.atl(643) 	mus_silence
   jsr mus_silence
;### fifteen.atl(644) 	eraserect 26 140 14 60
   lda #26
   sta eraserect__xscr
   lda #140
   sta eraserect__yscr
   lda #14
   sta eraserect__width
   lda #60
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(646) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(647) 	if cnt=16 shuffletiles showboard
   lda game__cnt
   cmp #16
   jne _lbl125
   jsr shuffletiles
   jsr showboard
;### fifteen.atl(648) 	moves=0
_lbl125:
   lda #0
   sta moves
   lda #0
   sta moves+1
;### fifteen.atl(649) 	start_timer
   jsr start_timer
;### fifteen.atl(651) 	drawbesttime
   jsr drawbesttime
;### fifteen.atl(653) 	gameloop@
gameloop:
;### fifteen.atl(654) 	CH = none
   lda #key__none
   sta CH
;### fifteen.atl(656) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(657) 	textbuf(0,0)="Good tiles:[cnt] "
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(11),c'Good tiles:'
   dta b(129),a(game__cnt)
   dta b(1),c' '
   dta b(0)
;### fifteen.atl(658) 	text 27 141 13
   lda #27
   sta text__xt
   lda #141
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(660) 	update_timer
   jsr update_timer
;### fifteen.atl(661) 	textbuf(0,0)="Time: [tdmin][tmin]:[tdsec][tsec]"
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(6),c'Time: '
   dta b(129),a(tdmin)
   dta b(129),a(tmin)
   dta b(1),c':'
   dta b(129),a(tdsec)
   dta b(129),a(tsec)
   dta b(0)
;### fifteen.atl(662) 	text 27 152 11
   lda #27
   sta text__xt
   lda #152
   sta text__yt
   lda #11
   sta text__len
   jsr text
;### fifteen.atl(664) 	if cnt=16
   lda game__cnt
   cmp #16
   jne _lbl126
;### fifteen.atl(665) 		textbuf(0,0)="            "
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(12),c'            '
   dta b(0)
;### fifteen.atl(666) 		text 7 86 12
   lda #7
   sta text__xt
   lda #86
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(667) 		text 7 102 12
   lda #7
   sta text__xt
   lda #102
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(668) 		text 7 114 12
   lda #7
   sta text__xt
   lda #114
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(669) 		windowpmg
   jsr windowpmg
;### fifteen.atl(670) 		textbuf(0,0)=" Well done! "
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(12),c' Well done! '
   dta b(0)
;### fifteen.atl(671) 		text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(672) 		textbuf(0,0)=" Time:[tdmin][tmin]:[tdsec][tsec] "
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(6),c' Time:'
   dta b(129),a(tdmin)
   dta b(129),a(tmin)
   dta b(1),c':'
   dta b(129),a(tdsec)
   dta b(129),a(tsec)
   dta b(1),c' '
   dta b(0)
;### fifteen.atl(673) 		text 7 106 12
   lda #7
   sta text__xt
   lda #106
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(674) 		if moves<bmoves
   lda moves+1
   cmp bmoves+1
   jeq _lbl154
   jcs _lbl127
_lbl154:
   lda moves
   cmp bmoves
   jcs _lbl127
;### fifteen.atl(675) 			besttime(0,0)="BESTTIME:[tdmin][tmin]:[tdsec][tsec]"
   ldy #0
   lda besttime_lo,y
   clc
   adc #0
   sta _arr
   lda besttime_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(9),c'BESTTIME:'
   dta b(129),a(tdmin)
   dta b(129),a(tmin)
   dta b(1),c':'
   dta b(129),a(tdsec)
   dta b(129),a(tsec)
   dta b(0)
;### fifteen.atl(676) 			sleep 100
   lda #100
   sta sleep__time
   jsr sleep
;### fifteen.atl(677) 			textbuf(0,0)=" NEW RECORD!"
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(12),c' NEW RECORD!'
   dta b(0)
;### fifteen.atl(678) 			text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(679) 			textbuf(0,0)=" [moves] MOVES!    "
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(1),c' '
   dta b(130),a(moves)
   dta b(11),c' MOVES!    '
   dta b(0)
;### fifteen.atl(680) 			text 7 106 12
   lda #7
   sta text__xt
   lda #106
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(681) 			bmoves=moves
   lda moves
   sta bmoves
   lda moves+1
   sta bmoves+1
;### fifteen.atl(682) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(683) 			AUDC1=$68
   lda #104
   sta AUDC1
;### fifteen.atl(690) 			AUDC1=0
   jmp _lbl128
_lbl130:
;### fifteen.atl(685) 				AUDF1=dir
   lda dir
   sta AUDF1
;### fifteen.atl(686) 				setboardcolor RANDOM
   lda RANDOM
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(687) 				sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(688) 				inc dir
   inc dir
_lbl128:
;### fifteen.atl(684) 			while dir<200
   lda dir
   cmp #200
   jcs _lbl129
   jmp _lbl130
_lbl129:
   lda #0
   sta AUDC1
;### fifteen.atl(691) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(692) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(693) 		else
   jmp _lbl131
_lbl127:
;### fifteen.atl(694) 			sleep 200
   lda #200
   sta sleep__time
   jsr sleep
;### fifteen.atl(696) 		CH=28
_lbl131:
   lda #28
   sta CH
;### fifteen.atl(697) 		goto gameend
   jmp gameend
;### fifteen.atl(699) 	textbuf(0,0)="Moves: [moves]     "
_lbl126:
   ldy #0
   lda textbuf_lo,y
   clc
   adc #0
   sta _arr
   lda textbuf_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(7),c'Moves: '
   dta b(130),a(moves)
   dta b(5),c'     '
   dta b(0)
;### fifteen.atl(700) 	text 27 163 12
   lda #27
   sta text__xt
   lda #163
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(702) 	ss:stick__state = STICK(0)
   lda STICK
   sta game__ss
;### fifteen.atl(704) 	dir=0
   lda #0
   sta dir
;### fifteen.atl(705) 	if ss = 14 dir = 1
   lda game__ss
   cmp #14
   jne _lbl132
   lda #1
   sta dir
;### fifteen.atl(706) 	if ss = 7  dir = 2
_lbl132:
   lda game__ss
   cmp #7
   jne _lbl133
   lda #2
   sta dir
;### fifteen.atl(707) 	if ss = 13 dir = 3
_lbl133:
   lda game__ss
   cmp #13
   jne _lbl134
   lda #3
   sta dir
;### fifteen.atl(708) 	if ss = 11 dir = 4
_lbl134:
   lda game__ss
   cmp #11
   jne _lbl135
   lda #4
   sta dir
;### fifteen.atl(709) 	if dir <> 0 
_lbl135:
   lda dir
   cmp #0
   jeq _lbl136
;### fifteen.atl(710) 		x,y=findempty
   jsr findempty
   lda findempty__x
   sta game__x
   lda findempty__y
   sta game__y
;### fifteen.atl(711) 		dec dir
   dec dir
;### fifteen.atl(712) 		movetile x y dir 1
   lda game__x
   sta movetile__xs
   lda game__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #1
   sta movetile__show
   jsr movetile
;### fifteen.atl(713) 		dir=0
   lda #0
   sta dir
;### fifteen.atl(714) 		sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(716) 	if CH <> 28 goto gameloop  ; ESC 
_lbl136:
   lda CH
   cmp #28
   jeq _lbl137
   jmp gameloop
;### fifteen.atl(718) 	gameend@
_lbl137:
gameend:
;### fifteen.atl(720) 	CH = none
   lda #key__none
   sta CH
   rts
.endp
