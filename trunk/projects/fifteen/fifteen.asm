_arr equ 144
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
sleep__time equ 128
copyblock__srcbm equ 129
copyblock__xbm equ 128
copyblock__ybm equ 131
copyblock__wbm equ 133
copyblock__xscr equ 134
copyblock__yscr equ 135
copyblock__xsize equ 136
copyblock__ysize equ 137
_s2__j equ 138
text__xt equ 128
text__yt equ 129
text__len equ 130
text__ii equ 131
copytile__xbm equ 128
copytile__ybm equ 129
copytile__xscr equ 130
copytile__yscr equ 131
copytile__xsize equ 142
copytile__ysize equ 143
eraserect__xscr equ 128
eraserect__yscr equ 129
eraserect__width equ 130
eraserect__height equ 131
findempty__x equ 128
findempty__y equ 129
gettile__tile equ 128
gettile__tx equ 129
gettile__ty equ 130
_s9__tile equ 128
movetile__xs equ 128
movetile__ys equ 129
movetile__m equ 130
movetile__show equ 131
movetile__tile equ 132
movetile__xoffset equ 133
movetile__yoffset equ 134
movetile__z equ 135
changepicture__c equ 128
countgoodtiles__res equ 128
_s14__tile equ 129
setboardcolor__col equ 128
changecolor__col equ 128
_s19__i equ 128
shuffletiles__c equ 128
shuffletiles__cnt equ 129
_s22__i equ 128
_s23__i equ 129
_s24__i equ 130
_s25__i equ 128
initscreen___94 equ 128
initscreen___95 equ 130
game__cnt equ 128
game__ss equ 129
_101 equ 146
   org $2e0
   dta a($2000)
   org $2000
;### fifteen.atl(158) piccnt@$612:byte= RANDOM bitand 3
;### fifteen.atl(749) PORTB = PORTB bitor 2
;### fifteen.atl(750) initscreen
   jsr initscreen
;### fifteen.atl(751) besttime(0)=0
;### fifteen.atl(752) bmoves=999
;### fifteen.atl(758) 	game
   jmp _lbl148
_lbl150:
;### fifteen.atl(754) 	titlescreen
   jsr titlescreen
;### fifteen.atl(755) 	game
   jsr game
_lbl148:
;### fifteen.atl(753) while 1=1
   lda #1
   cmp #1
   jeq _lbl150
_lbl149:
   jmp _lbl149
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
;### fifteen.atl(53) 		wait'line
   sta WSYNC
;### fifteen.atl(54) 		player_col2(3) = $9e
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
;### fifteen.atl(59) 		wait'line
   sta WSYNC
;### fifteen.atl(60) 		player_col2(3) = $8e
   lda #142
   sta player_col2+3
;### fifteen.atl(61) 		COLPF(3) = $86
   lda #134
   sta COLPF+3
;### fifteen.atl(62) 		wait'line
   sta WSYNC
;### fifteen.atl(63) 		player_col2(3) = $5e
   lda #94
   sta player_col2+3
;### fifteen.atl(64) 		COLPF(3) = $56
   lda #86
   sta COLPF+3
;### fifteen.atl(65) 		wait'line
   sta WSYNC
;### fifteen.atl(66) 		player_col2(3) = $4e
   lda #78
   sta player_col2+3
;### fifteen.atl(67) 		COLPF(3) = $46
   lda #70
   sta COLPF+3
;### fifteen.atl(68) 	else if vcnt = 1
   jmp _lbl6
_lbl1:
   lda vcnt
   cmp #1
   jne _lbl3
;### fifteen.atl(69) 		wait'line
   sta WSYNC
;### fifteen.atl(70) 		COLPF(3) = $4e
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
;### fifteen.atl(75) 	else if vcnt = 2
   jmp _lbl6
_lbl3:
   lda vcnt
   cmp #2
   jne _lbl4
;### fifteen.atl(76) 		wait'line
   sta WSYNC
;### fifteen.atl(77) 		player_col2(3) = $4c
   lda #76
   sta player_col2+3
;### fifteen.atl(78) 		wait'line
   sta WSYNC
;### fifteen.atl(79) 		player_col2(3) = $4a
   lda #74
   sta player_col2+3
;### fifteen.atl(80) 		wait'line
   sta WSYNC
;### fifteen.atl(81) 		player_col2(3) = $48
   lda #72
   sta player_col2+3
;### fifteen.atl(82) 		wait'line
   sta WSYNC
;### fifteen.atl(83) 		player_col2(3) = $46
   lda #70
   sta player_col2+3
;### fifteen.atl(84) 		wait'line
   sta WSYNC
;### fifteen.atl(85) 		player_col2(3) = $44
   lda #68
   sta player_col2+3
;### fifteen.atl(86) 		wait'line
   sta WSYNC
;### fifteen.atl(87) 		player_col2(3) = $42
   lda #66
   sta player_col2+3
;### fifteen.atl(88) 		inc vcnt
;### fifteen.atl(89) 	else if vcnt = 3
   jmp _lbl6
_lbl4:
   lda vcnt
   cmp #3
   jne _lbl5
;### fifteen.atl(90) 		wait'line
   sta WSYNC
;### fifteen.atl(91) 		player_col2(3) = $56
   lda #86
   sta player_col2+3
;### fifteen.atl(92) 		COLPF(3) = $56
   sta COLPF+3
;### fifteen.atl(93) 		wait'line
   sta WSYNC
;### fifteen.atl(94) 		COLPF(3) = $84
   lda #132
   sta COLPF+3
;### fifteen.atl(95) 		player_col2(3) = $86
   lda #134
   sta player_col2+3
;### fifteen.atl(96) 		wait'line
   sta WSYNC
;### fifteen.atl(97) 		COLPF(3) = $94
   lda #148
   sta COLPF+3
;### fifteen.atl(98) 		player_col2(3) = $96
   lda #150
   sta player_col2+3
;### fifteen.atl(99) 		inc vcnt
;### fifteen.atl(100) 	else if vcnt = 4
   jmp _lbl6
_lbl5:
   lda vcnt
   cmp #4
   jne _lbl6
;### fifteen.atl(101) 		wait'line
   sta WSYNC
;### fifteen.atl(102) 		COLPF(3) = $9a
   lda #154
   sta COLPF+3
;### fifteen.atl(103) 		player_col2(3) = $9c
   lda #156
   sta player_col2+3
