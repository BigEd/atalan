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
on__vbi__x equ 130
mem equ 0
screen equ 41136
PORTB equ 54017
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
xtilesize equ 1568
ytilesize equ 1569
sleep__time equ 132
copyblock__srcbm equ 133
copyblock__xbm equ 135
copyblock__ybm equ 136
copyblock__wbm equ 138
copyblock__xscr equ 139
copyblock__yscr equ 140
copyblock__xsize equ 141
copyblock__ysize equ 142
_s2__j equ 143
text__xt equ 147
text__yt equ 148
text__len equ 149
text__ii equ 150
_s3___12 equ 151
copytile__xbm equ 153
copytile__ybm equ 154
copytile__xscr equ 155
copytile__yscr equ 156
copytile__xsize equ 157
copytile__ysize equ 158
eraserect__xscr equ 159
eraserect__yscr equ 160
eraserect__width equ 161
eraserect__height equ 162
findempty__x equ 163
findempty__y equ 164
gettile__tile equ 165
gettile__tx equ 166
gettile__ty equ 167
_s9___20 equ 168
_s9___22 equ 169
_s9__tile equ 170
movetile__xs equ 171
movetile__ys equ 172
movetile__m equ 173
movetile__show equ 174
movetile__tile equ 175
movetile__xoffset equ 176
movetile__yoffset equ 177
movetile__z equ 178
_s10___26 equ 179
_s10___28 equ 181
changepicture__c equ 183
countgoodtiles__res equ 184
_s14__tile equ 185
_s16__tile equ 186
setboardcolor__col equ 187
changecolor__col equ 188
drawmainscreen___72 equ 189
_s19__i equ 191
_s19___74 equ 192
_s19___76 equ 194
_s19___78 equ 196
shuffletiles__c equ 198
shuffletiles__x equ 199
shuffletiles__y equ 200
shuffletiles__cnt equ 201
_s21__i equ 202
_s22__i equ 203
_s23__i equ 204
_s24__i equ 205
drawbesttime__tcnt equ 206
initscreen___94 equ 207
initscreen___95 equ 209
titlescreen__ctmp equ 211
game__cnt equ 212
game__ss equ 213
game__x equ 214
game__y equ 215
_98 equ 216
_99 equ 217
_100 equ 218
_101 equ 219
_102 equ 220
_103 equ 221
   org $2e0
   dta a($2000)
   org $2000
;### fifteen.atl(158) piccnt@$612:byte= RANDOM and 3
   lda RANDOM
   and #3
   sta piccnt
;### fifteen.atl(747) PORTB = PORTB or 2
   lda PORTB
   ora #2
   sta PORTB
;### fifteen.atl(748) initscreen
   jsr initscreen
;### fifteen.atl(749) besttime(0)=0
   lda #0
   sta besttime
;### fifteen.atl(750) bmoves=999
   lda #231
   sta bmoves
   lda #3
   sta bmoves+1
;### fifteen.atl(756) 	game
   jmp _lbl144
;### fifteen.atl(756) 	game
_lbl145:
;### fifteen.atl(752) 	titlescreen
   jsr titlescreen
;### fifteen.atl(753) 	game
   jsr game
;### fifteen.atl(756) 	game
_lbl144:
;### fifteen.atl(756) 	game
   jmp _lbl145
_lbl158:
   jmp _lbl158
dl:
;### fifteen.atl(29) const dl:array of byte = ( 2 times $70, $4f,  screen ,
   dta b(112)
   dta b(112)
   dta b(79)
   dta a(screen)
;### fifteen.atl(30) 	32 times $f,
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
;### fifteen.atl(31) 	$8f,
   dta b(143)
;### fifteen.atl(32) 	18 times $f
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
;### fifteen.atl(33) 	$8f,
   dta b(143)
;### fifteen.atl(34) 	12 times $f ,
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
;### fifteen.atl(35) 	$8f,
   dta b(143)
;### fifteen.atl(36) 	29 times $f ,
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
;### fifteen.atl(37) 	$8f
   dta b(143)
;### fifteen.atl(38) 	$f $f
   dta b(15)
   dta b(15)
;### fifteen.atl(39) 	$4f , 0, $B0 ,
   dta b(79)
   dta b(0)
   dta b(176)
;### fifteen.atl(40) 	40  times $f,
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
;### fifteen.atl(41) 	$8f
   dta b(143)
;### fifteen.atl(42) 	60  times $f,
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
;### fifteen.atl(43) 	$4f screen
   dta b(79)
   dta a(screen)
;### fifteen.atl(44) 	7 times $f
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
   dta b(15)
;### fifteen.atl(45) 	$41, dl
   dta b(65)
   dta a(dl)
buf1:
;### fifteen.atl(130) const buf1: array(23, 191) = file "einstein-tongue.pbm"
   ins 'einstein-tongue.pbm'
buf2:
;### fifteen.atl(131) const buf2: array(23, 191) = file "miner.pbm"
   ins 'miner.pbm'
buf3:
;### fifteen.atl(132) const buf3: array(23, 191) = file "johnromero.pbm"
   ins 'johnromero.pbm'
buf4:
;### fifteen.atl(133) const buf4: array(23, 191) = file "clinteastwood.pbm"
   ins 'clinteastwood.pbm'
leafh:
;### fifteen.atl(134) const leafh: array(47) = file "leafhoriz.pbm"
   ins 'leafhoriz.pbm'
leafv:
;### fifteen.atl(135) const leafv: array(47) = file "leafvert.pbm"
   ins 'leafvert.pbm'
mains:
;### fifteen.atl(136) const mains: array(2800) = file "15.pbm"
   ins '15.pbm'
fonts:
;### fifteen.atl(137) const fonts: array(1024) = file "STENCIL.FNT"
   ins 'STENCIL.FNT'
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
;### fifteen.atl(51) 	if vcnt = 0
   lda vcnt
   cmp #0
   jne _lbl1
;### fifteen.atl(52) 		inc vcnt
   inc vcnt
;### fifteen.atl(54) 		player_col2(3) = $9e
   sta WSYNC
   lda #158
   sta player_col2+3
;### fifteen.atl(55) 		COLPF(3) = $96
   lda #150
   sta COLPF+3
;### fifteen.atl(56) 		player_x(3)=180
   lda #180
   sta player_x+3
;### fifteen.atl(57) 		missile_x(2)=166
   lda #166
   sta missile_x+2
;### fifteen.atl(58) 		missile_x(3)=172
   lda #172
   sta missile_x+3
;### fifteen.atl(60) 		player_col2(3) = $8e
   sta WSYNC
   lda #142
   sta player_col2+3
;### fifteen.atl(61) 		COLPF(3) = $86
   lda #134
   sta COLPF+3
;### fifteen.atl(63) 		player_col2(3) = $5e
   sta WSYNC
   lda #94
   sta player_col2+3
;### fifteen.atl(64) 		COLPF(3) = $56
   lda #86
   sta COLPF+3
;### fifteen.atl(66) 		player_col2(3) = $4e
   sta WSYNC
   lda #78
   sta player_col2+3
;### fifteen.atl(67) 		COLPF(3) = $46
   lda #70
   sta COLPF+3
;### fifteen.atl(68) 	else if vcnt = 1
   jmp _lbl2
_lbl1:
   lda vcnt
   cmp #1
   jne _lbl3
;### fifteen.atl(70) 		COLPF(3) = $4e
   sta WSYNC
   lda #78
   sta COLPF+3
;### fifteen.atl(71) 		player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(72) 		missile_x(2)=192
   lda #192
   sta missile_x+2
;### fifteen.atl(73) 		missile_x(3)=200
   lda #200
   sta missile_x+3
;### fifteen.atl(74) 		inc vcnt
   inc vcnt
;### fifteen.atl(75) 	else if vcnt = 2
   jmp _lbl2
_lbl3:
   lda vcnt
   cmp #2
   jne _lbl4
