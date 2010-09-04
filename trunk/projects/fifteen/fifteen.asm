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
_97 equ 216
_98 equ 217
_99 equ 218
_100 equ 219
_101 equ 220
_102 equ 221
   org $2e0
   dta a($2000)
   org $2000
;### fifteen.atl(156) piccnt@$612:byte= RANDOM and 3
   lda RANDOM
   and #3
   sta piccnt
;### fifteen.atl(744) initscreen
   jsr initscreen
;### fifteen.atl(745) besttime(0)=0
   lda #0
   sta besttime
;### fifteen.atl(746) bmoves=999
   lda #231
   sta bmoves
   lda #3
   sta bmoves+1
;### fifteen.atl(752) 	game
   jmp _lbl144
;### fifteen.atl(752) 	game
_lbl145:
;### fifteen.atl(748) 	titlescreen
   jsr titlescreen
;### fifteen.atl(749) 	game
   jsr game
;### fifteen.atl(752) 	game
_lbl144:
;### fifteen.atl(752) 	game
   jmp _lbl145
_lbl158:
   jmp _lbl158
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
buf1:
;### fifteen.atl(128) const buf1: array(23, 191) = file "einstein-tongue.pbm"
   ins 'einstein-tongue.pbm'
buf2:
;### fifteen.atl(129) const buf2: array(23, 191) = file "miner.pbm"
   ins 'miner.pbm'
buf3:
;### fifteen.atl(130) const buf3: array(23, 191) = file "johnromero.pbm"
   ins 'johnromero.pbm'
buf4:
;### fifteen.atl(131) const buf4: array(23, 191) = file "clinteastwood.pbm"
   ins 'clinteastwood.pbm'
leafh:
;### fifteen.atl(132) const leafh: array(47) = file "leafhoriz.pbm"
   ins 'leafhoriz.pbm'
leafv:
;### fifteen.atl(133) const leafv: array(47) = file "leafvert.pbm"
   ins 'leafvert.pbm'
mains:
;### fifteen.atl(134) const mains: array(2800) = file "15.pbm"
   ins '15.pbm'
fonts:
;### fifteen.atl(135) const fonts: array(1024) = file "STENCIL.FNT"
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
;### fifteen.atl(171) 	tmp = timer
   lda timer
   sta drawmainscreen__tmp
;### fifteen.atl(172) 	tmp = tmp + time
   lda drawmainscreen__tmp
   clc
   adc sleep__time
   sta drawmainscreen__tmp
;### fifteen.atl(175) 	while timer<>tmp
   jmp _lbl7
;### fifteen.atl(175) 	while timer<>tmp
_lbl9:
;### fifteen.atl(175) 	while timer<>tmp
_lbl7:
;### fifteen.atl(173) 	while timer<>tmp
   lda timer
   cmp drawmainscreen__tmp
   jeq _lbl8
;### fifteen.atl(175) 	while timer<>tmp
   jmp _lbl9
;### fifteen.atl(175) 	while timer<>tmp
_lbl8:
   rts
.endp
copyblock .proc
;### fifteen.atl(177) 	bptr(0)=srcbm(0)
   lda copyblock__srcbm
   sta bptr
;### fifteen.atl(178) 	bptr(1)=srcbm(1)
   lda copyblock__srcbm+1
   sta bptr+1
;### fifteen.atl(179) 	i=0
   lda #0
   sta i
;### fifteen.atl(199) 		inc i
   jmp _lbl10
;### fifteen.atl(199) 		inc i
_lbl14:
;### fifteen.atl(182) 		xbtmp=i
   lda i
   sta xbtmp
;### fifteen.atl(183) 		xbtmp=xbtmp+xscr
   lda xbtmp
   clc
   adc copyblock__xscr
   sta xbtmp
;### fifteen.atl(184) 		ybtmp=yscr
   lda copyblock__yscr
   sta ybtmp
;### fifteen.atl(186) 		xctmp=xbm+i
   lda copyblock__xbm
   clc
   adc i
   sta xctmp
   lda #0
   rol
   sta xctmp+1
;### fifteen.atl(187) 		yctmp=ybm
   lda copyblock__ybm
   sta yctmp
   lda copyblock__ybm+1
   sta yctmp+1
;### fifteen.atl(189) 		ctmp1 = bptr
   lda bptr
   sta ctmp1
   lda bptr+1
   sta ctmp1+1
;### fifteen.atl(190) 		ctmp1= ctmp1+ xctmp
   lda ctmp1
   clc
   adc xctmp+0
   sta ctmp1
   lda ctmp1+1
   adc xctmp+1
   sta ctmp1+1
;### fifteen.atl(191) 		ctmp1= ctmp1+ yctmp
   lda ctmp1
   clc
   adc yctmp+0
   sta ctmp1
   lda ctmp1+1
   adc yctmp+1
   sta ctmp1+1
;### fifteen.atl(197) 		inc i
   lda #1
   sta _s2__j
_lbl13:
;### fifteen.atl(193) 			screen(xbtmp,ybtmp)=mem(ctmp1)
   lda #<mem
   sta _arr
   lda #>mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda (_arr),y
   sta _97
   ldy ybtmp
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy xbtmp
   lda _97
   sta (_arr),y
;### fifteen.atl(194) 			ctmp1 = ctmp1 + wbm
   lda ctmp1
   clc
   adc copyblock__wbm
   sta ctmp1
   jcc _lbl146
   inc ctmp1+1
_lbl146:
;### fifteen.atl(195) 			inc ybtmp
   inc ybtmp
   inc _s2__j
   lda copyblock__ysize
   cmp _s2__j
   jcs _lbl13
   inc i
;### fifteen.atl(199) 		inc i
_lbl10:
;### fifteen.atl(180) 	while i<xsize 
   lda i
   cmp copyblock__xsize
   jcs _lbl11
;### fifteen.atl(199) 		inc i
   jmp _lbl14
;### fifteen.atl(199) 		inc i
_lbl11:
   rts
.endp
text .proc
;### fifteen.atl(201) 	ii:byte=0
   lda #0
   sta text__ii
;### fifteen.atl(209) 		inc ii
   jmp _lbl15
;### fifteen.atl(209) 		inc ii
_lbl17:
;### fifteen.atl(203) 		ctmp1=textbuf(ii)
   ldx text__ii
   lda textbuf,x
   sta _98
   lda _98
   sta ctmp1
   lda #0
   sta ctmp1+1
;### fifteen.atl(204) 		ctmp1=ctmp1*8
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
;### fifteen.atl(205) 		btmp1= xt+ii
   lda text__xt
   clc
   adc text__ii
   sta btmp1