;### fifteen.atl(104) 		inc vcnt
_lbl6:
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
;### fifteen.atl(174) 	tmp = tmp + time
   clc
   adc sleep__time
   sta drawmainscreen__tmp
;### fifteen.atl(177) 	while timer<>tmp
   jmp _lbl9
_lbl9:
;### fifteen.atl(175) 	while timer<>tmp
   lda timer
   cmp drawmainscreen__tmp
   jne _lbl9
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
_lbl15:
;### fifteen.atl(184) 		xbtmp=i
   lda i
;### fifteen.atl(185) 		xbtmp=xbtmp+xscr
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
;### fifteen.atl(191) 		ctmp1 = bptr
;### fifteen.atl(192) 		ctmp1= ctmp1+ xctmp
   lda bptr
   clc
   adc xctmp+0
   sta ctmp1
   lda bptr+1
   adc xctmp+1
   sta ctmp1+1
;### fifteen.atl(193) 		ctmp1= ctmp1+ yctmp
   lda ctmp1
   clc
   adc copyblock__ybm+0
   sta ctmp1
   lda ctmp1+1
   adc copyblock__ybm+1
   sta ctmp1+1
;### fifteen.atl(194) 		for j:1..ysize
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
   tax
   ldy ybtmp
   lda screen_lo,y
   sta _arr
   lda screen_hi,y
   sta _arr+1
   ldy xbtmp
   txa
   sta (_arr),y
;### fifteen.atl(196) 			ctmp1 = ctmp1 + wbm
   lda ctmp1
   clc
   adc copyblock__wbm
   sta ctmp1
   jcc _lbl151
   inc ctmp1+1
_lbl151:
;### fifteen.atl(197) 			inc ybtmp
   inc ybtmp
   inc _s2__j
   jcs _lbl14
   lda copyblock__ysize
   cmp _s2__j
   jcs _lbl13
_lbl14:
;### fifteen.atl(199) 		inc i
   inc i
_lbl10:
;### fifteen.atl(182) 	while i<xsize 
   lda i
   cmp copyblock__xsize
   jcc _lbl15
_lbl11:
   rts
.endp
text .proc
;### fifteen.atl(203) 	ii:byte=0
   lda #0
   sta text__ii
;### fifteen.atl(211) 		inc ii
   jmp _lbl16
_lbl18:
;### fifteen.atl(205) 		ctmp1=textbuf(ii)
   ldx text__ii
;### fifteen.atl(206) 		ctmp1=ctmp1*8
   lda textbuf,x
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #8
   sta _TEMPW2
   lda #0
   sta _TEMPW2+1
   jsr _sys_mul16
;### fifteen.atl(207) 		btmp1= xt+ii
   lda text__xt
   clc
   adc text__ii
   sta btmp1
;### fifteen.atl(208) 		copyblock fonts 0 ctmp1 1 btmp1 yt 1 8
   lda #<fonts
   sta copyblock__srcbm
   lda #>fonts
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   lda _TEMPL1
   sta copyblock__ybm
   lda _TEMPL1+1
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
_lbl16:
;### fifteen.atl(204) 	while ii<len
   lda text__ii
   cmp text__len
   jcc _lbl18
_lbl17:
   rts
.endp
copytile .proc
;### fifteen.atl(213) 	ctmp1=ybm*24
   lda copytile__ybm
   ldx #24
   jsr _sys_mul8
;### fifteen.atl(214) 	ctmp1=ctmp1*48
   lda _TEMPW2
   sta _TEMPW1
   lda _TEMPW2+1
   sta _TEMPW1+1
   lda #48
   sta _TEMPW2
   lda #0
   sta _TEMPW2+1
   jsr _sys_mul16
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
   lda _TEMPL1
   sta copyblock__ybm
   lda _TEMPL1+1
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
   jmp _lbl19
_lbl24:
;### fifteen.atl(221) 		xbtmp=i+xscr
   lda i
   clc
   adc eraserect__xscr
   sta xbtmp
;### fifteen.atl(222) 		ybtmp=yscr
   lda eraserect__yscr
   sta ybtmp
;### fifteen.atl(223) 		j=0
   ldx #0
;### fifteen.atl(228) 		inc i
   jmp _lbl21
_lbl23:
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
   inx
_lbl21:
;### fifteen.atl(224) 		while j<height
   cpx eraserect__height
   jcc _lbl23
_lbl22:
   inc i
_lbl19:
;### fifteen.atl(220) 	while i<width
   lda i
   cmp eraserect__width
   jcc _lbl24
_lbl20:
   rts
.endp
findempty .proc
;### fifteen.atl(231) 	x=0
   lda #0
   sta findempty__x
;### fifteen.atl(232) 	y=0
   sta findempty__y
;### fifteen.atl(233) 	for k
   sta k
_lbl29:
;### fifteen.atl(234) 		for l
   lda #0
   sta l
   ldx findempty__x
_lbl28:
;### fifteen.atl(235) 			if playfield(k,l) = T_EMPTY
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   lda (_arr),y
   cmp #root__T_EMPTY
   jne _lbl27
;### fifteen.atl(236) 				x=k
   ldx k
;### fifteen.atl(237) 				y=l
   lda l
   sta findempty__y
_lbl27:
   inc l
   lda l
   cmp #4
   jne _lbl28
   stx findempty__x
   inc k
   lda k
   cmp #4
   jne _lbl29
   rts
.endp
gettile .proc
;### fifteen.atl(241) 	ty = tile bitand 3
   lda gettile__tile
   and #3
   sta gettile__ty
;### fifteen.atl(242) 	tx = tile / 4
   lda gettile__tile
   lsr
   lsr
;### fifteen.atl(243) 	tx = tx bitand 3
   and #3
   sta gettile__tx
   rts
.endp
showboard .proc
;### fifteen.atl(246) 	for k
   lda #0
   sta k
_lbl35:
;### fifteen.atl(247) 		for l
   lda #0
   sta l
_lbl34:
;### fifteen.atl(248) 			xbtmp = 6*k+XOFF
   lda k
   asl
   clc
   adc k
   asl
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
   cmp #root__T_EMPTY
   jeq _lbl32
;### fifteen.atl(252) 				i,j=gettile tile
   lda _s9__tile
   sta gettile__tile
   jsr gettile
;### fifteen.atl(253) 				copytile i j xbtmp ybtmp 6 48
   lda gettile__tx
   sta copytile__xbm
   lda gettile__ty
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
   jmp _lbl33