;### fifteen.atl(77) 		player_col2(3) = $4c
   sta WSYNC
   lda #76
   sta player_col2+3
;### fifteen.atl(79) 		player_col2(3) = $4a
   sta WSYNC
   lda #74
   sta player_col2+3
;### fifteen.atl(81) 		player_col2(3) = $48
   sta WSYNC
   lda #72
   sta player_col2+3
;### fifteen.atl(83) 		player_col2(3) = $46
   sta WSYNC
   lda #70
   sta player_col2+3
;### fifteen.atl(85) 		player_col2(3) = $44
   sta WSYNC
   lda #68
   sta player_col2+3
;### fifteen.atl(87) 		player_col2(3) = $42
   sta WSYNC
   lda #66
   sta player_col2+3
;### fifteen.atl(88) 		inc vcnt
   inc vcnt
;### fifteen.atl(89) 	else if vcnt = 3
   jmp _lbl2
_lbl4:
   lda vcnt
   cmp #3
   jne _lbl5
;### fifteen.atl(91) 		player_col2(3) = $56
   sta WSYNC
   lda #86
   sta player_col2+3
;### fifteen.atl(92) 		COLPF(3) = $56
   lda #86
   sta COLPF+3
;### fifteen.atl(94) 		COLPF(3) = $84
   sta WSYNC
   lda #132
   sta COLPF+3
;### fifteen.atl(95) 		player_col2(3) = $86
   lda #134
   sta player_col2+3
;### fifteen.atl(97) 		COLPF(3) = $94
   sta WSYNC
   lda #148
   sta COLPF+3
;### fifteen.atl(98) 		player_col2(3) = $96
   lda #150
   sta player_col2+3
;### fifteen.atl(99) 		inc vcnt
   inc vcnt
;### fifteen.atl(100) 	else if vcnt = 4
   jmp _lbl2
_lbl5:
   lda vcnt
   cmp #4
   jne _lbl6
;### fifteen.atl(102) 		COLPF(3) = $9a
   sta WSYNC
   lda #154
   sta COLPF+3
;### fifteen.atl(103) 		player_col2(3) = $9c
   lda #156
   sta player_col2+3
;### fifteen.atl(104) 		inc vcnt
   inc vcnt
;### fifteen.atl(105) 		inc vcnt
_lbl6:
;### fifteen.atl(105) 		inc vcnt
_lbl2:
   pla
   tay
   pla
   tax
   pla
   rti
.endp
sleep .proc
;### fifteen.atl(173) 	tmp = timer
   lda timer
   sta drawmainscreen__tmp
;### fifteen.atl(174) 	tmp = tmp + time
   lda drawmainscreen__tmp
   clc
   adc sleep__time
   sta drawmainscreen__tmp
;### fifteen.atl(177) 	while timer<>tmp
   jmp _lbl7
;### fifteen.atl(177) 	while timer<>tmp
_lbl9:
;### fifteen.atl(177) 	while timer<>tmp
_lbl7:
;### fifteen.atl(175) 	while timer<>tmp
   lda timer
   cmp drawmainscreen__tmp
   jeq _lbl8
;### fifteen.atl(177) 	while timer<>tmp
   jmp _lbl9
;### fifteen.atl(177) 	while timer<>tmp
_lbl8:
   rts
.endp
copyblock .proc
;### fifteen.atl(179) 	bptr(0)=srcbm(0)
   lda copyblock__srcbm
   sta bptr
;### fifteen.atl(180) 	bptr(1)=srcbm(1)
   lda copyblock__srcbm+1
   sta bptr+1
;### fifteen.atl(181) 	i=0
   lda #0
   sta i
;### fifteen.atl(201) 		inc i
   jmp _lbl10
;### fifteen.atl(201) 		inc i
_lbl14:
;### fifteen.atl(184) 		xbtmp=i
   lda i
   sta xbtmp
;### fifteen.atl(185) 		xbtmp=xbtmp+xscr
   lda xbtmp
   clc
   adc copyblock__xscr
   sta xbtmp
;### fifteen.atl(186) 		ybtmp=yscr
   lda copyblock__yscr
   sta ybtmp
;### fifteen.atl(188) 		xctmp=xbm+i
   lda copyblock__xbm
   clc
   adc i
   sta xctmp
   lda #0
   rol
   sta xctmp+1
;### fifteen.atl(189) 		yctmp=ybm
   lda copyblock__ybm
   sta yctmp
   lda copyblock__ybm+1
   sta yctmp+1
;### fifteen.atl(191) 		ctmp1 = bptr
   lda bptr
   sta ctmp1
   lda bptr+1
   sta ctmp1+1
;### fifteen.atl(192) 		ctmp1= ctmp1+ xctmp
   lda ctmp1
   clc
   adc xctmp+0
   sta ctmp1
   lda ctmp1+1
   adc xctmp+1
   sta ctmp1+1
;### fifteen.atl(193) 		ctmp1= ctmp1+ yctmp
   lda ctmp1
   clc
   adc yctmp+0
   sta ctmp1
   lda ctmp1+1
   adc yctmp+1
   sta ctmp1+1
;### fifteen.atl(199) 		inc i
   lda #1
   sta _s2__j
_lbl13:
;### fifteen.atl(195) 			screen(xbtmp,ybtmp)=mem(ctmp1)
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
;### fifteen.atl(196) 			ctmp1 = ctmp1 + wbm
   lda ctmp1
   clc
   adc copyblock__wbm
   sta ctmp1
   jcc _lbl146
   inc ctmp1+1
_lbl146:
;### fifteen.atl(197) 			inc ybtmp
   inc ybtmp
   inc _s2__j
   lda copyblock__ysize
   cmp _s2__j
   jcs _lbl13
   inc i
;### fifteen.atl(201) 		inc i
_lbl10:
;### fifteen.atl(182) 	while i<xsize 
   lda i
   cmp copyblock__xsize
   jcs _lbl11
;### fifteen.atl(201) 		inc i
   jmp _lbl14
;### fifteen.atl(201) 		inc i
_lbl11:
   rts
.endp
text .proc
;### fifteen.atl(203) 	ii:byte=0
   lda #0
   sta text__ii
;### fifteen.atl(211) 		inc ii
   jmp _lbl15
;### fifteen.atl(211) 		inc ii
_lbl17:
;### fifteen.atl(205) 		ctmp1=textbuf(ii)
   ldx text__ii
   lda textbuf,x
   sta _99
   lda _99
   sta ctmp1
   lda #0
   sta ctmp1+1
;### fifteen.atl(206) 		ctmp1=ctmp1*8
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
;### fifteen.atl(207) 		btmp1= xt+ii
   lda text__xt
   clc
   adc text__ii
   sta btmp1
;### fifteen.atl(208) 		copyblock fonts 0 ctmp1 1 btmp1 yt 1 8
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
;### fifteen.atl(209) 		inc ii
   inc text__ii
;### fifteen.atl(211) 		inc ii
_lbl15:
;### fifteen.atl(204) 	while ii<len
   lda text__ii
   cmp text__len
   jcs _lbl16
;### fifteen.atl(211) 		inc ii
   jmp _lbl17
;### fifteen.atl(211) 		inc ii
_lbl16:
   rts
.endp
copytile .proc
;### fifteen.atl(213) 	ctmp1=ybm*24
   lda copytile__ybm
   ldx #24
   jsr _sys_mul8
   lda _TEMPW2
   sta ctmp1
   lda _TEMPW2+1
   sta ctmp1+1
;### fifteen.atl(214) 	ctmp1=ctmp1*48
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
   sta ctmp1
   lda _TEMPL1+1
   sta ctmp1+1
;### fifteen.atl(215) 	xbm=xbm*6
   lda copytile__xbm
   asl
   clc
   adc copytile__xbm
   asl
   sta copytile__xbm