;### fifteen.atl(206) 		copyblock fonts 0 ctmp1 1 btmp1 yt 1 8
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
;### fifteen.atl(207) 		inc ii
   inc text__ii
;### fifteen.atl(209) 		inc ii
_lbl15:
;### fifteen.atl(202) 	while ii<len
   lda text__ii
   cmp text__len
   jcs _lbl16
;### fifteen.atl(209) 		inc ii
   jmp _lbl17
;### fifteen.atl(209) 		inc ii
_lbl16:
   rts
.endp
copytile .proc
;### fifteen.atl(211) 	ctmp1=ybm*24
   lda copytile__ybm
   ldx #24
   jsr _sys_mul8
   lda _TEMPW2
   sta ctmp1
   lda _TEMPW2+1
   sta ctmp1+1
;### fifteen.atl(212) 	ctmp1=ctmp1*48
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
;### fifteen.atl(213) 	xbm=xbm*6
   lda copytile__xbm
   asl
   clc
   adc copytile__xbm
   sta copytile__xbm
   asl copytile__xbm
;### fifteen.atl(214) 	copyblock currentbuf xbm ctmp1 24 xscr yscr xsize ysize
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
;### fifteen.atl(217) 	i=0
   lda #0
   sta i
;### fifteen.atl(227) 		inc i
   jmp _lbl18
;### fifteen.atl(227) 		inc i
_lbl23:
;### fifteen.atl(219) 		xbtmp=i+xscr
   lda i
   clc
   adc eraserect__xscr
   sta xbtmp
;### fifteen.atl(220) 		ybtmp=yscr
   lda eraserect__yscr
   sta ybtmp
;### fifteen.atl(221) 		j=0
   lda #0
   sta j
;### fifteen.atl(226) 		inc i
   jmp _lbl20
_lbl22:
;### fifteen.atl(223) 			screen(xbtmp,ybtmp)=$ff
   ldy ybtmp
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy xbtmp
   lda #255
   sta (_arr),y
;### fifteen.atl(224) 			inc ybtmp
   inc ybtmp
;### fifteen.atl(225) 			inc j
   inc j
_lbl20:
;### fifteen.atl(222) 		while j<height
   lda j
   cmp eraserect__height
   jcs _lbl21
   jmp _lbl22
_lbl21:
   inc i
;### fifteen.atl(227) 		inc i
_lbl18:
;### fifteen.atl(218) 	while i<width
   lda i
   cmp eraserect__width
   jcs _lbl19
;### fifteen.atl(227) 		inc i
   jmp _lbl23
;### fifteen.atl(227) 		inc i
_lbl19:
   rts
.endp
findempty .proc
;### fifteen.atl(229) 	x=0
   lda #0
   sta findempty__x
;### fifteen.atl(230) 	y=0
   lda #0
   sta findempty__y
;### fifteen.atl(237) 				y=l
   lda #0
   sta k
;### fifteen.atl(237) 				y=l
_lbl28:
;### fifteen.atl(237) 				y=l
   lda #0
   sta l
;### fifteen.atl(237) 				y=l
_lbl27:
;### fifteen.atl(233) 			if playfield(k,l) = T_EMPTY
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _99
   lda _99
   cmp #root__T_EMPTY
   jne _lbl26
;### fifteen.atl(234) 				x=k
   lda k
   sta findempty__x
;### fifteen.atl(235) 				y=l
   lda l
   sta findempty__y
;### fifteen.atl(237) 				y=l
_lbl26:
;### fifteen.atl(237) 				y=l
   inc l
;### fifteen.atl(237) 				y=l
   lda l
   cmp #4
   jne _lbl27
;### fifteen.atl(237) 				y=l
   inc k
;### fifteen.atl(237) 				y=l
   lda k
   cmp #4
   jne _lbl28
   rts
.endp
gettile .proc
;### fifteen.atl(239) 	ty = tile and 3
   lda gettile__tile
   and #3
   sta gettile__ty
;### fifteen.atl(240) 	tx = tile / 4
   lda gettile__tile
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #4
   jsr _sys_div8
   sta gettile__tx
;### fifteen.atl(241) 	tx = tx and 3
   lda gettile__tx
   and #3
   sta gettile__tx
   rts
.endp
showboard .proc
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   lda #0
   sta k
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
_lbl34:
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   lda #0
   sta l
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
_lbl33:
;### fifteen.atl(246) 			xbtmp = 6*k+XOFF
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
;### fifteen.atl(247) 			ybtmp = 48*l+YOFF
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
;### fifteen.atl(248) 			tile = playfield(k,l)
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _s9__tile
;### fifteen.atl(249) 			if tile <> T_EMPTY
   lda _s9__tile
   cmp #root__T_EMPTY
   jeq _lbl31
;### fifteen.atl(250) 				i,j=gettile tile
   lda _s9__tile
   sta gettile__tile
   jsr gettile
   lda gettile__tx
   sta i
   lda gettile__ty
   sta j
;### fifteen.atl(251) 				copytile i j xbtmp ybtmp 6 48
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
;### fifteen.atl(252) 			else
   jmp _lbl32
_lbl31:
;### fifteen.atl(253) 				eraserect  xbtmp ybtmp 6 48
   lda xbtmp
   sta eraserect__xscr
   lda ybtmp
   sta eraserect__yscr
   lda #6
   sta eraserect__width
   lda #48
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
_lbl32:
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   inc l
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   lda l
   cmp #4
   jne _lbl33
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   inc k
;### fifteen.atl(255) 				eraserect  xbtmp ybtmp 6 48
   lda k
   cmp #4
   jne _lbl34
   rts
.endp
movetile .proc
;### fifteen.atl(260) 	xoffset=1
   lda #1
   sta movetile__xoffset
;### fifteen.atl(261) 	yoffset=1
   lda #1
   sta movetile__yoffset
;### fifteen.atl(262) 	AUDC1=$22
   lda #34
   sta AUDC1
;### fifteen.atl(263) 	if m = D_UP
   lda movetile__m
   cmp #root__D_UP
   jne _lbl35
;### fifteen.atl(264) 		if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl36
;### fifteen.atl(265) 			yoffset=2
   lda #2
   sta movetile__yoffset
;### fifteen.atl(267) 	if m = D_RIGHT
_lbl36:
_lbl35:
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl37
;### fifteen.atl(268) 		if xs>0
   lda #0
   cmp movetile__xs
   jcs _lbl38
;### fifteen.atl(269) 			xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(271) 	if m = D_DOWN
_lbl38:
_lbl37:
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl39
;### fifteen.atl(272) 		if ys>0
   lda #0
   cmp movetile__ys
   jcs _lbl40