_lbl32:
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
_lbl33:
   inc l
   lda l
   cmp #4
   jne _lbl34
   inc k
   lda k
   cmp #4
   jne _lbl35
   rts
.endp
movetile .proc
;### fifteen.atl(262) 	xoffset=1
   lda #1
   sta movetile__xoffset
;### fifteen.atl(263) 	yoffset=1
   sta movetile__yoffset
;### fifteen.atl(264) 	AUDC1=$22
   lda #34
   sta AUDC1
;### fifteen.atl(265) 	if m = D_UP
   lda movetile__m
   cmp #root__D_UP
   jne _lbl37
;### fifteen.atl(266) 		if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl37
;### fifteen.atl(267) 			yoffset=2
   lda #2
   sta movetile__yoffset
_lbl37:
;### fifteen.atl(269) 	if m = D_RIGHT
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl39
;### fifteen.atl(270) 		if xs>0
   lda #0
   cmp movetile__xs
   jcs _lbl39
;### fifteen.atl(271) 			xoffset=0
   lda #0
   sta movetile__xoffset
_lbl39:
;### fifteen.atl(273) 	if m = D_DOWN
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl41
;### fifteen.atl(274) 		if ys>0
   lda #0
   cmp movetile__ys
   jcs _lbl41
;### fifteen.atl(275) 			yoffset=0
   lda #0
   sta movetile__yoffset
_lbl41:
;### fifteen.atl(277) 	if m = D_LEFT
   lda movetile__m
   cmp #root__D_LEFT
   jne _lbl43
;### fifteen.atl(278) 		if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl43
;### fifteen.atl(279) 			xoffset=2
   lda #2
   sta movetile__xoffset
_lbl43:
;### fifteen.atl(281) 	if xoffset <>1 or yoffset <>1
   lda movetile__xoffset
   cmp #1
   jne _lbl45
_lbl44:
   lda movetile__yoffset
   cmp #1
   jeq _lbl47
_lbl45:
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
   jne _lbl152
   inc moves+1
_lbl152:
;### fifteen.atl(290) 		if moves > 999 moves=999
   lda moves+1
   cmp #3
   jcc _lbl47
   jne _lbl153
   lda moves
   cmp #231
   jcc _lbl47
_lbl153:
_lbl47:
;### fifteen.atl(292) 	xoffset=0
   lda #0
   sta movetile__xoffset
;### fifteen.atl(293) 	yoffset=0
   sta movetile__yoffset
;### fifteen.atl(294) 	if show >0
   cmp movetile__show
   jcs _lbl51
;### fifteen.atl(295) 		z=0
   lda #0
   sta movetile__z
;### fifteen.atl(296) 		if show=2 z=5
   lda movetile__show
   cmp #2
   jne _lbl50
   lda #5
   sta movetile__z
_lbl49:
;### fifteen.atl(361) 	AUDC1=0
   jmp _lbl50
_lbl68:
;### fifteen.atl(299) 			xbtmp=xs*6+XOFF
   lda movetile__xs
   ldx #6
   jsr _sys_mul8
   lda _TEMPW2
   clc
   adc #root__XOFF
   sta xbtmp
;### fifteen.atl(300) 			ybtmp=ys*48+YOFF
   lda movetile__ys
   ldx #48
   jsr _sys_mul8
   lda _TEMPW2
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
   jne _lbl53
;### fifteen.atl(306) 				if xs<3
   lda movetile__xs
   cmp #3
   jcs _lbl53
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
;### fifteen.atl(310) 					btmp1 = btmp1+xoffset
   clc
   adc movetile__xoffset
   sta btmp1
;### fifteen.atl(311) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc #0
   sta btmp2
;### fifteen.atl(312) 					xtilesize=1
   lda #1
   sta xtilesize
_lbl53:
;### fifteen.atl(314) 			if m = D_DOWN
   lda movetile__m
   cmp #root__D_DOWN
   jne _lbl55
;### fifteen.atl(315) 				if ys>0
   lda #0
   cmp movetile__ys
   jcs _lbl55
;### fifteen.atl(316) 					yoffset=z-5
   lda movetile__z
   sec
   sbc #5
;### fifteen.atl(317) 					yoffset = yoffset * 8
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
   adc #0
   sta btmp1
;### fifteen.atl(320) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
;### fifteen.atl(321) 					btmp2=btmp2-8
   sec
   sbc #8
   sta btmp2
;### fifteen.atl(322) 					if z=5 btmp2 = btmp2-40
   lda movetile__z
   cmp #5
   jne _lbl56
   lda btmp2
   sec
   sbc #40
   sta btmp2
_lbl56:
;### fifteen.atl(323) 					ytilesize=8
   lda #8
   sta ytilesize
_lbl55:
;### fifteen.atl(324) 			if m = D_RIGHT
   lda movetile__m
   cmp #root__D_RIGHT
   jne _lbl58
;### fifteen.atl(325) 				if xs>0
   lda #0
   cmp movetile__xs
   jcs _lbl58
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
   jne _lbl59
   lda btmp1
   sec
   sbc #5
   sta btmp1
_lbl59:
;### fifteen.atl(331) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(332) 					xtilesize=1
   lda #1
   sta xtilesize
_lbl58:
;### fifteen.atl(334) 			if m = D_UP
   lda movetile__m
   cmp #root__D_UP
   jne _lbl61
;### fifteen.atl(335) 				if ys<3
   lda movetile__ys
   cmp #3
   jcs _lbl61
;### fifteen.atl(336) 					yoffset=5-z
   lda #5
   sec
   sbc movetile__z
;### fifteen.atl(337) 					yoffset = yoffset * 8
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
   adc #0
   sta btmp1
;### fifteen.atl(340) 					btmp2=ybtmp+yoffset
   lda ybtmp
   clc
   adc movetile__yoffset
;### fifteen.atl(341) 					btmp2=btmp2+48
   clc
   adc #48
   sta btmp2
;### fifteen.atl(342) 					ytilesize=8
   lda #8
   sta ytilesize
_lbl61:
;### fifteen.atl(344) 			if xtilesize<>6 or ytilesize<>48
   lda xtilesize
   cmp #6
   jne _lbl63
_lbl62:
   lda ytilesize
   cmp #48
   jeq _lbl64
_lbl63:
;### fifteen.atl(345) 				if z=5 xtilesize=6 ytilesize=48 
   lda movetile__z
   cmp #5
   jne _lbl65
   lda #6
   sta xtilesize
   lda #48
   sta ytilesize