;### fifteen.atl(216) 	copyblock currentbuf xbm ctmp1 24 xscr yscr xsize ysize
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
;### fifteen.atl(219) 	i=0
   lda #0
   sta i
;### fifteen.atl(229) 		inc i
   jmp _lbl18
;### fifteen.atl(229) 		inc i
_lbl23:
;### fifteen.atl(221) 		xbtmp=i+xscr
   lda i
   clc
   adc eraserect__xscr
   sta xbtmp
;### fifteen.atl(222) 		ybtmp=yscr
   lda eraserect__yscr
   sta ybtmp
;### fifteen.atl(223) 		j=0
   lda #0
   sta j
;### fifteen.atl(228) 		inc i
   jmp _lbl20
_lbl22:
;### fifteen.atl(225) 			screen(xbtmp,ybtmp)=$ff
   ldy ybtmp
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy xbtmp
   lda #255
   sta (_arr),y
;### fifteen.atl(226) 			inc ybtmp
   inc ybtmp
;### fifteen.atl(227) 			inc j
   inc j
_lbl20:
;### fifteen.atl(224) 		while j<height
   lda j
   cmp eraserect__height
   jcs _lbl21
   jmp _lbl22
_lbl21:
   inc i
;### fifteen.atl(229) 		inc i
_lbl18:
;### fifteen.atl(220) 	while i<width
   lda i
   cmp eraserect__width
   jcs _lbl19
;### fifteen.atl(229) 		inc i
   jmp _lbl23
;### fifteen.atl(229) 		inc i
_lbl19:
   rts
.endp
findempty .proc
;### fifteen.atl(231) 	x=0
   lda #0
   sta findempty__x
;### fifteen.atl(232) 	y=0
   lda #0
   sta findempty__y
;### fifteen.atl(239) 				y=l
   lda #0
   sta k
;### fifteen.atl(239) 				y=l
_lbl28:
;### fifteen.atl(239) 				y=l
   lda #0
   sta l
;### fifteen.atl(239) 				y=l
_lbl27:
;### fifteen.atl(235) 			if playfield(k,l) = T_EMPTY
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
;### fifteen.atl(236) 				x=k
   lda k
   sta findempty__x
;### fifteen.atl(237) 				y=l
   lda l
   sta findempty__y
;### fifteen.atl(239) 				y=l
_lbl26:
;### fifteen.atl(239) 				y=l
   inc l
;### fifteen.atl(239) 				y=l
   lda l
   cmp #4
   jne _lbl27
;### fifteen.atl(239) 				y=l
   inc k
;### fifteen.atl(239) 				y=l
   lda k
   cmp #4
   jne _lbl28
   rts
.endp
gettile .proc
;### fifteen.atl(241) 	ty = tile and 3
   lda gettile__tile
   and #3
   sta gettile__ty
;### fifteen.atl(242) 	tx = tile / 4
   lda gettile__tile
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #4
   jsr _sys_div8
   sta gettile__tx
;### fifteen.atl(243) 	tx = tx and 3
   lda gettile__tx
   and #3
   sta gettile__tx
   rts
.endp
showboard .proc
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
   lda #0
   sta k
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
_lbl34:
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
   lda #0
   sta l
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
_lbl33:
;### fifteen.atl(248) 			xbtmp = 6*k+XOFF
   lda k
   asl
   clc
   adc k
   asl
   sta _s9___20
   lda _s9___20
   clc
   adc #root__XOFF
   sta xbtmp
;### fifteen.atl(249) 			ybtmp = 48*l+YOFF
   lda l
   asl
   clc
   adc l
   asl
   asl
   asl
   asl
   sta _s9___22
   lda _s9___22
   clc
   adc #root__YOFF
   sta ybtmp
;### fifteen.atl(250) 			tile = playfield(k,l)
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _s9__tile
;### fifteen.atl(251) 			if tile <> T_EMPTY
   lda _s9__tile
   cmp #root__T_EMPTY
   jeq _lbl31
;### fifteen.atl(252) 				i,j=gettile tile
   lda _s9__tile
   sta gettile__tile
   jsr gettile
   lda gettile__tx
   sta i
   lda gettile__ty
   sta j
;### fifteen.atl(253) 				copytile i j xbtmp ybtmp 6 48
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
;### fifteen.atl(254) 			else
   jmp _lbl32
_lbl31:
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   lda xbtmp
   sta eraserect__xscr
   lda ybtmp
   sta eraserect__yscr
   lda #6
   sta eraserect__width
   lda #48
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
_lbl32:
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
   inc l
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
   lda l
   cmp #4
   jne _lbl33
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
   inc k
;### fifteen.atl(257) 				eraserect  xbtmp ybtmp 6 48
   lda k
   cmp #4
   jne _lbl34
   rts
.endp
movetile .proc
;### fifteen.atl(262) 	xoffset=1
   lda #1
   sta movetile__xoffset
;### fifteen.atl(263) 	yoffset=1
   lda #1
   sta movetile__yoffset
;### fifteen.atl(264) 	AUDC1=$22
   lda #34
   sta AUDC1
;### fifteen.atl(265) 	if m = D_UP
   lda movetile__m
   cmp #root__D_UP
   jne _lbl35
;### fifteen.atl(266) 		if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl36
;### fifteen.atl(267) 			yoffset=2
   lda #2
   sta movetile__yoffset
;### fifteen.atl(269) 	if m = D_RIGHT
_lbl36:
_lbl35:
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl37
;### fifteen.atl(270) 		if xs>0
   lda #0
   cmp movetile__xs
   jcs _lbl38
;### fifteen.atl(271) 			xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(273) 	if m = D_DOWN
_lbl38:
_lbl37:
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl39
;### fifteen.atl(274) 		if ys>0
   lda #0
   cmp movetile__ys
   jcs _lbl40
;### fifteen.atl(275) 			yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(277) 	if m = D_LEFT
_lbl40:
_lbl39:
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl41
;### fifteen.atl(278) 		if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl42
;### fifteen.atl(279) 			xoffset=2
   lda #2
   sta movetile__xoffset
;### fifteen.atl(281) 	if xoffset <>1 or yoffset <>1
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
;### fifteen.atl(283) 		xbtmp=xs+xoffset
   lda movetile__xs
   clc
   adc movetile__xoffset
   sta xbtmp
;### fifteen.atl(284) 		dec xbtmp
   dec xbtmp
;### fifteen.atl(285) 		ybtmp=ys+yoffset
   lda movetile__ys
   clc
   adc movetile__yoffset
   sta ybtmp
;### fifteen.atl(286) 		dec ybtmp
   dec ybtmp
;### fifteen.atl(287) 		playfield(xs,ys)=playfield(xbtmp,ybtmp)
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
;### fifteen.atl(288) 		playfield(xbtmp,ybtmp)=T_EMPTY
   ldy ybtmp
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy xbtmp
   lda #root__T_EMPTY
   sta (_arr),y
;### fifteen.atl(289) 		inc moves
   inc moves+0
   jne _lbl147
   inc moves+1
_lbl147:
;### fifteen.atl(290) 		if moves > 999 moves=999
   lda #231
   cmp moves
   lda #3
   sbc moves+1
   jcs _lbl46
   lda #231
   sta moves
   lda #3
   sta moves+1
;### fifteen.atl(292) 	xoffset=0
_lbl46:
_lbl45:
   lda #0
   sta movetile__xoffset
;### fifteen.atl(293) 	yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(294) 	if show >0
   lda #0
   cmp movetile__show
   jcs _lbl47
;### fifteen.atl(295) 		z=0
   lda #0
   sta movetile__z
;### fifteen.atl(296) 		if show=2 z=5
   lda movetile__show
   cmp #2
   jne _lbl48
   lda #5
   sta movetile__z
;### fifteen.atl(297) 		while z<6
_lbl48:
;### fifteen.atl(361) 	AUDC1=0
   jmp _lbl49