;### fifteen.atl(273) 			yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(275) 	if m = D_LEFT
_lbl40:
_lbl39:
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl41
;### fifteen.atl(276) 		if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl42
;### fifteen.atl(277) 			xoffset=2
   lda #2
   sta movetile__xoffset
;### fifteen.atl(279) 	if xoffset <>1 or yoffset <>1
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
;### fifteen.atl(281) 		xbtmp=xs+xoffset
   lda movetile__xs
   clc
   adc movetile__xoffset
   sta xbtmp
;### fifteen.atl(282) 		dec xbtmp
   dec xbtmp
;### fifteen.atl(283) 		ybtmp=ys+yoffset
   lda movetile__ys
   clc
   adc movetile__yoffset
   sta ybtmp
;### fifteen.atl(284) 		dec ybtmp
   dec ybtmp
;### fifteen.atl(285) 		playfield(xs,ys)=playfield(xbtmp,ybtmp)
   ldy ybtmp
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy xbtmp
   lda (_arr),y
   sta _100
   ldy movetile__ys
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy movetile__xs
   lda _100
   sta (_arr),y
;### fifteen.atl(286) 		playfield(xbtmp,ybtmp)=T_EMPTY
   ldy ybtmp
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy xbtmp
   lda #root__T_EMPTY
   sta (_arr),y
;### fifteen.atl(287) 		inc moves
   inc moves+0
   jne _lbl147
   inc moves+1
_lbl147:
;### fifteen.atl(288) 		if moves > 999 moves=999
   lda #231
   cmp moves
   lda #3
   sbc moves+1
   jcs _lbl46
   lda #231
   sta moves
   lda #3
   sta moves+1
;### fifteen.atl(290) 	xoffset=0
_lbl46:
_lbl45:
   lda #0
   sta movetile__xoffset
;### fifteen.atl(291) 	yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(292) 	if show >0
   lda #0
   cmp movetile__show
   jcs _lbl47
;### fifteen.atl(293) 		z=0
   lda #0
   sta movetile__z
;### fifteen.atl(294) 		if show=2 z=5
   lda movetile__show
   cmp #2
   jne _lbl48
   lda #5
   sta movetile__z
;### fifteen.atl(295) 		while z<6
_lbl48:
;### fifteen.atl(359) 	AUDC1=0
   jmp _lbl49
_lbl67:
;### fifteen.atl(297) 			xbtmp=xs*6+XOFF
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
   lda #0
   rol
   sta xbtmp+1
;### fifteen.atl(298) 			ybtmp=ys*48+YOFF
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
   lda #0
   rol
   sta ybtmp+1
;### fifteen.atl(299) 			AUDF1=z+xs
   lda movetile__z
   clc
   adc movetile__xs
   sta AUDF1
;### fifteen.atl(300) 			xtilesize=6
   lda #6
   sta xtilesize
;### fifteen.atl(301) 			ytilesize=48
   lda #48
   sta ytilesize
;### fifteen.atl(303) 			if m = D_LEFT
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl51
;### fifteen.atl(304) 				if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl52
;### fifteen.atl(305) 					xoffset=5-z
   lda #5
   sec
   sbc movetile__z
   sta movetile__xoffset
;### fifteen.atl(306) 					yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(307) 					btmp1 = xbtmp+6
   lda xbtmp
   clc
   adc #6
   sta btmp1
;### fifteen.atl(308) 					btmp1 = btmp1+xoffset
   lda btmp1
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(309) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(310) 					xtilesize=1
   lda #1
   sta xtilesize
;### fifteen.atl(312) 			if m = D_DOWN
_lbl52:
_lbl51:
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl53
;### fifteen.atl(313) 				if ys>0
   lda #0
   cmp movetile__ys
   jcs _lbl54
;### fifteen.atl(314) 					yoffset=z-5
   lda movetile__z
   sec
   sbc #5
   sta movetile__yoffset
;### fifteen.atl(315) 					yoffset = yoffset * 8
   lda movetile__yoffset
   asl
   asl
   asl
   sta movetile__yoffset
;### fifteen.atl(316) 					xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(317) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(318) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(319) 					btmp2=btmp2-8
   lda btmp2
   sec
   sbc #8
   sta btmp2
;### fifteen.atl(320) 					if z=5 btmp2 = btmp2-40
   lda movetile__z
   cmp #5
   jne _lbl55
   lda btmp2
   sec
   sbc #40
   sta btmp2
;### fifteen.atl(321) 					ytilesize=8
_lbl55:
   lda #8
   sta ytilesize
;### fifteen.atl(322) 			if m = D_RIGHT
_lbl54:
_lbl53:
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl56
;### fifteen.atl(323) 				if xs>0
   lda #0
   cmp movetile__xs
   jcs _lbl57
;### fifteen.atl(324) 					xoffset=z-5
   lda movetile__z
   sec
   sbc #5
   sta movetile__xoffset
;### fifteen.atl(325) 					yoffset=0
   lda #0
   sta movetile__yoffset
;### fifteen.atl(326) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(327) 					dec btmp1
   dec btmp1
;### fifteen.atl(328) 					if z=5 btmp1 = btmp1-5
   lda movetile__z
   cmp #5
   jne _lbl58
   lda btmp1
   sec
   sbc #5
   sta btmp1
;### fifteen.atl(329) 					btmp2=ybtmp+yoffset
_lbl58:
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(330) 					xtilesize=1
   lda #1
   sta xtilesize
;### fifteen.atl(332) 			if m = D_UP
_lbl57:
_lbl56:
   lda movetile__m
   cmp #root__D_UP
   jne _lbl59
;### fifteen.atl(333) 				if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl60
;### fifteen.atl(334) 					yoffset=5-z
   lda #5
   sec
   sbc movetile__z
   sta movetile__yoffset
;### fifteen.atl(335) 					yoffset = yoffset * 8
   lda movetile__yoffset
   asl
   asl
   asl
   sta movetile__yoffset
;### fifteen.atl(336) 					xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(337) 					btmp1=xbtmp+xoffset
   lda xbtmp
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(338) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(339) 					btmp2=btmp2+48
   lda btmp2
   clc
   adc #48
   sta btmp2
;### fifteen.atl(340) 					ytilesize=8
   lda #8
   sta ytilesize
;### fifteen.atl(342) 			if xtilesize<>6 or ytilesize<>48
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
;### fifteen.atl(343) 				if z=5 xtilesize=6 ytilesize=48 
   lda movetile__z
   cmp #5
   jne _lbl64
   lda #6
   sta xtilesize
   lda #48
   sta ytilesize