_lbl65:
;### fifteen.atl(346) 				eraserect btmp1 btmp2 xtilesize ytilesize
   lda btmp1
   sta eraserect__xscr
   lda btmp2
   sta eraserect__yscr
   lda xtilesize
   sta eraserect__width
   lda ytilesize
   sta eraserect__height
   jsr eraserect
_lbl64:
;### fifteen.atl(349) 			tile = playfield(xs,ys)
   ldy movetile__ys
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy movetile__xs
   lda (_arr),y
   sta movetile__tile
;### fifteen.atl(350) 			if tile <> T_EMPTY
   cmp #root__T_EMPTY
   jeq _lbl67
;### fifteen.atl(351) 				if show >0
   lda #0
   cmp movetile__show
   jcs _lbl67
;### fifteen.atl(352) 					k,l = gettile tile
   lda movetile__tile
   sta gettile__tile
   jsr gettile
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
;### fifteen.atl(357) 					btmp2=btmp2+YOFF
   clc
   adc #root__YOFF
;### fifteen.atl(358) 					btmp2=btmp2+yoffset
   clc
   adc movetile__yoffset
   sta btmp2
;### fifteen.atl(359) 					copytile k l btmp1 btmp2 6 48
   lda gettile__tx
   sta copytile__xbm
   lda gettile__ty
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
_lbl67:
;### fifteen.atl(360) 			inc z
   inc movetile__z
_lbl50:
;### fifteen.atl(297) 		while z<6
   lda movetile__z
   cmp #6
   jcc _lbl68
_lbl51:
   lda #0
   sta AUDC1
   rts
.endp
clrscr .proc
;### fifteen.atl(365) 	i=0
   lda #0
   sta i
;### fifteen.atl(373) 		inc i
   jmp _lbl69
_lbl74:
;### fifteen.atl(367) 		j=0
   lda #0
   sta j
;### fifteen.atl(371) 		inc i
   jmp _lbl71
_lbl73:
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
_lbl71:
;### fifteen.atl(368) 		while j<200
   lda j
   cmp #200
   jcc _lbl73
_lbl72:
   inc i
_lbl69:
;### fifteen.atl(366) 	while i<40
   lda i
   cmp #40
   jcc _lbl74
_lbl70:
   rts
.endp
changepicture .proc
;### fifteen.atl(375) 	if piccnt=0
   lda piccnt
   cmp #0
   jne _lbl75
;### fifteen.atl(376) 		currentbuf=buf1
;### fifteen.atl(377) 		c=$1c
   lda #28
   sta changepicture__c
_lbl75:
;### fifteen.atl(378) 	if piccnt=1
   lda piccnt
   cmp #1
   jne _lbl76
;### fifteen.atl(379) 		currentbuf=buf2
;### fifteen.atl(380) 		c=$0e
   lda #14
   sta changepicture__c
_lbl76:
;### fifteen.atl(382) 	if piccnt=2
   lda piccnt
   cmp #2
   jne _lbl77
;### fifteen.atl(383) 		currentbuf=buf3
;### fifteen.atl(384) 		c=$3a
   lda #58
   sta changepicture__c
_lbl77:
;### fifteen.atl(385) 	if piccnt=3
   lda piccnt
   cmp #3
   jne _lbl78
;### fifteen.atl(386) 		currentbuf=buf4
;### fifteen.atl(387) 		c=$7a
   lda #122
   sta changepicture__c
_lbl78:
;### fifteen.atl(389) 	inc piccnt
   inc piccnt
;### fifteen.atl(390) 	if piccnt>=4 piccnt=0
   lda piccnt
   cmp #4
   jcc _lbl79
_lbl79:
   rts
.endp
countgoodtiles .proc
;### fifteen.atl(395) 	res=0
   lda #0
   sta countgoodtiles__res
;### fifteen.atl(396) 	for k
   sta k
_lbl84:
;### fifteen.atl(397) 		for l
   lda #0
   sta l
   ldx countgoodtiles__res
_lbl83:
;### fifteen.atl(398) 			tile=k*4
   lda k
   asl
   asl
;### fifteen.atl(399) 			tile=tile+l
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
   cmp _s14__tile
   jne _lbl82
   inx
_lbl82:
   inc l
   lda l
   cmp #4
   jne _lbl83
   stx countgoodtiles__res
   inc k
   lda k
   cmp #4
   jne _lbl84
   rts
.endp
inittiles .proc
;### fifteen.atl(403) 	for k
   lda #0
   sta k
_lbl88:
;### fifteen.atl(404) 		for l
   lda #0
   sta l
_lbl87:
;### fifteen.atl(405) 			tile=k*4
   lda k
   asl
   asl
;### fifteen.atl(406) 			tile=tile+l
   clc
   adc l
   tax
;### fifteen.atl(407) 			playfield(k,l)=tile
   ldy l
   lda playfield_lo,y
   sta _arr
   lda playfield_hi,y
   sta _arr+1
   ldy k
   txa
   sta (_arr),y
   inc l
   lda l
   cmp #4
   jne _lbl87
   inc k
   lda k
   cmp #4
   jne _lbl88
   rts
.endp
setboardcolor .proc
;### fifteen.atl(410) 	player_color(0)=col
   lda setboardcolor__col
   sta player_color
;### fifteen.atl(411) 	player_color(1)=col
   sta player_color+1
;### fifteen.atl(412) 	player_color(2)=col
   sta player_color+2
   rts
.endp
changecolor .proc
;### fifteen.atl(416) 	btmp1=col bitand $f
   lda changecolor__col
   and #15
   sta btmp1
;### fifteen.atl(417) 	btmp2=col bitand $f0
   lda changecolor__col
   and #240
   sta btmp2
;### fifteen.atl(419) 	xbtmp=currcol bitand $f
   lda currcol
   and #15
   sta xbtmp
;### fifteen.atl(420) 	ybtmp=currcol bitand $f0
   lda currcol
   and #240
   sta ybtmp
;### fifteen.atl(422) 	if xbtmp < btmp1 ; fade in
   lda xbtmp
   cmp btmp1
   jcs _lbl94
;### fifteen.atl(429) 	else ; fade out
   jmp _lbl90
_lbl92:
;### fifteen.atl(424) 			xbtmp = xbtmp + 2
   lda xbtmp
   clc
   adc #2
   sta xbtmp