_lbl67:
;### fifteen.atl(299) 			xbtmp=xs*6+XOFF
   lda movetile__xs
   ldx #6
   jsr _sys_mul8
   lda _TEMPW2
   sta _s10___26
   lda _TEMPW2+1
   sta _s10___26+1
   lda _s10___26
   clc
   adc #root__XOFF
   sta xbtmp
;### fifteen.atl(300) 			ybtmp=ys*48+YOFF
   lda movetile__ys
   ldx #48
   jsr _sys_mul8
   lda _TEMPW2
   sta _s10___28
   lda _TEMPW2+1
   sta _s10___28+1
   lda _s10___28
   clc
   adc #root__YOFF
   sta ybtmp
;### fifteen.atl(301) 			AUDF1=z+xs
   lda movetile__z
   clc
   adc movetile__xs
   sta AUDF1
;### fifteen.atl(302) 			xtilesize=6
   lda #6
   sta xtilesize
;### fifteen.atl(303) 			ytilesize=48
   lda #48
   sta ytilesize
;### fifteen.atl(305) 			if m = D_LEFT
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl51
;### fifteen.atl(306) 				if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl52
;### fifteen.atl(307) 					xoffset=5-z
   lda #5
   sec
   sbc movetile__z
   sta movetile__xoffset
;### fifteen.atl(308) 					yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(309) 					btmp1 = xbtmp+6
   lda xbtmp
   clc
   adc #6
   sta btmp1
;### fifteen.atl(310) 					btmp1 = btmp1+xoffset
   lda btmp1
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(311) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(312) 					xtilesize=1
   lda #1
   sta xtilesize
;### fifteen.atl(314) 			if m = D_DOWN
_lbl52:
_lbl51:
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl53
;### fifteen.atl(315) 				if ys>0
   lda #0
   cmp movetile__ys
   jcs _lbl54
;### fifteen.atl(316) 					yoffset=z-5
   lda movetile__z
   sec
   sbc #5
   sta movetile__yoffset
;### fifteen.atl(317) 					yoffset = yoffset * 8
   lda movetile__yoffset
   asl
   asl
   asl
   sta movetile__yoffset
;### fifteen.atl(318) 					xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(319) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(320) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(321) 					btmp2=btmp2-8
   lda btmp2
   sec
   sbc #8
   sta btmp2
;### fifteen.atl(322) 					if z=5 btmp2 = btmp2-40
   lda movetile__z
   cmp #5
   jne _lbl55
   lda btmp2
   sec
   sbc #40
   sta btmp2
;### fifteen.atl(323) 					ytilesize=8
_lbl55:
   lda #8
   sta ytilesize
;### fifteen.atl(324) 			if m = D_RIGHT
_lbl54:
_lbl53:
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl56
;### fifteen.atl(325) 				if xs>0
   lda #0
   cmp movetile__xs
   jcs _lbl57
;### fifteen.atl(326) 					xoffset=z-5
   lda movetile__z
   sec
   sbc #5
   sta movetile__xoffset
;### fifteen.atl(327) 					yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(328) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(329) 					dec btmp1
   dec btmp1
;### fifteen.atl(330) 					if z=5 btmp1 = btmp1-5
   lda movetile__z
   cmp #5
   jne _lbl58
   lda btmp1
   sec
   sbc #5
   sta btmp1
;### fifteen.atl(331) 					btmp2=ybtmp+yoffset
_lbl58:
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(332) 					xtilesize=1
   lda #1
   sta xtilesize
;### fifteen.atl(334) 			if m = D_UP
_lbl57:
_lbl56:
   lda movetile__m
   cmp #root__D_UP
   jne _lbl59
;### fifteen.atl(335) 				if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl60
;### fifteen.atl(336) 					yoffset=5-z
   lda #5
   sec
   sbc movetile__z
   sta movetile__yoffset
;### fifteen.atl(337) 					yoffset = yoffset * 8
   lda movetile__yoffset
   asl
   asl
   asl
   sta movetile__yoffset
;### fifteen.atl(338) 					xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(339) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(340) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(341) 					btmp2=btmp2+48
   lda btmp2
   clc
   adc #48
   sta btmp2
;### fifteen.atl(342) 					ytilesize=8
   lda #8
   sta ytilesize
;### fifteen.atl(344) 			if xtilesize<>6 or ytilesize<>48
_lbl60:
_lbl59:
   lda xtilesize
   cmp #6
   jeq _lbl61
   jmp _lbl62
_lbl61:
   lda ytilesize
   cmp #48
   jeq _lbl63
_lbl62:
;### fifteen.atl(345) 				if z=5 xtilesize=6 ytilesize=48 
   lda movetile__z
   cmp #5
   jne _lbl64
   lda #6
   sta xtilesize
   lda #48
   sta ytilesize
;### fifteen.atl(346) 				eraserect btmp1 btmp2 xtilesize ytilesize
_lbl64:
   lda btmp1
   sta eraserect__xscr
   lda btmp2
   sta eraserect__yscr
   lda xtilesize
   sta eraserect__width
   lda ytilesize
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(349) 			tile = playfield(xs,ys)
_lbl63:
   ldy movetile__ys
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy movetile__xs
   lda (_arr),y
   sta movetile__tile
;### fifteen.atl(350) 			if tile <> T_EMPTY
   lda movetile__tile
   cmp #root__T_EMPTY
   jeq _lbl65
;### fifteen.atl(351) 				if show >0
   lda #0
   cmp movetile__show
   jcs _lbl66
;### fifteen.atl(352) 					k,l = gettile tile
   lda movetile__tile
   sta gettile__tile
   jsr gettile
   lda gettile__tx
   sta k
   lda gettile__ty
   sta l
;### fifteen.atl(353) 					btmp1=xs*6
   lda movetile__xs
   asl
   clc
   adc movetile__xs
   asl
   sta btmp1
;### fifteen.atl(354) 					btmp1=btmp1+XOFF
   inc btmp1
;### fifteen.atl(355) 					btmp1=btmp1+xoffset
   lda btmp1
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(356) 					btmp2=ys*48
   lda movetile__ys
   asl
   clc
   adc movetile__ys
   asl
   asl
   asl
   asl
   sta btmp2
;### fifteen.atl(357) 					btmp2=btmp2+YOFF
   lda btmp2
   clc
   adc #root__YOFF
   sta btmp2
;### fifteen.atl(358) 					btmp2=btmp2+yoffset
   lda btmp2
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(359) 					copytile k l btmp1 btmp2 6 48
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
;### fifteen.atl(360) 			inc z
_lbl66:
_lbl65:
   inc movetile__z
_lbl49:
   lda movetile__z
   cmp #6
   jcs _lbl50
   jmp _lbl67
_lbl50:
_lbl47:
   lda #0
   sta AUDC1
   rts
.endp
clrscr .proc
;### fifteen.atl(365) 	i=0
   lda #0
   sta i
;### fifteen.atl(373) 		inc i
   jmp _lbl68
;### fifteen.atl(373) 		inc i
_lbl73:
;### fifteen.atl(367) 		j=0
   lda #0
   sta j
;### fifteen.atl(371) 		inc i
   jmp _lbl70
_lbl72:
;### fifteen.atl(369) 			screen(i,j)=255
   ldy j
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy i
   lda #255
   sta (_arr),y
;### fifteen.atl(370) 			inc j
   inc j
_lbl70:
;### fifteen.atl(368) 		while j<200
   lda j
   cmp #200
   jcs _lbl71
   jmp _lbl72
_lbl71:
   inc i
;### fifteen.atl(373) 		inc i
_lbl68:
;### fifteen.atl(366) 	while i<40
   lda i
   cmp #40
   jcs _lbl69
;### fifteen.atl(373) 		inc i
   jmp _lbl73
;### fifteen.atl(373) 		inc i
_lbl69:
   rts