;### fifteen.atl(344) 				eraserect btmp1 btmp2 xtilesize ytilesize
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
;### fifteen.atl(347) 			tile = playfield(xs,ys)
_lbl63:
   ldy movetile__ys
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy movetile__xs
   lda (_arr),y
   sta movetile__tile
;### fifteen.atl(348) 			if tile <> T_EMPTY
   lda movetile__tile
   cmp #root__T_EMPTY
   jeq _lbl65
;### fifteen.atl(349) 				if show >0
   lda #0
   cmp movetile__show
   jcs _lbl66
;### fifteen.atl(350) 					k,l = gettile tile
   lda movetile__tile
   sta gettile__tile
   jsr gettile
   lda gettile__tx
   sta k
   lda gettile__ty
   sta l
;### fifteen.atl(351) 					btmp1=xs*6
   lda movetile__xs
   asl
   clc
   adc movetile__xs
   sta btmp1
   asl btmp1
;### fifteen.atl(352) 					btmp1=btmp1+XOFF
   inc btmp1
;### fifteen.atl(353) 					btmp1=btmp1+xoffset
   lda btmp1
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(354) 					btmp2=ys*48
   lda movetile__ys
   asl
   clc
   adc movetile__ys
   sta btmp2
   asl btmp2
   asl btmp2
   asl btmp2
   asl btmp2
;### fifteen.atl(355) 					btmp2=btmp2+YOFF
   lda btmp2
   clc
   adc #root__YOFF
   sta btmp2
;### fifteen.atl(356) 					btmp2=btmp2+yoffset
   lda btmp2
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(357) 					copytile k l btmp1 btmp2 6 48
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
;### fifteen.atl(358) 			inc z
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
;### fifteen.atl(363) 	i=0
   lda #0
   sta i
;### fifteen.atl(371) 		inc i
   jmp _lbl68
;### fifteen.atl(371) 		inc i
_lbl73:
;### fifteen.atl(365) 		j=0
   lda #0
   sta j
;### fifteen.atl(369) 		inc i
   jmp _lbl70
_lbl72:
;### fifteen.atl(367) 			screen(i,j)=255
   ldy j
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy i
   lda #255
   sta (_arr),y
;### fifteen.atl(368) 			inc j
   inc j
_lbl70:
;### fifteen.atl(366) 		while j<200
   lda j
   cmp #200
   jcs _lbl71
   jmp _lbl72
_lbl71:
   inc i
;### fifteen.atl(371) 		inc i
_lbl68:
;### fifteen.atl(364) 	while i<40
   lda i
   cmp #40
   jcs _lbl69
;### fifteen.atl(371) 		inc i
   jmp _lbl73
;### fifteen.atl(371) 		inc i
_lbl69:
   rts
.endp
changepicture .proc
;### fifteen.atl(373) 	if piccnt=0
   lda piccnt
   cmp #0
   jne _lbl74
;### fifteen.atl(374) 		currentbuf=buf1
   lda #<buf1
   sta currentbuf
   lda #>buf1
   sta currentbuf+1
;### fifteen.atl(375) 		c=$1c
   lda #28
   sta changepicture__c
;### fifteen.atl(376) 	if piccnt=1
_lbl74:
   lda piccnt
   cmp #1
   jne _lbl75
;### fifteen.atl(377) 		currentbuf=buf2
   lda #<buf2
   sta currentbuf
   lda #>buf2
   sta currentbuf+1
;### fifteen.atl(378) 		c=$0e
   lda #14
   sta changepicture__c
;### fifteen.atl(380) 	if piccnt=2
_lbl75:
   lda piccnt
   cmp #2
   jne _lbl76
;### fifteen.atl(381) 		currentbuf=buf3
   lda #<buf3
   sta currentbuf
   lda #>buf3
   sta currentbuf+1
;### fifteen.atl(382) 		c=$3a
   lda #58
   sta changepicture__c
;### fifteen.atl(383) 	if piccnt=3
_lbl76:
   lda piccnt
   cmp #3
   jne _lbl77
;### fifteen.atl(384) 		currentbuf=buf4
   lda #<buf4
   sta currentbuf
   lda #>buf4
   sta currentbuf+1
;### fifteen.atl(385) 		c=$7a
   lda #122
   sta changepicture__c
;### fifteen.atl(387) 	inc piccnt
_lbl77:
   inc piccnt
;### fifteen.atl(388) 	if piccnt>=4 piccnt=0
   lda piccnt
   cmp #4
   jcc _lbl78
   lda #0
   sta piccnt
;### fifteen.atl(391) 	if piccnt>=4 piccnt=0
_lbl78:
   rts
.endp
countgoodtiles .proc
;### fifteen.atl(393) 	res=0
   lda #0
   sta countgoodtiles__res
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
   lda #0
   sta k
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
_lbl83:
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
   lda #0
   sta l
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
_lbl82:
;### fifteen.atl(396) 			tile=k*4
   lda k
   asl
   asl
   sta _s14__tile
;### fifteen.atl(397) 			tile=tile+l
   lda _s14__tile
   clc
   adc l
   sta _s14__tile
;### fifteen.atl(398) 			if playfield(k,l) = tile inc res
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   sta _101
   lda _101
   cmp _s14__tile
   jne _lbl81
   inc countgoodtiles__res
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
_lbl81:
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
   inc l
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
   lda l
   cmp #4
   jne _lbl82
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
   inc k
;### fifteen.atl(399) 			if playfield(k,l) = tile inc res
   lda k
   cmp #4
   jne _lbl83
   rts
.endp
inittiles .proc
;### fifteen.atl(406) 			playfield(k,l)=tile
   lda #0
   sta k
;### fifteen.atl(406) 			playfield(k,l)=tile
_lbl87:
;### fifteen.atl(406) 			playfield(k,l)=tile
   lda #0
   sta l
;### fifteen.atl(406) 			playfield(k,l)=tile
_lbl86:
;### fifteen.atl(403) 			tile=k*4
   lda k
   asl
   asl
   sta _s16__tile
;### fifteen.atl(404) 			tile=tile+l
   lda _s16__tile
   clc
   adc l
   sta _s16__tile
;### fifteen.atl(405) 			playfield(k,l)=tile
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda _s16__tile
   sta (_arr),y
;### fifteen.atl(406) 			playfield(k,l)=tile
   inc l
;### fifteen.atl(406) 			playfield(k,l)=tile
   lda l
   cmp #4
   jne _lbl86
;### fifteen.atl(406) 			playfield(k,l)=tile
   inc k