;### fifteen.atl(425) 			currcol = btmp2
   lda btmp2
;### fifteen.atl(426) 			currcol = currcol + xbtmp
   clc
   adc xbtmp
;### fifteen.atl(427) 			setboardcolor currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(428) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
_lbl90:
;### fifteen.atl(423) 		while btmp1 > xbtmp
   lda xbtmp
   cmp btmp1
   jcc _lbl92
_lbl91:
   jmp _lbl95
_lbl96:
;### fifteen.atl(431) 			xbtmp = xbtmp -2
   lda xbtmp
   sec
   sbc #2
   sta xbtmp
;### fifteen.atl(432) 			currcol = ybtmp
   lda ybtmp
;### fifteen.atl(433) 			currcol = currcol + xbtmp
   clc
   adc xbtmp
;### fifteen.atl(434) 			setboardcolor currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(435) 			sleep 1
   lda #1
   sta sleep__time
   jsr sleep
_lbl94:
;### fifteen.atl(430) 		while btmp1 < xbtmp
   lda btmp1
   cmp xbtmp
   jcc _lbl96
_lbl95:
   rts
.endp
drawmainscreen .proc
;### fifteen.atl(439) 	copyblock mains 0 0 14 26 0 14 200
   lda #<mains
   sta copyblock__srcbm
   lda #>mains
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   sta copyblock__ybm
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
;### fifteen.atl(441) 	for i:0..3 
   sta _s19__i
_lbl98:
;### fifteen.atl(443) 		xbtmp=i*6
   lda _s19__i
   asl
   clc
   adc _s19__i
   asl
   sta xbtmp
;### fifteen.atl(444) 		inc xbtmp
   inc xbtmp
;### fifteen.atl(445) 		copyblock leafh 0 0 6 xbtmp 0 6 8
   lda #<leafh
   sta copyblock__srcbm
   lda #>leafh
   sta copyblock__srcbm+1
   lda #0
   sta copyblock__xbm
   sta copyblock__ybm
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
;### fifteen.atl(446) 		xbtmp=8+j
   lda j
   clc
   adc #8
   sta xbtmp
;### fifteen.atl(447) 		copyblock leafv 0 0 1 0 xbtmp 1 48
   lda #<leafv
   sta copyblock__srcbm
   lda #>leafv
   sta copyblock__srcbm+1
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
;### fifteen.atl(448) 		xbtmp=8+j
   clc
;### fifteen.atl(449) 		copyblock leafv 0 0 1 25 xbtmp 1 48
   lda #25
   sta copyblock__xscr
   jsr copyblock
;### fifteen.atl(450) 		j=j+48
   lda j
   clc
   adc #48
   sta j
   inc _s19__i
   lda _s19__i
   cmp #4
   jne _lbl98
;### fifteen.atl(453) 	tmp:byte = changepicture
   jmp _lbl101
_lbl101:
;### fifteen.atl(451) 	while 1=1
   lda #1
   cmp #1
   jeq _lbl101
_lbl100:
   jsr changepicture
   lda changepicture__c
;### fifteen.atl(454) 	showboard
   jsr showboard
;### fifteen.atl(455) 	currcol=0
;### fifteen.atl(456) 	changecolor tmp
   sta changecolor__col
   jsr changecolor
   rts
.endp
shuffletiles .proc
;### fifteen.atl(459) 	c:byte=0
   lda #0
   sta shuffletiles__c
loop:
;### fifteen.atl(461) 	dir=RANDOM bitand 3
   lda RANDOM
   and #3
   sta dir
;### fifteen.atl(462) 	x,y=findempty
   jsr findempty
;### fifteen.atl(463) 	movetile x y dir 2
   lda findempty__x
   sta movetile__xs
   lda findempty__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #2
   sta movetile__show
   jsr movetile
;### fifteen.atl(464) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta shuffletiles__cnt
;### fifteen.atl(465) 	if c<50 inc c
   lda shuffletiles__c
   cmp #50
   jcs _lbl102
   inc shuffletiles__c
_lbl102:
;### fifteen.atl(466) 	if cnt>0 or c<50 goto loop
   lda #0
   cmp shuffletiles__cnt
   jcc loop
_lbl103:
   lda shuffletiles__c
   cmp #50
   jcc loop
_lbl104:
_lbl105:
   rts
.endp
cycle .proc
;### fifteen.atl(470) 	if play = 1
   lda play
   cmp #1
   jne _lbl106
;### fifteen.atl(471) 		mus_play
   jsr mus_play
_lbl106:
;### fifteen.atl(472) 	vcnt=0
   jmp $e462
.endp
fillpmg .proc
;### fifteen.atl(475) 	ctmp1 = $180
   lda #1
   sta ctmp1+1
   ldy #128
;### fifteen.atl(481) 	for i:16..111
   jmp _lbl107
_lbl109:
;### fifteen.atl(478) 		pmg_mem(i)=$0
   lda #0
   sta pmg_mem,x
;### fifteen.atl(479) 		inc ctmp1
   iny
   jne _lbl154
   inc ctmp1+1
_lbl154:
_lbl107:
;### fifteen.atl(477) 	while ctmp1<=$3ff
   lda ctmp1+1
   cmp #3
   jeq _lbl155
   jcs _lbl108
_lbl155:
   cpy #255
   jeq _lab2
   jcs _lbl108
_lab2:
   jmp _lbl109
_lbl108:
   lda #16
   sta _s22__i
_lbl111:
;### fifteen.atl(482) 		ctmp1=$200
   lda #2
   sta ctmp1+1
;### fifteen.atl(483) 		ctmp1 = ctmp1+i
   lda #0
   clc
   adc _s22__i
   sta ctmp1
   jcc _lbl156
   inc ctmp1+1
_lbl156:
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
   jcc _lbl157
   inc ctmp1+1
_lbl157:
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
   jcc _lbl158
   inc ctmp1+1
_lbl158:
;### fifteen.atl(488) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(489) 		ctmp1=ctmp1+ $80
   clc
   jcc _lbl159
_lbl159:
   inc _s22__i
   lda _s22__i
   cmp #112
   jne _lbl111
;### fifteen.atl(491) 	for i:12..115
   lda #12
   sta _s23__i
_lbl113:
;### fifteen.atl(492) 		ctmp1=$180
   lda #1
   sta ctmp1+1
;### fifteen.atl(493) 		ctmp1=ctmp1 + i
   lda #128
   clc
   adc _s23__i
   sta ctmp1
   jcc _lbl160
   inc ctmp1+1