.endp
changepicture .proc
;### fifteen.atl(375) 	if piccnt=0
   lda piccnt
   cmp #0
   jne _lbl74
;### fifteen.atl(376) 		currentbuf=buf1
   lda #<buf1
   sta currentbuf
   lda #>buf1
   sta currentbuf+1
;### fifteen.atl(377) 		c=$1c
   lda #28
   sta changepicture__c
;### fifteen.atl(378) 	if piccnt=1
_lbl74:
   lda piccnt
   cmp #1
   jne _lbl75
;### fifteen.atl(379) 		currentbuf=buf2
   lda #<buf2
   sta currentbuf
   lda #>buf2
   sta currentbuf+1
;### fifteen.atl(380) 		c=$0e
   lda #14
   sta changepicture__c
;### fifteen.atl(382) 	if piccnt=2
_lbl75:
   lda piccnt
   cmp #2
   jne _lbl76
;### fifteen.atl(383) 		currentbuf=buf3
   lda #<buf3
   sta currentbuf
   lda #>buf3
   sta currentbuf+1
;### fifteen.atl(384) 		c=$3a
   lda #58
   sta changepicture__c
;### fifteen.atl(385) 	if piccnt=3
_lbl76:
   lda piccnt
   cmp #3
   jne _lbl77
;### fifteen.atl(386) 		currentbuf=buf4
   lda #<buf4
   sta currentbuf
   lda #>buf4
   sta currentbuf+1
;### fifteen.atl(387) 		c=$7a
   lda #122
   sta changepicture__c
;### fifteen.atl(389) 	inc piccnt
_lbl77:
   inc piccnt
;### fifteen.atl(390) 	if piccnt>=4 piccnt=0
   lda piccnt
   cmp #4
   jcc _lbl78
   lda #0
   sta piccnt
;### fifteen.atl(393) 	if piccnt>=4 piccnt=0
_lbl78:
   rts
.endp
countgoodtiles .proc
;### fifteen.atl(395) 	res=0
   lda #0
   sta countgoodtiles__res
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
   lda #0
   sta k
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
_lbl83:
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
   lda #0
   sta l
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
_lbl82:
;### fifteen.atl(398) 			tile=k*4
   lda k
   asl
   asl
   sta _s14__tile
;### fifteen.atl(399) 			tile=tile+l
   lda _s14__tile
   clc
   adc l
   sta _s14__tile
;### fifteen.atl(400) 			if playfield(k,l) = tile inc res
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
   jne _lbl81
   inc countgoodtiles__res
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
_lbl81:
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
   inc l
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
   lda l
   cmp #4
   jne _lbl82
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
   inc k
;### fifteen.atl(401) 			if playfield(k,l) = tile inc res
   lda k
   cmp #4
   jne _lbl83
   rts
.endp
inittiles .proc
;### fifteen.atl(408) 			playfield(k,l)=tile
   lda #0
   sta k
;### fifteen.atl(408) 			playfield(k,l)=tile
_lbl87:
;### fifteen.atl(408) 			playfield(k,l)=tile
   lda #0
   sta l
;### fifteen.atl(408) 			playfield(k,l)=tile
_lbl86:
;### fifteen.atl(405) 			tile=k*4
   lda k
   asl
   asl
   sta _s16__tile
;### fifteen.atl(406) 			tile=tile+l
   lda _s16__tile
   clc
   adc l
   sta _s16__tile
;### fifteen.atl(407) 			playfield(k,l)=tile
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda _s16__tile
   sta (_arr),y
;### fifteen.atl(408) 			playfield(k,l)=tile
   inc l
;### fifteen.atl(408) 			playfield(k,l)=tile
   lda l
   cmp #4
   jne _lbl86
;### fifteen.atl(408) 			playfield(k,l)=tile
   inc k
;### fifteen.atl(408) 			playfield(k,l)=tile
   lda k
   cmp #4
   jne _lbl87
   rts
.endp
setboardcolor .proc
;### fifteen.atl(410) 	player_color(0)=col
   lda setboardcolor__col
   sta player_color
;### fifteen.atl(411) 	player_color(1)=col
   lda setboardcolor__col
   sta player_color+1
;### fifteen.atl(412) 	player_color(2)=col
   lda setboardcolor__col
   sta player_color+2
   rts
.endp
changecolor .proc
;### fifteen.atl(416) 	btmp1=col and $f
   lda changecolor__col
   and #15
   sta btmp1
;### fifteen.atl(417) 	btmp2=col and $f0
   lda changecolor__col
   and #240
   sta btmp2
;### fifteen.atl(419) 	xbtmp=currcol and $f
   lda currcol
   and #15
   sta xbtmp
;### fifteen.atl(420) 	ybtmp=currcol and $f0
   lda currcol
   and #240
   sta ybtmp
;### fifteen.atl(422) 	if xbtmp < btmp1 ; fade in
   lda xbtmp
   cmp btmp1
   jcs _lbl88
;### fifteen.atl(429) 	else ; fade out
   jmp _lbl89
_lbl91:
;### fifteen.atl(424) 			xbtmp = xbtmp + 2
   lda xbtmp
   clc
   adc #2
   sta xbtmp
;### fifteen.atl(425) 			currcol = btmp2
   lda btmp2
   sta currcol
;### fifteen.atl(426) 			currcol = currcol + xbtmp
   lda currcol
   clc
   adc xbtmp
   sta currcol
;### fifteen.atl(427) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(428) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
_lbl89:
;### fifteen.atl(423) 		while btmp1 > xbtmp
   lda xbtmp
   cmp btmp1
   jcs _lbl90
   jmp _lbl91
_lbl90:
   jmp _lbl92
_lbl88:
;### fifteen.atl(437) 			sleep 1
   jmp _lbl93
;### fifteen.atl(437) 			sleep 1
_lbl95:
;### fifteen.atl(431) 			xbtmp = xbtmp -2
   lda xbtmp
   sec
   sbc #2
   sta xbtmp
;### fifteen.atl(432) 			currcol = ybtmp
   lda ybtmp
   sta currcol
;### fifteen.atl(433) 			currcol = currcol + xbtmp
   lda currcol
   clc
   adc xbtmp
   sta currcol
;### fifteen.atl(434) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(435) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(437) 			sleep 1
_lbl93:
;### fifteen.atl(430) 		while btmp1 < xbtmp
   lda btmp1
   cmp xbtmp
   jcs _lbl94
;### fifteen.atl(437) 			sleep 1
   jmp _lbl95
;### fifteen.atl(437) 			sleep 1
_lbl94:
;### fifteen.atl(437) 			sleep 1
_lbl92:
   rts
.endp
drawmainscreen .proc
;### fifteen.atl(439) 	copyblock mains 0 0 14 26 0 14 200
   lda #<mains
   sta drawmainscreen___72
   lda #>mains
   sta drawmainscreen___72+1
   lda drawmainscreen___72
   sta copyblock__srcbm
   lda drawmainscreen___72+1
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
;### fifteen.atl(440) 	j=0
   lda #0
   sta j
;### fifteen.atl(451) 	tmp:byte = changepicture
   lda #0
   sta _s19__i
_lbl97:
;### fifteen.atl(442) 		xbtmp=i*6
   lda _s19__i
   asl
   clc
   adc _s19__i
   asl
   sta xbtmp
;### fifteen.atl(443) 		inc xbtmp
   inc xbtmp