;### fifteen.atl(406) 			playfield(k,l)=tile
   lda k
   cmp #4
   jne _lbl87
   rts
.endp
setboardcolor .proc
;### fifteen.atl(408) 	player_color(0)=col
   lda setboardcolor__col
   sta player_color
;### fifteen.atl(409) 	player_color(1)=col
   lda setboardcolor__col
   sta player_color+1
;### fifteen.atl(410) 	player_color(2)=col
   lda setboardcolor__col
   sta player_color+2
   rts
.endp
changecolor .proc
;### fifteen.atl(414) 	btmp1=col and $f
   lda changecolor__col
   and #15
   sta btmp1
;### fifteen.atl(415) 	btmp2=col and $f0
   lda changecolor__col
   and #240
   sta btmp2
;### fifteen.atl(417) 	xbtmp=currcol and $f
   lda currcol
   and #15
   sta xbtmp
;### fifteen.atl(418) 	ybtmp=currcol and $f0
   lda currcol
   and #240
   sta ybtmp
;### fifteen.atl(420) 	if xbtmp < btmp1 ; fade in
   lda xbtmp
   cmp btmp1
   jcs _lbl88
;### fifteen.atl(427) 	else ; fade out
   jmp _lbl89
_lbl91:
;### fifteen.atl(422) 			xbtmp = xbtmp + 2
   lda xbtmp
   clc
   adc #2
   sta xbtmp
;### fifteen.atl(423) 			currcol = btmp2
   lda btmp2
   sta currcol
;### fifteen.atl(424) 			currcol = currcol + xbtmp
   lda currcol
   clc
   adc xbtmp
   sta currcol
;### fifteen.atl(425) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(426) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
_lbl89:
;### fifteen.atl(421) 		while btmp1 > xbtmp
   lda xbtmp
   cmp btmp1
   jcs _lbl90
   jmp _lbl91
_lbl90:
   jmp _lbl92
_lbl88:
;### fifteen.atl(435) 			sleep 1
   jmp _lbl93
;### fifteen.atl(435) 			sleep 1
_lbl95:
;### fifteen.atl(429) 			xbtmp = xbtmp -2
   lda xbtmp
   sec
   sbc #2
   sta xbtmp
;### fifteen.atl(430) 			currcol = ybtmp
   lda ybtmp
   sta currcol
;### fifteen.atl(431) 			currcol = currcol + xbtmp
   lda currcol
   clc
   adc xbtmp
   sta currcol
;### fifteen.atl(432) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(433) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(435) 			sleep 1
_lbl93:
;### fifteen.atl(428) 		while btmp1 < xbtmp
   lda btmp1
   cmp xbtmp
   jcs _lbl94
;### fifteen.atl(435) 			sleep 1
   jmp _lbl95
;### fifteen.atl(435) 			sleep 1
_lbl94:
;### fifteen.atl(435) 			sleep 1
_lbl92:
   rts
.endp
drawmainscreen .proc
;### fifteen.atl(437) 	copyblock mains 0 0 14 26 0 14 200
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
;### fifteen.atl(438) 	j=0
   lda #0
   sta j
;### fifteen.atl(449) 	tmp:byte = changepicture
   lda #0
   sta _s19__i
_lbl97:
;### fifteen.atl(440) 		xbtmp=i*6
   lda _s19__i
   asl
   clc
   adc _s19__i
   sta xbtmp
   asl xbtmp
;### fifteen.atl(441) 		inc xbtmp
   inc xbtmp
;### fifteen.atl(442) 		copyblock leafh 0 0 6 xbtmp 0 6 8
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
;### fifteen.atl(443) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(444) 		copyblock leafv 0 0 1 0 xbtmp 1 48
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
;### fifteen.atl(445) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(446) 		copyblock leafv 0 0 1 25 xbtmp 1 48
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
;### fifteen.atl(447) 		j=j+48
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
;### fifteen.atl(450) 	showboard
   jsr showboard
;### fifteen.atl(451) 	currcol=0
   lda #0
   sta currcol
;### fifteen.atl(452) 	changecolor tmp
   lda drawmainscreen__tmp
   sta changecolor__col
   jsr changecolor
   rts
.endp
shuffletiles .proc
;### fifteen.atl(455) 	c:byte=0
   lda #0
   sta shuffletiles__c
;### fifteen.atl(456) 	loop@
loop:
;### fifteen.atl(457) 	dir=RANDOM and 3
   lda RANDOM
   and #3
   sta dir
;### fifteen.atl(458) 	x,y=findempty
   jsr findempty
   lda findempty__x
   sta shuffletiles__x
   lda findempty__y
   sta shuffletiles__y
;### fifteen.atl(459) 	movetile x y dir 2
   lda shuffletiles__x
   sta movetile__xs
   lda shuffletiles__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #2
   sta movetile__show
   jsr movetile
;### fifteen.atl(460) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta shuffletiles__cnt
;### fifteen.atl(461) 	if c<50 inc c
   lda shuffletiles__c
   cmp #50
   jcs _lbl98
   inc shuffletiles__c
;### fifteen.atl(462) 	if cnt>0 or c<50 goto loop
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
;### fifteen.atl(464) 	if cnt>0 or c<50 goto loop
_lbl101:
   rts
.endp
cycle .proc
;### fifteen.atl(466) 	if play = 1
   lda play
   cmp #1
   jne _lbl102
;### fifteen.atl(467) 		mus_play
   jsr mus_play
;### fifteen.atl(468) 	vcnt=0
_lbl102:
   lda #0
   sta vcnt
   jmp $e462
.endp
fillpmg .proc
;### fifteen.atl(471) 	ctmp1 = $180
   lda #128
   sta ctmp1
   lda #1
   sta ctmp1+1
;### fifteen.atl(477) 	for i:16..111
   jmp _lbl103
_lbl105:
;### fifteen.atl(474) 		pmg_mem(i)=$0
   lda #0
   ldx i
   sta pmg_mem,x
;### fifteen.atl(475) 		inc ctmp1
   inc ctmp1+0
   jne _lbl148
   inc ctmp1+1
_lbl148:
_lbl103:
;### fifteen.atl(473) 	while ctmp1<=$3ff
   lda #255
   cmp ctmp1
   lda #3
   sbc ctmp1+1
   jcc _lbl104
   jmp _lbl105
_lbl104:
;### fifteen.atl(487) 	for i:12..115
   lda #16
   sta _s21__i
_lbl107:
;### fifteen.atl(478) 		ctmp1=$200
   lda #0
   sta ctmp1
   lda #2
   sta ctmp1+1