_lbl160:
;### fifteen.atl(494) 		pmg_mem(ctmp1)=$ff
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #255
   sta (_arr),y
;### fifteen.atl(495) 		ctmp1=ctmp1+ $200
   lda ctmp1
   clc
   adc #0
   sta ctmp1
   lda ctmp1+1
   adc #2
   sta ctmp1+1
;### fifteen.atl(496) 		pmg_mem(ctmp1)=$fe
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #254
   sta (_arr),y
   inc _s23__i
   lda _s23__i
   cmp #116
   jne _lbl113
;### fifteen.atl(498) 	for i:75..79
   lda #75
   sta _s24__i
_lbl115:
;### fifteen.atl(499) 		ctmp1=$380
   lda #3
   sta ctmp1+1
;### fifteen.atl(500) 		ctmp1= ctmp1+ i
   lda #128
   clc
   adc _s24__i
   sta ctmp1
   jcc _lbl161
   inc ctmp1+1
_lbl161:
;### fifteen.atl(501) 		pmg_mem(ctmp1)=$00
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
   inc _s24__i
   lda _s24__i
   cmp #80
   jne _lbl115
   rts
.endp
windowpmg .proc
;### fifteen.atl(508) 	for i:0..17
   lda #0
   sta _s25__i
_lbl117:
;### fifteen.atl(509) 		ctmp1=$237
   lda #2
   sta ctmp1+1
;### fifteen.atl(510) 		ctmp1 = ctmp1 +i
   lda #55
   clc
   adc _s25__i
   sta ctmp1
   jcc _lbl162
   inc ctmp1+1
_lbl162:
;### fifteen.atl(511) 		pmg_mem(ctmp1)=$fc
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #252
   sta (_arr),y
;### fifteen.atl(512) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl163
   inc ctmp1+1
_lbl163:
;### fifteen.atl(513) 		pmg_mem(ctmp1)=$0
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #0
   sta (_arr),y
;### fifteen.atl(514) 		ctmp1 = ctmp1 +$80
   lda ctmp1
   clc
   adc #128
   sta ctmp1
   jcc _lbl164
   inc ctmp1+1
_lbl164:
;### fifteen.atl(515) 		pmg_mem(ctmp1)=$3f
   lda #<pmg_mem
   sta _arr
   lda #>pmg_mem
   clc
   adc ctmp1+1
   sta _arr+1
   ldy ctmp1+0
   lda #63
   sta (_arr),y
   inc _s25__i
   lda _s25__i
   cmp #18
   jne _lbl117
   rts
.endp
setpmg .proc
;### fifteen.atl(519) 	fillpmg
   jsr fillpmg
;### fifteen.atl(521) 	PMBASE= $9c
   lda #156
   sta PMBASE
;### fifteen.atl(522) 	GRACTL=3
   lda #3
   sta GRACTL
;### fifteen.atl(524) 	GTICTLS=$18
   lda #24
   sta GTICTLS
;### fifteen.atl(526) 	player_x(0)=52
   lda #52
   sta player_x
;### fifteen.atl(527) 	player_x(1)=84
   lda #84
   sta player_x+1
;### fifteen.atl(528) 	player_x(2)=116
   lda #116
   sta player_x+2
;### fifteen.atl(529) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(530) 	player_size(0)=3
   lda #3
   sta player_size
;### fifteen.atl(531) 	player_size(1)=3
   sta player_size+1
;### fifteen.atl(532) 	player_size(2)=3
   sta player_size+2
;### fifteen.atl(533) 	player_size(3)=3
   sta player_size+3
;### fifteen.atl(534) 	missile_x(0)=152
   lda #152
   sta missile_x
;### fifteen.atl(535) 	missile_x(1)=160
   lda #160
   sta missile_x+1
;### fifteen.atl(536) 	player_x(3)=166
   lda #166
   sta player_x+3
;### fifteen.atl(537) 	missile_x(2)=192
   lda #192
   sta missile_x+2
;### fifteen.atl(538) 	missile_x(3)=200
   lda #200
   sta missile_x+3
;### fifteen.atl(539) 	player_color(3)=$98
   lda #152
   sta player_color+3
;### fifteen.atl(540) 	COLOR0(3)=$96
   lda #150
   sta COLOR0+3
;### fifteen.atl(541) 	missile_size=$ff
   lda #255
   sta missile_size
;### fifteen.atl(542) 	DMACTL =  $2e
   lda #46
   sta DMACTL
   rts
.endp
start_timer .proc
;### fifteen.atl(548) 	timer=0
;### fifteen.atl(549) 	tsec=0
;### fifteen.atl(550) 	tdsec=0
;### fifteen.atl(551) 	tmin=0
;### fifteen.atl(552) 	tdmin=0
   rts
.endp
update_timer .proc
;### fifteen.atl(556) 	if timer>49
   lda #49
   cmp timer
   jcs _lbl118
;### fifteen.atl(557) 		timer = timer -50
   sec
;### fifteen.atl(558) 		inc tsec
   inc tsec
_lbl118:
;### fifteen.atl(560) 	if tsec=10
   lda tsec
   cmp #10
   jne _lbl119
;### fifteen.atl(561) 		tsec=0
;### fifteen.atl(562) 		inc tdsec
   inc tdsec
_lbl119:
;### fifteen.atl(564) 	if tdsec=6
   lda tdsec
   cmp #6
   jne _lbl120
;### fifteen.atl(565) 		tdsec=0
;### fifteen.atl(566) 		inc tmin
   inc tmin
_lbl120:
;### fifteen.atl(568) 	if tmin=10
   lda tmin
   cmp #10
   jne _lbl121
;### fifteen.atl(569) 		tmin=0
;### fifteen.atl(570) 		inc tdmin
   inc tdmin
_lbl121:
;### fifteen.atl(572) 	if tdmin=6
   lda tdmin
   cmp #6
   jne _lbl122
;### fifteen.atl(573) 		tdmin=5
;### fifteen.atl(574) 		tmin=9
;### fifteen.atl(575) 		tdsec=5
;### fifteen.atl(576) 		tsec=9
_lbl122:
   rts
.endp
drawbesttime .proc
;### fifteen.atl(579) 	if besttime(0,0) <> 0
   ldy #0
   lda besttime_lo,y
   sta _arr
   lda besttime_hi,y
   sta _arr+1
   lda (_arr),y
   cmp #0
   jeq _lbl123