;### fifteen.atl(444) 		copyblock leafh 0 0 6 xbtmp 0 6 8
   lda #<leafh
   sta _s19___74
   lda #>leafh
   sta _s19___74+1
   lda _s19___74
   sta copyblock__srcbm
   lda _s19___74+1
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
;### fifteen.atl(445) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(446) 		copyblock leafv 0 0 1 0 xbtmp 1 48
   lda #<leafv
   sta _s19___76
   lda #>leafv
   sta _s19___76+1
   lda _s19___76
   sta copyblock__srcbm
   lda _s19___76+1
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
;### fifteen.atl(447) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(448) 		copyblock leafv 0 0 1 25 xbtmp 1 48
   lda #<leafv
   sta _s19___78
   lda #>leafv
   sta _s19___78+1
   lda _s19___78
   sta copyblock__srcbm
   lda _s19___78+1
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
;### fifteen.atl(449) 		j=j+48
   lda j
   clc
   adc #48
   sta j
   inc _s19__i
   lda _s19__i
   cmp #4
   jne _lbl97
   jsr changepicture
   lda changepicture__c
   sta drawmainscreen__tmp
;### fifteen.atl(452) 	showboard
   jsr showboard
;### fifteen.atl(453) 	currcol=0
   lda #0
   sta currcol
;### fifteen.atl(454) 	changecolor tmp
   lda drawmainscreen__tmp
   sta changecolor__col
   jsr changecolor
   rts
.endp
shuffletiles .proc
;### fifteen.atl(457) 	c:byte=0
   lda #0
   sta shuffletiles__c
;### fifteen.atl(458) 	loop@
loop:
;### fifteen.atl(459) 	dir=RANDOM and 3
   lda RANDOM
   and #3
   sta dir
;### fifteen.atl(460) 	x,y=findempty
   jsr findempty
   lda findempty__x
   sta shuffletiles__x
   lda findempty__y
   sta shuffletiles__y
;### fifteen.atl(461) 	movetile x y dir 2
   lda shuffletiles__x
   sta movetile__xs
   lda shuffletiles__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #2
   sta movetile__show
   jsr movetile
;### fifteen.atl(462) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta shuffletiles__cnt
;### fifteen.atl(463) 	if c<50 inc c
   lda shuffletiles__c
   cmp #50
   jcs _lbl98
   inc shuffletiles__c
;### fifteen.atl(464) 	if cnt>0 or c<50 goto loop
_lbl98:
   lda #0
   cmp shuffletiles__cnt
   jcs _lbl99
   jmp _lbl100
_lbl99:
   lda shuffletiles__c
   cmp #50
   jcs _lbl101
_lbl100:
   jmp loop
;### fifteen.atl(466) 	if cnt>0 or c<50 goto loop
_lbl101:
   rts
.endp
cycle .proc
;### fifteen.atl(468) 	if play = 1
   lda play
   cmp #1
   jne _lbl102
;### fifteen.atl(469) 		mus_play
   jsr mus_play
;### fifteen.atl(470) 	vcnt=0
_lbl102:
   lda #0
   sta vcnt
   jmp $e462
.endp
fillpmg .proc
;### fifteen.atl(473) 	ctmp1 = $180
   lda #128
   sta ctmp1
   lda #1
   sta ctmp1+1
;### fifteen.atl(479) 	for i:16..111
   jmp _lbl103
_lbl105:
;### fifteen.atl(476) 		pmg_mem(i)=$0
   lda #0
   ldx i
   sta pmg_mem,x
;### fifteen.atl(477) 		inc ctmp1
   inc ctmp1+0
   jne _lbl148
   inc ctmp1+1
_lbl148:
_lbl103:
;### fifteen.atl(475) 	while ctmp1<=$3ff
   lda #255
   cmp ctmp1
   lda #3
   sbc ctmp1+1
   jcc _lbl104
   jmp _lbl105
_lbl104:
;### fifteen.atl(489) 	for i:12..115
   lda #16
   sta _s21__i
_lbl107:
;### fifteen.atl(480) 		ctmp1=$200
   lda #0
   sta ctmp1
   lda #2
   sta ctmp1+1
;### fifteen.atl(481) 		ctmp1 = ctmp1+i
   lda ctmp1
   clc
   adc _s21__i
   sta ctmp1
   jcc _lbl149
   inc ctmp1+1
_lbl149:
;### fifteen.atl(482) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(483) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl150
   inc ctmp1+1
_lbl150:
;### fifteen.atl(484) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(485) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl151
   inc ctmp1+1
_lbl151:
;### fifteen.atl(486) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(487) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl152
   inc ctmp1+1
_lbl152:
   inc _s21__i
   lda _s21__i
   cmp #112
   jne _lbl107
;### fifteen.atl(496) 	for i:75..79
   lda #12
   sta _s22__i
_lbl109:
;### fifteen.atl(490) 		ctmp1=$180
   lda #128
   sta ctmp1
   lda #1
   sta ctmp1+1
;### fifteen.atl(491) 		ctmp1=ctmp1 + i
   lda ctmp1
   clc
   adc _s22__i
   sta ctmp1
   jcc _lbl153
   inc ctmp1+1
_lbl153:
;### fifteen.atl(492) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(493) 		ctmp1=ctmp1+ $200
   lda ctmp1
   clc
   adc #0
   sta ctmp1
   lda ctmp1+1
   adc #2
   sta ctmp1+1
;### fifteen.atl(494) 		pmg_mem(ctmp1)=$fe
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
   jne _lbl109
;### fifteen.atl(500) 		pmg_mem(ctmp1)=$00
   lda #75
   sta _s23__i
;### fifteen.atl(500) 		pmg_mem(ctmp1)=$00
_lbl111:
;### fifteen.atl(497) 		ctmp1=$380
   lda #128
   sta ctmp1
   lda #3
   sta ctmp1+1
;### fifteen.atl(498) 		ctmp1= ctmp1+ i
   lda ctmp1
   clc
   adc _s23__i
   sta ctmp1
   jcc _lbl154
   inc ctmp1+1
_lbl154:
;### fifteen.atl(499) 		pmg_mem(ctmp1)=$00
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(500) 		pmg_mem(ctmp1)=$00
   inc _s23__i
;### fifteen.atl(500) 		pmg_mem(ctmp1)=$00
   lda _s23__i
   cmp #80
   jne _lbl111
   rts
.endp
windowpmg .proc
;### fifteen.atl(514) 		pmg_mem(ctmp1)=$3f
   lda #0
   sta _s24__i
;### fifteen.atl(514) 		pmg_mem(ctmp1)=$3f
_lbl113:
;### fifteen.atl(507) 		ctmp1=$237
   lda #55
   sta ctmp1
   lda #2
   sta ctmp1+1
;### fifteen.atl(508) 		ctmp1 = ctmp1 +i
   lda ctmp1
   clc
   adc _s24__i
   sta ctmp1
   jcc _lbl155
   inc ctmp1+1
_lbl155:
;### fifteen.atl(509) 		pmg_mem(ctmp1)=$fc
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #252
   sta (_arr),y
;### fifteen.atl(510) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl156
   inc ctmp1+1
_lbl156:
;### fifteen.atl(511) 		pmg_mem(ctmp1)=$0
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(512) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl157
   inc ctmp1+1
_lbl157:
;### fifteen.atl(513) 		pmg_mem(ctmp1)=$3f
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #63
   sta (_arr),y
;### fifteen.atl(514) 		pmg_mem(ctmp1)=$3f
   inc _s24__i
;### fifteen.atl(514) 		pmg_mem(ctmp1)=$3f
   lda _s24__i
   cmp #18
   jne _lbl113
   rts
.endp
setpmg .proc
;### fifteen.atl(517) 	fillpmg
   jsr fillpmg
;### fifteen.atl(519) 	PMBASE= $9c
   lda #156
   sta PMBASE
;### fifteen.atl(520) 	GRACTL=3
   lda #3
   sta GRACTL
;### fifteen.atl(522) 	GTICTLS=$18
   lda #24
   sta GTICTLS
;### fifteen.atl(524) 	player_x(0)=52
   lda #52
   sta player_x
;### fifteen.atl(525) 	player_x(1)=84
   lda #84
   sta player_x+1
;### fifteen.atl(526) 	player_x(2)=116
   lda #116
   sta player_x+2
;### fifteen.atl(527) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(528) 	player_size(0)=3
   lda #3
   sta player_size