;### fifteen.atl(479) 		ctmp1 = ctmp1+i
   lda ctmp1
   clc
   adc _s21__i
   sta ctmp1
   jcc _lbl149
   inc ctmp1+1
_lbl149:
;### fifteen.atl(480) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(481) 		ctmp1=ctmp1+ $80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl150
   inc ctmp1+1
_lbl150:
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
   jcc _lbl151
   inc ctmp1+1
_lbl151:
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
   jcc _lbl152
   inc ctmp1+1
_lbl152:
   inc _s21__i
   lda _s21__i
   cmp #112
   jne _lbl107
;### fifteen.atl(494) 	for i:75..79
   lda #12
   sta _s22__i
_lbl109:
;### fifteen.atl(488) 		ctmp1=$180
   lda #128
   sta ctmp1
   lda #1
   sta ctmp1+1
;### fifteen.atl(489) 		ctmp1=ctmp1 + i
   lda ctmp1
   clc
   adc _s22__i
   sta ctmp1
   jcc _lbl153
   inc ctmp1+1
_lbl153:
;### fifteen.atl(490) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(491) 		ctmp1=ctmp1+ $200
   lda ctmp1
   clc
   adc #0
   sta ctmp1
   lda ctmp1+1
   adc #2
   sta ctmp1+1
;### fifteen.atl(492) 		pmg_mem(ctmp1)=$fe
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
;### fifteen.atl(498) 		pmg_mem(ctmp1)=$00
   lda #75
   sta _s23__i
;### fifteen.atl(498) 		pmg_mem(ctmp1)=$00
_lbl111:
;### fifteen.atl(495) 		ctmp1=$380
   lda #128
   sta ctmp1
   lda #3
   sta ctmp1+1
;### fifteen.atl(496) 		ctmp1= ctmp1+ i
   lda ctmp1
   clc
   adc _s23__i
   sta ctmp1
   jcc _lbl154
   inc ctmp1+1
_lbl154:
;### fifteen.atl(497) 		pmg_mem(ctmp1)=$00
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(498) 		pmg_mem(ctmp1)=$00
   inc _s23__i
;### fifteen.atl(498) 		pmg_mem(ctmp1)=$00
   lda _s23__i
   cmp #80
   jne _lbl111
   rts
.endp
windowpmg .proc
;### fifteen.atl(512) 		pmg_mem(ctmp1)=$3f
   lda #0
   sta _s24__i
;### fifteen.atl(512) 		pmg_mem(ctmp1)=$3f
_lbl113:
;### fifteen.atl(505) 		ctmp1=$237
   lda #55
   sta ctmp1
   lda #2
   sta ctmp1+1
;### fifteen.atl(506) 		ctmp1 = ctmp1 +i
   lda ctmp1
   clc
   adc _s24__i
   sta ctmp1
   jcc _lbl155
   inc ctmp1+1
_lbl155:
;### fifteen.atl(507) 		pmg_mem(ctmp1)=$fc
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #252
   sta (_arr),y
;### fifteen.atl(508) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl156
   inc ctmp1+1
_lbl156:
;### fifteen.atl(509) 		pmg_mem(ctmp1)=$0
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(510) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl157
   inc ctmp1+1
_lbl157:
;### fifteen.atl(511) 		pmg_mem(ctmp1)=$3f
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #63
   sta (_arr),y
;### fifteen.atl(512) 		pmg_mem(ctmp1)=$3f
   inc _s24__i
;### fifteen.atl(512) 		pmg_mem(ctmp1)=$3f
   lda _s24__i
   cmp #18
   jne _lbl113
   rts
.endp
setpmg .proc
;### fifteen.atl(515) 	fillpmg
   jsr fillpmg
;### fifteen.atl(517) 	PMBASE= $9c
   lda #156
   sta PMBASE
;### fifteen.atl(518) 	GRACTL=3
   lda #3
   sta GRACTL
;### fifteen.atl(520) 	GTICTLS=$18
   lda #24
   sta GTICTLS
;### fifteen.atl(522) 	player_x(0)=52
   lda #52
   sta player_x
;### fifteen.atl(523) 	player_x(1)=84
   lda #84
   sta player_x+1
;### fifteen.atl(524) 	player_x(2)=116
   lda #116
   sta player_x+2
;### fifteen.atl(525) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(526) 	player_size(0)=3
   lda #3
   sta player_size
;### fifteen.atl(527) 	player_size(1)=3
   lda #3
   sta player_size+1
;### fifteen.atl(528) 	player_size(2)=3
   lda #3
   sta player_size+2
;### fifteen.atl(529) 	player_size(3)=3
   lda #3
   sta player_size+3
;### fifteen.atl(530) 	missile_x(0)=152
   lda #152
   sta missile_x
;### fifteen.atl(531) 	missile_x(1)=160
   lda #160
   sta missile_x+1
;### fifteen.atl(532) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(533) 	missile_x(2)=192
   lda #192
   sta missile_x+2
;### fifteen.atl(534) 	missile_x(3)=200
   lda #200
   sta missile_x+3
;### fifteen.atl(535) 	player_color(3)=$98
   lda #152
   sta player_color+3
;### fifteen.atl(536) 	COLOR0(3)=$96
   lda #150
   sta COLOR0+3
;### fifteen.atl(537) 	missile_size=$ff
   lda #255
   sta missile_size
;### fifteen.atl(538) 	DMACTL =  $2e
   lda #46
   sta DMACTL
   rts
.endp
start_timer .proc
;### fifteen.atl(544) 	timer=0
   lda #0
   sta timer
;### fifteen.atl(545) 	tsec=0
   lda #0
   sta tsec
;### fifteen.atl(546) 	tdsec=0
   lda #0
   sta tdsec
;### fifteen.atl(547) 	tmin=0
   lda #0
   sta tmin
;### fifteen.atl(548) 	tdmin=0
   lda #0
   sta tdmin
   rts
.endp
update_timer .proc
;### fifteen.atl(552) 	if timer>49
   lda #49
   cmp timer
   jcs _lbl114
;### fifteen.atl(553) 		timer = timer -50
   lda timer
   sec
   sbc #50
   sta timer
;### fifteen.atl(554) 		inc tsec
   inc tsec
;### fifteen.atl(556) 	if tsec=10
_lbl114:
   lda tsec
   cmp #10
   jne _lbl115
;### fifteen.atl(557) 		tsec=0
   lda #0
   sta tsec
;### fifteen.atl(558) 		inc tdsec
   inc tdsec
;### fifteen.atl(560) 	if tdsec=6
_lbl115:
   lda tdsec
   cmp #6
   jne _lbl116