;### fifteen.atl(581) 		tcnt=0
   ldx #0
;### fifteen.atl(585) 		text 26 181 14
   jmp _lbl124
_lbl126:
;### fifteen.atl(583) 			textbuf(tcnt)=besttime(tcnt)
;### fifteen.atl(584) 			inc tcnt
   inx
_lbl124:
;### fifteen.atl(582) 		while tcnt<14
   cpx #14
   jcc _lbl126
_lbl125:
   lda #26
   sta text__xt
   lda #181
   sta text__yt
   lda #14
   sta text__len
   jsr text
;### fifteen.atl(587) 		textbuf(0,0)="IN [bmoves] MOVES!   "
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
;### fifteen.atl(588) 		text 27 192 13
   lda #27
   sta text__xt
   lda #192
   sta text__yt
   lda #13
   sta text__len
   jsr text
_lbl123:
   rts
.endp
initscreen .proc
;### fifteen.atl(594) 	sdlstl = dl
   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
;### fifteen.atl(595) 	currentbuf=buf1
;### fifteen.atl(600) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(602) 	setpmg
   jsr setpmg
;### fifteen.atl(603) 	play=0
;### fifteen.atl(604) 	on'vbi cycle
   lda #<cycle
   sta initscreen___94
   lda #>cycle
   sta initscreen___94+1
   lda initscreen___94
   sta VVBLKD
   lda initscreen___94+1
   sta VVBLKD+1
;### fifteen.atl(605) 	on'dli set_col
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
;### fifteen.atl(609) 	clrscr
   jsr clrscr
;### fifteen.atl(610) 	piccnt = RANDOM bitand 3
;### fifteen.atl(611) 	inittiles
   jsr inittiles
;### fifteen.atl(612) 	drawmainscreen
   jsr drawmainscreen
;### fifteen.atl(613) 	fillpmg
   jsr fillpmg
;### fifteen.atl(614) 	DMACTL =  $2e
   lda #46
   sta DMACTL
;### fifteen.atl(616) 	mus_setpokey
   jsr mus_setpokey
;### fifteen.atl(617) 	CH=62
   lda #62
   sta CH
;### fifteen.atl(619) 	COLOR0(1) = $c0
   lda #192
   sta COLOR0+1
;### fifteen.atl(620) 	COLOR0(2) = $c8
   lda #200
   sta COLOR0+2
;### fifteen.atl(621) 	COLOR0(4) = $c0
   lda #192
   sta COLOR0+4
;### fifteen.atl(623) 	drawbesttime
   jsr drawbesttime
loop:
;### fifteen.atl(626) 	if CH = 28  ; ESC 
   lda CH
   cmp #28
   jne _lbl128
;### fifteen.atl(627) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(628) 		ctmp=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
;### fifteen.atl(629) 		if ctmp<16
   cmp #16
   jcs _lbl128
;### fifteen.atl(630) 			ctmp=currcol
;### fifteen.atl(631) 			changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(632) 			inittiles
   jsr inittiles
;### fifteen.atl(633) 			showboard
   jsr showboard
;### fifteen.atl(634) 			changecolor ctmp
   lda currcol
   sta changecolor__col
   jsr changecolor
_lbl128:
;### fifteen.atl(636) 	if CH=62
   lda CH
   cmp #62
   jne _lbl131
;### fifteen.atl(637) 		CH = none
   lda #key__none
   sta CH
;### fifteen.atl(638) 		if play = 1
   lda play
   cmp #1
   jne _lbl130
;### fifteen.atl(639) 			mus_silence
   jsr mus_silence
;### fifteen.atl(640) 			play = 0
   lda #0
   sta play
;### fifteen.atl(641) 		else
   jmp _lbl131
_lbl130:
;### fifteen.atl(642) 			_x= $0
;### fifteen.atl(643) 			_y= $9a
;### fifteen.atl(644) 			mus_init
   jsr mus_init
;### fifteen.atl(645) 			play=1
   lda #1
   sta play
_lbl131:
;### fifteen.atl(647) 	if CONSOL = 3
   lda CONSOL
   cmp #3
   jne _lbl132
;### fifteen.atl(648) 		shuffletiles
   jsr shuffletiles
;### fifteen.atl(649) 		showboard
   jsr showboard
;### fifteen.atl(650) 		goto loop
   jmp loop
_lbl132:
;### fifteen.atl(652) 	if CONSOL = 5
   lda CONSOL
   cmp #5
   jne _lbl133
;### fifteen.atl(653) 		changecolor 0
   lda #0
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(654) 		btmp1 = changepicture
   jsr changepicture
   lda changepicture__c
;### fifteen.atl(655) 		showboard
   jsr showboard
;### fifteen.atl(656) 		changecolor btmp1
   sta changecolor__col
   jsr changecolor
;### fifteen.atl(657) 		goto loop
   jmp loop
_lbl133:
;### fifteen.atl(660) 	if CONSOL <> 6
   lda CONSOL
   cmp #6
   jne loop
_lbl134:
   rts
.endp
game .proc
;### fifteen.atl(666) 	showboard
   jsr showboard
;### fifteen.atl(667) 	play=0
;### fifteen.atl(668) 	mus_silence
   jsr mus_silence
;### fifteen.atl(669) 	eraserect 26 140 14 60
   lda #26
   sta eraserect__xscr
   lda #140
   sta eraserect__yscr
   lda #14
   sta eraserect__width
   lda #60
   sta eraserect__height
   jsr eraserect
;### fifteen.atl(671) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
;### fifteen.atl(672) 	if cnt=16 shuffletiles showboard
   cmp #16
   jne _lbl135
   jsr shuffletiles
   jsr showboard
_lbl135:
;### fifteen.atl(673) 	moves=0
   lda #0
   sta moves
;### fifteen.atl(674) 	start_timer
   jsr start_timer
;### fifteen.atl(676) 	drawbesttime
   jsr drawbesttime
gameloop:
;### fifteen.atl(679) 	CH = none
   lda #key__none
   sta CH
;### fifteen.atl(681) 	cnt=countgoodtiles
   jsr countgoodtiles
   lda countgoodtiles__res
   sta game__cnt
;### fifteen.atl(682) 	textbuf(0,0)="Good tiles:[cnt] "
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
;### fifteen.atl(683) 	text 27 141 13
   lda #27
   sta text__xt
   lda #141
   sta text__yt
   lda #13
   sta text__len
   jsr text