;### fifteen.atl(529) 	player_size(1)=3
   lda #3
   sta player_size+1
;### fifteen.atl(530) 	player_size(2)=3
   lda #3
   sta player_size+2
;### fifteen.atl(531) 	player_size(3)=3
   lda #3
   sta player_size+3
;### fifteen.atl(532) 	missile_x(0)=152
   lda #152
   sta missile_x
;### fifteen.atl(533) 	missile_x(1)=160
   lda #160
   sta missile_x+1
;### fifteen.atl(534) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(535) 	missile_x(2)=192
   lda #192
   sta missile_x+2
;### fifteen.atl(536) 	missile_x(3)=200
   lda #200
   sta missile_x+3
;### fifteen.atl(537) 	player_color(3)=$98
   lda #152
   sta player_color+3
;### fifteen.atl(538) 	COLOR0(3)=$96
   lda #150
   sta COLOR0+3
;### fifteen.atl(539) 	missile_size=$ff
   lda #255
   sta missile_size
;### fifteen.atl(540) 	DMACTL =  $2e
   lda #46
   sta DMACTL
   rts
.endp
start_timer .proc
;### fifteen.atl(546) 	timer=0
   lda #0
   sta timer
;### fifteen.atl(547) 	tsec=0
   lda #0
   sta tsec
;### fifteen.atl(548) 	tdsec=0
   lda #0
   sta tdsec
;### fifteen.atl(549) 	tmin=0
   lda #0
   sta tmin
;### fifteen.atl(550) 	tdmin=0
   lda #0
   sta tdmin
   rts
.endp
update_timer .proc
;### fifteen.atl(554) 	if timer>49
   lda #49
   cmp timer
   jcs _lbl114
;### fifteen.atl(555) 		timer = timer -50
   lda timer
   sec
   sbc #50
   sta timer
;### fifteen.atl(556) 		inc tsec
   inc tsec
;### fifteen.atl(558) 	if tsec=10
_lbl114:
   lda tsec
   cmp #10
   jne _lbl115
;### fifteen.atl(559) 		tsec=0
   lda #0
   sta tsec
;### fifteen.atl(560) 		inc tdsec
   inc tdsec
;### fifteen.atl(562) 	if tdsec=6
_lbl115:
   lda tdsec
   cmp #6
   jne _lbl116
;### fifteen.atl(563) 		tdsec=0
   lda #0
   sta tdsec
;### fifteen.atl(564) 		inc tmin
   inc tmin
;### fifteen.atl(566) 	if tmin=10
_lbl116:
   lda tmin
   cmp #10
   jne _lbl117
;### fifteen.atl(567) 		tmin=0
   lda #0
   sta tmin
;### fifteen.atl(568) 		inc tdmin
   inc tdmin
;### fifteen.atl(570) 	if tdmin=6
_lbl117:
   lda tdmin
   cmp #6
   jne _lbl118
;### fifteen.atl(571) 		tdmin=5
   lda #5
   sta tdmin
;### fifteen.atl(572) 		tmin=9
   lda #9
   sta tmin
;### fifteen.atl(573) 		tdsec=5
   lda #5
   sta tdsec
;### fifteen.atl(574) 		tsec=9
   lda #9
   sta tsec
;### fifteen.atl(575) 		tsec=9
_lbl118:
   rts
.endp
drawbesttime .proc
;### fifteen.atl(577) 	if besttime(0,0) <> 0
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
   jeq _lbl119
;### fifteen.atl(579) 		tcnt=0
   lda #0
   sta drawbesttime__tcnt
;### fifteen.atl(583) 		text 26 181 14
   jmp _lbl120
_lbl122:
;### fifteen.atl(581) 			textbuf(tcnt)=besttime(tcnt)
   ldx drawbesttime__tcnt
   lda besttime,x
   ldx drawbesttime__tcnt
   sta textbuf,x
;### fifteen.atl(582) 			inc tcnt
   inc drawbesttime__tcnt
_lbl120:
;### fifteen.atl(580) 		while tcnt<14
   lda drawbesttime__tcnt
   cmp #14
   jcs _lbl121
   jmp _lbl122
_lbl121:
   lda #26
   sta text__xt
   lda #181
   sta text__yt
   lda #14
   sta text__len
   jsr text
;### fifteen.atl(585) 		textbuf(0,0)="IN [bmoves] MOVES!   "
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
;### fifteen.atl(586) 		text 27 192 13
   lda #27
   sta text__xt
   lda #192
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(588) 		text 27 192 13
_lbl119:
   rts
.endp
initscreen .proc
;### fifteen.atl(591) 	DMACTL=0
   lda #0
   sta DMACTL
;### fifteen.atl(592) 	sdlstl = dl
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
;### fifteen.atl(593) 	currentbuf=buf1
   lda #<buf1
   sta currentbuf
   lda #>buf1
   sta currentbuf+1
;### fifteen.atl(598) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(600) 	setpmg
   jsr setpmg
;### fifteen.atl(601) 	play=0
   lda #0
   sta play
;### fifteen.atl(602) 	on__vbi cycle
   lda #<cycle
   sta initscreen___94
   lda #>cycle
   sta initscreen___94+1
   lda initscreen___94
   sta VVBLKD
   lda initscreen___94+1
   sta VVBLKD+1
;### fifteen.atl(603) 	on__dli set_col
   lda #<set_col
   sta initscreen___95
   lda #>set_col
   sta initscreen___95+1
   lda initscreen___95
   sta VDSLST
   lda initscreen___95+1
   sta VDSLST+1
   lda #192
   sta NMIEN
   rts
.endp
titlescreen .proc
;### fifteen.atl(606) 	DMACTL=0
   lda #0
   sta DMACTL
;### fifteen.atl(607) 	clrscr
   jsr clrscr
;### fifteen.atl(608) 	piccnt = RANDOM and 3
   lda RANDOM
   and #3
   sta piccnt
;### fifteen.atl(609) 	inittiles
   jsr inittiles
;### fifteen.atl(610) 	drawmainscreen
   jsr drawmainscreen
;### fifteen.atl(611) 	fillpmg
   jsr fillpmg
;### fifteen.atl(612) 	DMACTL =  $2e
   lda #46
   sta DMACTL
;### fifteen.atl(614) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(615) 	CH=62
   lda #62
   sta CH
;### fifteen.atl(617) 	COLOR0(1) = $c0
   lda #192
   sta COLOR0+1
;### fifteen.atl(618) 	COLOR0(2) = $c8
   lda #200
   sta COLOR0+2
;### fifteen.atl(619) 	COLOR0(4) = $c0
   lda #192
   sta COLOR0+4
;### fifteen.atl(621) 	drawbesttime
   jsr drawbesttime
;### fifteen.atl(623) 	loop@
loop:
;### fifteen.atl(624) 	if CH = 28  ; ESC 
   lda CH
   cmp #28
   jne _lbl123
;### fifteen.atl(625) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(626) 		ctmp=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta titlescreen__ctmp
;### fifteen.atl(627) 		if ctmp<16
   lda titlescreen__ctmp
   cmp #16
   jcs _lbl124
;### fifteen.atl(628) 			ctmp=currcol
   lda currcol
   sta titlescreen__ctmp
;### fifteen.atl(629) 			changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(630) 			inittiles
   jsr inittiles
;### fifteen.atl(631) 			showboard
   jsr showboard
;### fifteen.atl(632) 			changecolor ctmp
   lda titlescreen__ctmp
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(634) 	if CH=62
_lbl124:
_lbl123:
   lda CH
   cmp #62
   jne _lbl125
;### fifteen.atl(635) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(636) 		if play = 1
   lda play
   cmp #1
   jne _lbl126
;### fifteen.atl(637) 			mus_silence
   jsr mus_silence
;### fifteen.atl(638) 			play = 0
   lda #0
   sta play