;### fifteen.atl(561) 		tdsec=0
   lda #0
   sta tdsec
;### fifteen.atl(562) 		inc tmin
   inc tmin
;### fifteen.atl(564) 	if tmin=10
_lbl116:
   lda tmin
   cmp #10
   jne _lbl117
;### fifteen.atl(565) 		tmin=0
   lda #0
   sta tmin
;### fifteen.atl(566) 		inc tdmin
   inc tdmin
;### fifteen.atl(568) 	if tdmin=6
_lbl117:
   lda tdmin
   cmp #6
   jne _lbl118
;### fifteen.atl(569) 		tdmin=5
   lda #5
   sta tdmin
;### fifteen.atl(570) 		tmin=9
   lda #9
   sta tmin
;### fifteen.atl(571) 		tdsec=5
   lda #5
   sta tdsec
;### fifteen.atl(572) 		tsec=9
   lda #9
   sta tsec
;### fifteen.atl(573) 		tsec=9
_lbl118:
   rts
.endp
drawbesttime .proc
;### fifteen.atl(575) 	if besttime(0,0) <> 0
   ldy #0
   lda besttime_lo,y
   sta _arr
   lda besttime_hi,y
   sta _arr+1
   ldy #0
   lda (_arr),y
   sta _102
   lda _102
   cmp #0
   jeq _lbl119
;### fifteen.atl(577) 		tcnt=0
   lda #0
   sta drawbesttime__tcnt
;### fifteen.atl(581) 		text 26 181 14
   jmp _lbl120
_lbl122:
;### fifteen.atl(579) 			textbuf(tcnt)=besttime(tcnt)
   ldx drawbesttime__tcnt
   lda besttime,x
   ldx drawbesttime__tcnt
   sta textbuf,x
;### fifteen.atl(580) 			inc tcnt
   inc drawbesttime__tcnt
_lbl120:
;### fifteen.atl(578) 		while tcnt<14
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
;### fifteen.atl(583) 		textbuf(0,0)="IN [bmoves] MOVES!   "
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
;### fifteen.atl(584) 		text 27 192 13
   lda #27
   sta text__xt
   lda #192
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(586) 		text 27 192 13
_lbl119:
   rts
.endp
initscreen .proc
;### fifteen.atl(589) 	DMACTL=0
   lda #0
   sta DMACTL
;### fifteen.atl(590) 	sdlstl = dl
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
;### fifteen.atl(591) 	currentbuf=buf1
   lda #<buf1
   sta currentbuf
   lda #>buf1
   sta currentbuf+1
;### fifteen.atl(596) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(598) 	setpmg
   jsr setpmg
;### fifteen.atl(599) 	play=0
   lda #0
   sta play
;### fifteen.atl(600) 	on__vbi cycle
   lda #<cycle
   sta initscreen___94
   lda #>cycle
   sta initscreen___94+1
   lda initscreen___94
   sta VVBLKD
   lda initscreen___94+1
   sta VVBLKD+1
;### fifteen.atl(601) 	on__dli set_col
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
;### fifteen.atl(604) 	DMACTL=0
   lda #0
   sta DMACTL
;### fifteen.atl(605) 	clrscr
   jsr clrscr
;### fifteen.atl(606) 	piccnt = RANDOM and 3
   lda RANDOM
   and #3
   sta piccnt
;### fifteen.atl(607) 	inittiles
   jsr inittiles
;### fifteen.atl(608) 	drawmainscreen
   jsr drawmainscreen
;### fifteen.atl(609) 	fillpmg
   jsr fillpmg
;### fifteen.atl(610) 	DMACTL =  $2e
   lda #46
   sta DMACTL
;### fifteen.atl(612) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(613) 	CH=62
   lda #62
   sta CH
;### fifteen.atl(615) 	COLOR0(1) = $c0
   lda #192
   sta COLOR0+1
;### fifteen.atl(616) 	COLOR0(2) = $c8
   lda #200
   sta COLOR0+2
;### fifteen.atl(617) 	COLOR0(4) = $c0
   lda #192
   sta COLOR0+4
;### fifteen.atl(619) 	drawbesttime
   jsr drawbesttime
;### fifteen.atl(621) 	loop@
loop:
;### fifteen.atl(622) 	if CH = 28  ; ESC 
   lda CH
   cmp #28
   jne _lbl123
;### fifteen.atl(623) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(624) 		ctmp=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta titlescreen__ctmp
;### fifteen.atl(625) 		if ctmp<16
   lda titlescreen__ctmp
   cmp #16
   jcs _lbl124
;### fifteen.atl(626) 			ctmp=currcol
   lda currcol
   sta titlescreen__ctmp
;### fifteen.atl(627) 			changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(628) 			inittiles
   jsr inittiles
;### fifteen.atl(629) 			showboard
   jsr showboard
;### fifteen.atl(630) 			changecolor ctmp
   lda titlescreen__ctmp
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(632) 	if CH=62
_lbl124:
_lbl123:
   lda CH
   cmp #62
   jne _lbl125
;### fifteen.atl(633) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(634) 		if play = 1
   lda play
   cmp #1
   jne _lbl126
;### fifteen.atl(635) 			mus_silence
   jsr mus_silence
;### fifteen.atl(636) 			play = 0
   lda #0
   sta play
;### fifteen.atl(637) 		else
   jmp _lbl127
_lbl126:
;### fifteen.atl(638) 			_x= $0
   ldx #0
;### fifteen.atl(639) 			_y= $9a
   ldy #154
;### fifteen.atl(640) 			mus_init
   jsr mus_init
;### fifteen.atl(641) 			play=1
   lda #1
   sta play
;### fifteen.atl(643) 	if CONSOL = 3
_lbl127:
_lbl125:
   lda CONSOL
   cmp #3
   jne _lbl128
;### fifteen.atl(644) 		shuffletiles
   jsr shuffletiles
;### fifteen.atl(645) 		showboard
   jsr showboard
;### fifteen.atl(646) 		goto loop
   jmp loop
;### fifteen.atl(648) 	if CONSOL = 5
_lbl128:
   lda CONSOL
   cmp #5
   jne _lbl129
;### fifteen.atl(649) 		changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(650) 		btmp1 = changepicture
   jsr changepicture
   lda changepicture__c
   sta btmp1
;### fifteen.atl(651) 		showboard
   jsr showboard
;### fifteen.atl(652) 		changecolor btmp1
   lda btmp1
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(653) 		goto loop
   jmp loop
;### fifteen.atl(656) 	if CONSOL <> 6
_lbl129:
   lda CONSOL
   cmp #6
   jeq _lbl130