;### fifteen.atl(685) 	update_timer
   jsr update_timer
;### fifteen.atl(686) 	textbuf(0,0)="Time: [tdmin][tmin]:[tdsec][tsec]"
   clc
   jsr _std_print_adr
   dta b(6),c'Time: '
   dta b(129),a(tdmin)
   dta b(129),a(tmin)
   dta b(1),c':'
   dta b(129),a(tdsec)
   dta b(129),a(tsec)
   dta b(0)
;### fifteen.atl(687) 	text 27 152 11
   lda #152
   sta text__yt
   lda #11
   sta text__len
   jsr text
;### fifteen.atl(689) 	if cnt=16
   lda countgoodtiles__res
   cmp #16
   jne _lbl136
;### fifteen.atl(690) 		textbuf(0,0)="            "
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
;### fifteen.atl(691) 		text 7 86 12
   lda #7
   sta text__xt
   lda #86
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(692) 		text 7 102 12
   lda #102
   sta text__yt
   jsr text
;### fifteen.atl(693) 		text 7 114 12
   lda #114
   sta text__yt
   jsr text
;### fifteen.atl(694) 		windowpmg
   jsr windowpmg
;### fifteen.atl(695) 		textbuf(0,0)=" Well done! "
   clc
   jsr _std_print_adr
   dta b(12),c' Well done! '
   dta b(0)
;### fifteen.atl(696) 		text 7 94 12
   lda #94
   sta text__yt
   jsr text
;### fifteen.atl(697) 		textbuf(0,0)=" Time:[tdmin][tmin]:[tdsec][tsec] "
   clc
   jsr _std_print_adr
   dta b(6),c' Time:'
   dta b(129),a(tdmin)
   dta b(129),a(tmin)
   dta b(1),c':'
   dta b(129),a(tdsec)
   dta b(129),a(tsec)
   dta b(1),c' '
   dta b(0)
;### fifteen.atl(698) 		text 7 106 12
   lda #106
   sta text__yt
   jsr text
;### fifteen.atl(699) 		if moves<bmoves
   lda moves
   cmp bmoves
   jcs _lbl137
;### fifteen.atl(700) 			besttime(0,0)="BESTTIME:[tdmin][tmin]:[tdsec][tsec]"
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
;### fifteen.atl(701) 			sleep 100
   lda #100
   sta sleep__time
   jsr sleep
;### fifteen.atl(702) 			textbuf(0,0)=" NEW RECORD!"
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
;### fifteen.atl(703) 			text 7 94 12
   lda #7
   sta text__xt
   lda #94
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(704) 			textbuf(0,0)=" [moves] MOVES!    "
   clc
   jsr _std_print_adr
   dta b(1),c' '
   dta b(130),a(moves)
   dta b(11),c' MOVES!    '
   dta b(0)
;### fifteen.atl(705) 			text 7 106 12
   lda #106
   sta text__yt
   jsr text
;### fifteen.atl(706) 			bmoves=moves
;### fifteen.atl(707) 			dir=0
   lda #0
   sta dir
;### fifteen.atl(708) 			AUDC1=$68
   lda #104
   sta AUDC1
;### fifteen.atl(715) 			AUDC1=0
   jmp _lbl138
_lbl140:
;### fifteen.atl(710) 				AUDF1=dir
   lda dir
   sta AUDF1
;### fifteen.atl(711) 				setboardcolor RANDOM
   lda RANDOM
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(712) 				sleep 1
   lda #1
   sta sleep__time
   jsr sleep
;### fifteen.atl(713) 				inc dir
   inc dir
_lbl138:
;### fifteen.atl(709) 			while dir<200
   lda dir
   cmp #200
   jcc _lbl140
_lbl139:
   lda #0
   sta AUDC1
;### fifteen.atl(716) 			setboardcolor currcol
   lda currcol
   sta setboardcolor__col
   jsr setboardcolor
;### fifteen.atl(717) 			dir=0
;### fifteen.atl(718) 		else
   jmp _lbl141
_lbl137:
;### fifteen.atl(719) 			sleep 200
   lda #200
   sta sleep__time
   jsr sleep
_lbl141:
;### fifteen.atl(721) 		CH=28
;### fifteen.atl(722) 		goto gameend
   jmp _lbl147
_lbl136:
;### fifteen.atl(724) 	textbuf(0,0)="Moves: [moves]     "
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
;### fifteen.atl(725) 	text 27 163 12
   lda #27
   sta text__xt
   lda #163
   sta text__yt
   lda #12
   sta text__len
   jsr text
;### fifteen.atl(727) 	ss:stick'state = STICK(0)
   lda STICK
   sta game__ss
;### fifteen.atl(729) 	dir=0
   lda #0
   sta dir
;### fifteen.atl(730) 	if ss = 14 dir = 1
   lda game__ss
   cmp #14
   jne _lbl142
   lda #1
   sta dir
_lbl142:
;### fifteen.atl(731) 	if ss = 7  dir = 2
   lda game__ss
   cmp #7
   jne _lbl143
   lda #2
   sta dir
_lbl143:
;### fifteen.atl(732) 	if ss = 13 dir = 3
   lda game__ss
   cmp #13
   jne _lbl144
   lda #3
   sta dir
_lbl144:
;### fifteen.atl(733) 	if ss = 11 dir = 4
   lda game__ss
   cmp #11
   jne _lbl145
   lda #4
   sta dir
_lbl145:
;### fifteen.atl(734) 	if dir <> 0 
   lda dir
   cmp #0
   jeq _lbl146
;### fifteen.atl(735) 		x,y=findempty
   jsr findempty
;### fifteen.atl(736) 		dec dir
   dec dir
;### fifteen.atl(737) 		movetile x y dir 1
   lda findempty__x
   sta movetile__xs
   lda findempty__y
   sta movetile__ys
   lda dir
   sta movetile__m
   lda #1
   sta movetile__show
   jsr movetile
;### fifteen.atl(738) 		dir=0
;### fifteen.atl(739) 		sleep 1
   sta sleep__time
   jsr sleep
_lbl146:
;### fifteen.atl(741) 	if CH <> 28 goto gameloop  ; ESC 
   lda CH
   cmp #28
   jne gameloop
_lbl147:
;### fifteen.atl(745) 	CH = none
   rts
.endp
   icl '../../src/platform/atari/atari.asm'
   icl '../../src/processor/m6502/m6502.asm'