;### fifteen.atl(639) 		else
   jmp _lbl127
_lbl126:
;### fifteen.atl(640) 			_x= $0
   ldx #0
;### fifteen.atl(641) 			_y= $9a
   ldy #154
;### fifteen.atl(642) 			mus_init
   jsr mus_init
;### fifteen.atl(643) 			play=1
   lda #1
   sta play
;### fifteen.atl(645) 	if CONSOL = 3
_lbl127:
_lbl125:
   lda CONSOL
   cmp #3
   jne _lbl128
;### fifteen.atl(646) 		shuffletiles
   jsr shuffletiles
;### fifteen.atl(647) 		showboard
   jsr showboard
;### fifteen.atl(648) 		goto loop
   jmp loop
;### fifteen.atl(650) 	if CONSOL = 5
_lbl128:
   lda CONSOL
   cmp #5
   jne _lbl129
;### fifteen.atl(651) 		changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(652) 		btmp1 = changepicture
   jsr changepicture
   lda changepicture__c
   sta btmp1
;### fifteen.atl(653) 		showboard
   jsr showboard
;### fifteen.atl(654) 		changecolor btmp1
   lda btmp1
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(655) 		goto loop
   jmp loop
;### fifteen.atl(658) 	if CONSOL <> 6
_lbl129:
   lda CONSOL
   cmp #6
   jeq _lbl130
;### fifteen.atl(659) 		goto loop
   jmp loop
;### fifteen.atl(660) 		goto loop
_lbl130:
   rts
.endp
game .proc
;### fifteen.atl(664) 	showboard
   jsr showboard
;### fifteen.atl(665) 	play=0
   lda #0
   sta play
;### fifteen.atl(666) 	mus_silence
   jsr mus_silence
;### fifteen.atl(667) 	eraserect 26 140 14 60
   lda #26
   sta eraserect__xscr
   lda #140
   sta eraserect__yscr
   lda #14
   sta eraserect__width
   lda #60
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(669) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(670) 	if cnt=16 shuffletiles showboard
   lda game__cnt
   cmp #16
   jne _lbl131
   jsr shuffletiles
   jsr showboard
;### fifteen.atl(671) 	moves=0
_lbl131:
   lda #0
   sta moves
   lda #0
   sta moves+1
;### fifteen.atl(672) 	start_timer
   jsr start_timer
;### fifteen.atl(674) 	drawbesttime
   jsr drawbesttime
;### fifteen.atl(676) 	gameloop@
gameloop:
;### fifteen.atl(677) 	CH = none
   lda #key__none
   sta CH
;### fifteen.atl(679) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(680) 	textbuf(0,0)="Good tiles:[cnt] "
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
;### fifteen.atl(681) 	text 27 141 13
   lda #27
   sta text__xt
   lda #141
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(683) 	update_timer
   jsr update_timer
;### fifteen.atl(684) 	textbuf(0,0)="Time: [tdmin][tmin]:[tdsec][tsec]"
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
;### fifteen.atl(685) 	text 27 152 11
   lda #27
   sta text__xt
   lda #152
   sta text__yt
   lda #11
   sta text__len
   jsr text
;### fifteen.atl(687) 	if cnt=16
   lda game__cnt
   cmp #16
   jne _lbl132
;### fifteen.atl(688) 		textbuf(0,0)="            "
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
;### fifteen.atl(689) 		text 7 86 12
   lda #7
   sta text__xt
   lda #86
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(690) 		text 7 102 12
   lda #7
   sta text__xt
   lda #102
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(691) 		text 7 114 12
   lda #7
   sta text__xt
   lda #114
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(692) 		windowpmg
   jsr windowpmg
;### fifteen.atl(693) 		textbuf(0,0)=" Well done! "
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
;### fifteen.atl(694) 		text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(695) 		textbuf(0,0)=" Time:[tdmin][tmin]:[tdsec][tsec] "
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
;### fifteen.atl(696) 		text 7 106 12
   lda #7
   sta text__xt
   lda #106
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(697) 		if moves<bmoves
   lda moves
   cmp bmoves
   lda moves+1
   sbc bmoves+1
   jcs _lbl133
;### fifteen.atl(698) 			besttime(0,0)="BESTTIME:[tdmin][tmin]:[tdsec][tsec]"
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
;### fifteen.atl(699) 			sleep 100
   lda #100
   sta sleep__time
   jsr sleep
;### fifteen.atl(700) 			textbuf(0,0)=" NEW RECORD!"
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
;### fifteen.atl(701) 			text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(702) 			textbuf(0,0)=" [moves] MOVES!    "
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
;### fifteen.atl(703) 			text 7 106 12
   lda #7
   sta text__xt
   lda #106
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(704) 			bmoves=moves
   lda moves
   sta bmoves
   lda moves+1
   sta bmoves+1
;### fifteen.atl(705) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(706) 			AUDC1=$68
   lda #104
   sta AUDC1
;### fifteen.atl(713) 			AUDC1=0
   jmp _lbl134
_lbl136:
;### fifteen.atl(708) 				AUDF1=dir
   lda dir
   sta AUDF1
;### fifteen.atl(709) 				setboardcolor RANDOM
   lda RANDOM
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(710) 				sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(711) 				inc dir
   inc dir
_lbl134:
;### fifteen.atl(707) 			while dir<200
   lda dir
   cmp #200
   jcs _lbl135
   jmp _lbl136
_lbl135:
   lda #0
   sta AUDC1
;### fifteen.atl(714) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(715) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(716) 		else
   jmp _lbl137
_lbl133:
;### fifteen.atl(717) 			sleep 200
   lda #200
   sta sleep__time
   jsr sleep
;### fifteen.atl(719) 		CH=28
_lbl137:
   lda #28
   sta CH
;### fifteen.atl(720) 		goto gameend
   jmp gameend
;### fifteen.atl(722) 	textbuf(0,0)="Moves: [moves]     "
_lbl132:
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
;### fifteen.atl(723) 	text 27 163 12
   lda #27
   sta text__xt
   lda #163
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(725) 	ss:stick__state = STICK(0)
   lda STICK
   sta game__ss
;### fifteen.atl(727) 	dir=0
   lda #0
   sta dir
;### fifteen.atl(728) 	if ss = 14 dir = 1
   lda game__ss
   cmp #14
   jne _lbl138
   lda #1
   sta dir
;### fifteen.atl(729) 	if ss = 7  dir = 2
_lbl138:
   lda game__ss
   cmp #7
   jne _lbl139
   lda #2
   sta dir
;### fifteen.atl(730) 	if ss = 13 dir = 3
_lbl139:
   lda game__ss
   cmp #13
   jne _lbl140
   lda #3
   sta dir
;### fifteen.atl(731) 	if ss = 11 dir = 4
_lbl140:
   lda game__ss
   cmp #11
   jne _lbl141
   lda #4
   sta dir
;### fifteen.atl(732) 	if dir <> 0 
_lbl141:
   lda dir
   cmp #0
   jeq _lbl142
;### fifteen.atl(733) 		x,y=findempty
   jsr findempty
   lda findempty__x
   sta game__x
   lda findempty__y
   sta game__y
;### fifteen.atl(734) 		dec dir
   dec dir
;### fifteen.atl(735) 		movetile x y dir 1
   lda game__x
   sta movetile__xs
   lda game__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #1
   sta movetile__show
   jsr movetile
;### fifteen.atl(736) 		dir=0
   lda #0
   sta dir
;### fifteen.atl(737) 		sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(739) 	if CH <> 28 goto gameloop  ; ESC 
_lbl142:
   lda CH
   cmp #28
   jeq _lbl143
   jmp gameloop
;### fifteen.atl(741) 	gameend@
_lbl143:
gameend:
;### fifteen.atl(743) 	CH = none
   lda #key__none
   sta CH
   rts
.endp
   icl '../../src/platform/atari/atari.asm'