;### fifteen.atl(657) 		goto loop
   jmp loop
;### fifteen.atl(658) 		goto loop
_lbl130:
   rts
.endp
game .proc
;### fifteen.atl(662) 	showboard
   jsr showboard
;### fifteen.atl(663) 	play=0
   lda #0
   sta play
;### fifteen.atl(664) 	mus_silence
   jsr mus_silence
;### fifteen.atl(665) 	eraserect 26 140 14 60
   lda #26
   sta eraserect__xscr
   lda #140
   sta eraserect__yscr
   lda #14
   sta eraserect__width
   lda #60
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(667) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(668) 	if cnt=16 shuffletiles showboard
   lda game__cnt
   cmp #16
   jne _lbl131
   jsr shuffletiles
   jsr showboard
;### fifteen.atl(669) 	moves=0
_lbl131:
   lda #0
   sta moves
   lda #0
   sta moves+1
;### fifteen.atl(670) 	start_timer
   jsr start_timer
;### fifteen.atl(672) 	drawbesttime
   jsr drawbesttime
;### fifteen.atl(674) 	gameloop@
gameloop:
;### fifteen.atl(675) 	CH = none
   lda #key__none
   sta CH
;### fifteen.atl(677) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(678) 	textbuf(0,0)="Good tiles:[cnt] "
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
;### fifteen.atl(679) 	text 27 141 13
   lda #27
   sta text__xt
   lda #141
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(681) 	update_timer
   jsr update_timer
;### fifteen.atl(682) 	textbuf(0,0)="Time: [tdmin][tmin]:[tdsec][tsec]"
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
;### fifteen.atl(683) 	text 27 152 11
   lda #27
   sta text__xt
   lda #152
   sta text__yt
   lda #11
   sta text__len
   jsr text
;### fifteen.atl(685) 	if cnt=16
   lda game__cnt
   cmp #16
   jne _lbl132
;### fifteen.atl(686) 		textbuf(0,0)="            "
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
;### fifteen.atl(687) 		text 7 86 12
   lda #7
   sta text__xt
   lda #86
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(688) 		text 7 102 12
   lda #7
   sta text__xt
   lda #102
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(689) 		text 7 114 12
   lda #7
   sta text__xt
   lda #114
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(690) 		windowpmg
   jsr windowpmg
;### fifteen.atl(691) 		textbuf(0,0)=" Well done! "
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
;### fifteen.atl(692) 		text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(693) 		textbuf(0,0)=" Time:[tdmin][tmin]:[tdsec][tsec] "
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
;### fifteen.atl(694) 		text 7 106 12
   lda #7
   sta text__xt
   lda #106
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(695) 		if moves<bmoves
   lda moves
   cmp bmoves
   lda moves+1
   sbc bmoves+1
   jcs _lbl133
;### fifteen.atl(696) 			besttime(0,0)="BESTTIME:[tdmin][tmin]:[tdsec][tsec]"
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
;### fifteen.atl(697) 			sleep 100
   lda #100
   sta sleep__time
   jsr sleep
;### fifteen.atl(698) 			textbuf(0,0)=" NEW RECORD!"
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
;### fifteen.atl(699) 			text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(700) 			textbuf(0,0)=" [moves] MOVES!    "
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
;### fifteen.atl(701) 			text 7 106 12
   lda #7
   sta text__xt
   lda #106
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(702) 			bmoves=moves
   lda moves
   sta bmoves
   lda moves+1
   sta bmoves+1
;### fifteen.atl(703) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(704) 			AUDC1=$68
   lda #104
   sta AUDC1
;### fifteen.atl(711) 			AUDC1=0
   jmp _lbl134
_lbl136:
;### fifteen.atl(706) 				AUDF1=dir
   lda dir
   sta AUDF1
;### fifteen.atl(707) 				setboardcolor RANDOM
   lda RANDOM
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(708) 				sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(709) 				inc dir
   inc dir
_lbl134:
;### fifteen.atl(705) 			while dir<200
   lda dir
   cmp #200
   jcs _lbl135
   jmp _lbl136
_lbl135:
   lda #0
   sta AUDC1
;### fifteen.atl(712) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(713) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(714) 		else
   jmp _lbl137
_lbl133:
;### fifteen.atl(715) 			sleep 200
   lda #200
   sta sleep__time
   jsr sleep
;### fifteen.atl(717) 		CH=28
_lbl137:
   lda #28
   sta CH
;### fifteen.atl(718) 		goto gameend
   jmp gameend
;### fifteen.atl(720) 	textbuf(0,0)="Moves: [moves]     "
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
;### fifteen.atl(721) 	text 27 163 12
   lda #27
   sta text__xt
   lda #163
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(723) 	ss:stick__state = STICK(0)
   lda STICK
   sta game__ss
;### fifteen.atl(725) 	dir=0
   lda #0
   sta dir
;### fifteen.atl(726) 	if ss = 14 dir = 1
   lda game__ss
   cmp #14
   jne _lbl138
   lda #1
   sta dir
;### fifteen.atl(727) 	if ss = 7  dir = 2
_lbl138:
   lda game__ss
   cmp #7
   jne _lbl139
   lda #2
   sta dir
;### fifteen.atl(728) 	if ss = 13 dir = 3
_lbl139:
   lda game__ss
   cmp #13
   jne _lbl140
   lda #3
   sta dir
;### fifteen.atl(729) 	if ss = 11 dir = 4
_lbl140:
   lda game__ss
   cmp #11
   jne _lbl141
   lda #4
   sta dir
;### fifteen.atl(730) 	if dir <> 0 
_lbl141:
   lda dir
   cmp #0
   jeq _lbl142
;### fifteen.atl(731) 		x,y=findempty
   jsr findempty
   lda findempty__x
   sta game__x
   lda findempty__y
   sta game__y
;### fifteen.atl(732) 		dec dir
   dec dir
;### fifteen.atl(733) 		movetile x y dir 1
   lda game__x
   sta movetile__xs
   lda game__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #1
   sta movetile__show
   jsr movetile
;### fifteen.atl(734) 		dir=0
   lda #0
   sta dir
;### fifteen.atl(735) 		sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(737) 	if CH <> 28 goto gameloop  ; ESC 
_lbl142:
   lda CH
   cmp #28
   jeq _lbl143
   jmp gameloop
;### fifteen.atl(739) 	gameend@
_lbl143:
gameend:
;### fifteen.atl(741) 	CH = none
   lda #key__none
   sta CH
   rts
.endp
   icl '../../src/platform/atari/atari.asm'
