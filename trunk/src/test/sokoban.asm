_arr equ 128
_TEMPW1 equ 50
_TEMPW2 equ 52
RANDOM equ 53770
CHBAS equ 756
stick__state__left equ 11
stick__state__right equ 7
stick__state__up equ 14
stick__state__down equ 13
STICK equ 632
COLOR0 equ 708
SDLSTL equ 560
VVBLKD equ 548
_scr equ 32768
licznik_int equ 130
testowo equ 131
testowo2 equ 132
poz_player_x equ 133
poz_player_y equ 134
rys_spychacza equ 135
root__GWIAZDKI1 equ 74
root__GWIAZDKI2 equ 75
root__GWIAZDKI3 equ 76
root__SPYCHACZ_G equ 77
root__SPYCHACZ_P equ 78
root__SPYCHACZ_D equ 79
root__SPYCHACZ_L equ 80
root__PODLOGA equ 0
losuj_gwiazde__gwiazda equ 136
losuj_gwiazde__gw equ 137
rysuj_level__lev equ 138
rysuj_level__pocz equ 139
rysuj_level__aa equ 140
_s0__roz_x equ 141
_s0__roz_y equ 142
_s0___4 equ 143
rysuj_level__lev2 equ 145
rysuj_level__roz_x equ 146
rysuj_level__roz_y equ 147
rysuj_level__yy equ 148
_s1__xx equ 149
_s2__poz_z equ 150
_s2___6 equ 151
_s2___7 equ 152
_s2__scr_xx equ 153
_s2__scr_yy equ 154
_s2__znak equ 155
_s3__x equ 156
_s4__y equ 157
licznik_joya equ 158
_13 equ 159
_17 equ 161
_18 equ 162
_19 equ 163
_20 equ 164
   org $2e0
   dta a($2000)
   org $2000
;(147) 

   lda #>fontFN1
   sta CHBAS
;(148) licznik_joya = 1

   lda #1
   sta licznik_joya
;(150) sdlstl = dl

   lda #<dl
   sta SDLSTL
   lda #>dl
   sta SDLSTL+1
;(151) COLOR0(2) = 0

   lda #0
   sta COLOR0+2
;(152) COLOR0(5) = 0

   sta COLOR0+5
;(155) on__vbi cycle

   lda #<cycle
   sta _13
   lda #>cycle
   sta _13+1
;(156) 

   lda _13
   sta VVBLKD
   lda _13+1
   sta VVBLKD+1
;(157) rys_spychacza = SPYCHACZ_G

   lda #root__SPYCHACZ_G
   sta rys_spychacza
;(159) rysuj_tlo

   jsr rysuj_tlo
;(161) rysuj_level 1

   lda #1
   sta rysuj_level__lev
;(162) 

   jsr rysuj_level
mail_loop:
;(165) if STICK(0) = right

   lda STICK
   sta _17
   cmp #stick__state__right
   jne _lbl26
;(166)         rys_spychacza = SPYCHACZ_P

   lda #root__SPYCHACZ_P
   sta rys_spychacza
;(167)         _scr(poz_player_x,poz_player_y) = rys_spychacza

   ldy poz_player_y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy poz_player_x
   lda #root__SPYCHACZ_P
   sta (_arr),y
;(168)         testowo = licznik_int

   lda licznik_int
;(169)         testowo = testowo + 25

   clc
   adc #25
   sta testowo
;(170)         testowo = testowo mod 50

   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #50
   jsr _sys_div8
   stx testowo
;(171)         testowo2 = licznik_int

   lda licznik_int
   sta testowo2
;(175)         )

   jmp _lbl27
_lbl29:
;(173)              testowo2 = licznik_int

   lda licznik_int
   sta testowo2
   ldy #5
   lda _scr_lo,y
   clc
   adc #5
   sta _arr
   lda _scr_hi,y
   adc #0
   sta _arr+1
   jsr _std_print_adr
   dta b(1),c' '
;(174)              _scr(5,5) = " "

   dta b(0)
_lbl27:
   lda testowo2
   cmp testowo
   jeq _lbl28
   jmp _lbl29
_lbl28:
;(176)         _scr(poz_player_x,poz_player_y) = PODLOGA

   ldy poz_player_y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy poz_player_x
   lda #root__PODLOGA
   sta (_arr),y
;(177)         poz_player_x = poz_player_x + 1

   inc poz_player_x
;(178)         _scr(poz_player_x,poz_player_y) = rys_spychacza

   ldy poz_player_y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy poz_player_x
   lda rys_spychacza
   sta (_arr),y
   jmp _lbl30
_lbl26:
;(179) else if STICK(0) = left

   lda STICK
   sta _18
   cmp #stick__state__left
   jne _lbl31
;(180)         rys_spychacza = SPYCHACZ_L

   lda #root__SPYCHACZ_L
   sta rys_spychacza
;(181)         _scr(poz_player_x,poz_player_y) = rys_spychacza

   ldy poz_player_y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy poz_player_x
   lda #root__SPYCHACZ_L
   sta (_arr),y
   jmp _lbl30
_lbl31:
;(182) else if STICK(0) = up

   lda STICK
   sta _19
   cmp #stick__state__up
   jne _lbl32
;(183)         rys_spychacza = SPYCHACZ_G

   lda #root__SPYCHACZ_G
   sta rys_spychacza
;(184)         _scr(poz_player_x,poz_player_y) = rys_spychacza

   ldy poz_player_y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy poz_player_x
   lda #root__SPYCHACZ_G
   sta (_arr),y
   jmp _lbl30
_lbl32:
;(185) else if STICK(0) = down

   lda STICK
   sta _20
   cmp #stick__state__down
   jne _lbl33
;(186)         rys_spychacza = SPYCHACZ_D

   lda #root__SPYCHACZ_D
   sta rys_spychacza
;(187)         _scr(poz_player_x,poz_player_y) = rys_spychacza

   ldy poz_player_y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy poz_player_x
   lda #root__SPYCHACZ_D
   sta (_arr),y
;(189) 

_lbl33:
_lbl30:
;(191) 
   jmp mail_loop
   jmp *
   icl 'atari.asm'
dl:
;(18) ;const dl:array of dl_command = 3 times $70, $42, _scr, 2 times $02, $02+$80, 20 times $02, $41, dl

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
;(19) const dl:array = 2 times $70, $42, _scr, $10, 23 times $02, $10, $02, $41, dl

   dta a(dl)
   .align 1024
fontFN1:
;(23) ;Font file

   ins 'sokoban1.fnt'
   .align 1024
fontFN2:
;(25) 

   ins 'sokoban2.fnt'
lev__mapa:
;(56) ;--Level 0

   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
;(57)      4,4,4,4,4,4,6

   dta b(6)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(5)
   dta b(0)
   dta b(1)
;(58)      4,0,0,5,0,1,6

   dta b(6)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(3)
   dta b(0)
   dta b(0)
;(59)      4,0,0,3,0,0,6

   dta b(6)
   dta b(4)
   dta b(8)
   dta b(0)
   dta b(3)
   dta b(0)
   dta b(0)
;(60)      4,8,0,3,0,0,6

   dta b(6)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(5)
   dta b(0)
   dta b(1)
;(61)      4,0,0,5,0,1,6

   dta b(6)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
;(62)      4,4,4,4,4,4,6

   dta b(6)
;(63) ;--1

   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(6)
;(64)      4,4,4,4,4,4,6,9

   dta b(9)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(5)
;(65)      4,0,0,0,0,0,5,6

   dta b(6)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(3)
   dta b(0)
   dta b(0)
   dta b(1)
;(66)      4,0,0,3,0,0,1,6

   dta b(6)
   dta b(4)
   dta b(8)
   dta b(0)
   dta b(3)
   dta b(0)
   dta b(0)
   dta b(1)
;(67)      4,8,0,3,0,0,1,6

   dta b(6)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(3)
   dta b(0)
   dta b(0)
   dta b(1)
;(68)      4,0,0,3,0,0,1,6

   dta b(6)
   dta b(4)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(0)
   dta b(5)
;(69)      4,0,0,0,0,0,5,6

   dta b(6)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(4)
   dta b(6)
;(70)      4,4,4,4,4,4,6,9

   dta b(9)
   dta b(9)
lev__rozmiar:
;(73) const lev__rozmiar:array(lev__max, 0..1 ) = ( 

   dta b(7)
;(74)    7, 8

   dta b(8)
   dta b(6)
;(75)    6, 7

   dta b(7)
   dta b(7)
_scr_lo:  :25 dta l(_scr + #*40)
_scr_hi:  :25 dta h(_scr + #*40)
lev__rozmiar_lo:  :2 dta l(lev__rozmiar + #*2)
lev__rozmiar_hi:  :2 dta h(lev__rozmiar + #*2)
losuj_gwiazde .proc
;(79)           gw = RANDOM mod 3

   lda RANDOM
   sta _TEMPW1
   lda #0
   sta _TEMPW1+1
   lda #3
   jsr _sys_div8
   stx losuj_gwiazde__gw
   lda losuj_gwiazde__gw
   cmp #0
   jne _lbl1
;(80)           if gw = 0 then gwiazda = GWIAZDKI1

   lda #root__GWIAZDKI1
   sta losuj_gwiazde__gwiazda
   jmp _lbl2
_lbl1:
   lda losuj_gwiazde__gw
   cmp #1
   jne _lbl3
;(81)           else if gw = 1 then gwiazda = GWIAZDKI2

   lda #root__GWIAZDKI2
   sta losuj_gwiazde__gwiazda
   jmp _lbl2
_lbl3:
   lda losuj_gwiazde__gw
   cmp #2
   jne _lbl4
;(82)           else if gw = 2 then gwiazda = GWIAZDKI3

   lda #root__GWIAZDKI3
   sta losuj_gwiazde__gwiazda
;(83) 

_lbl4:
_lbl2:
   rts
.endp
rysuj_level .proc
;(87)    pocz = 0

   lda #0
   sta rysuj_level__pocz
;(88)    aa = 0

   lda #0
   sta rysuj_level__aa
;(94)    ;rozmiary levelu do narysowania

   jmp _lbl5
_lbl7:
   ldy #0
   lda lev__rozmiar_lo,y
   sta _arr
   lda lev__rozmiar_hi,y
   sta _arr+1
   ldy rysuj_level__aa
   lda (_arr),y
   sta _s0__roz_x
;(90)       roz_x, roz_y = lev__rozmiar(aa,0), lev__rozmiar(aa,1)

   ldy #1
   lda lev__rozmiar_lo,y
   sta _arr
   lda lev__rozmiar_hi,y
   sta _arr+1
   ldy rysuj_level__aa
   lda (_arr),y
   sta _s0__roz_y
;(91)       pocz = pocz +  roz_x * roz_y

   lda _s0__roz_x
   ldx _s0__roz_y
   jsr _sys_mul8
   lda _TEMPW2
   sta _s0___4
   lda _TEMPW2+1
   sta _s0___4+1
   lda rysuj_level__pocz
   clc
   adc _s0___4+0
   sta rysuj_level__pocz
   lda rysuj_level__pocz+1
   adc _s0___4+1
   sta rysuj_level__pocz+1
;(93) 

   inc rysuj_level__aa
_lbl5:
;(89)    while aa < lev

   lda rysuj_level__aa
   cmp rysuj_level__lev
   jcs _lbl6
   jmp _lbl7
_lbl6:
;(95)    lev2 = lev

   lda rysuj_level__lev
   sta rysuj_level__lev2
;(96)    roz_x = lev__rozmiar(lev2, 0)

   ldy #0
   lda lev__rozmiar_lo,y
   sta _arr
   lda lev__rozmiar_hi,y
   sta _arr+1
   ldy rysuj_level__lev2
   lda (_arr),y
   sta rysuj_level__roz_x
;(97)    roz_y = lev__rozmiar(lev2, 1)

   ldy #1
   lda lev__rozmiar_lo,y
   sta _arr
   lda lev__rozmiar_hi,y
   sta _arr+1
   ldy rysuj_level__lev2
   lda (_arr),y
   sta rysuj_level__roz_y
;(99)    yy = 0

   lda #0
   sta rysuj_level__yy
;(123) 

   jmp _lbl8
_lbl17:
;(102)          xx = 0

   lda #0
   sta _s1__xx
   jmp _lbl10
_lbl16:
;(104)                poz_z = pocz

   lda rysuj_level__pocz
   sta _s2__poz_z
   lda rysuj_level__yy
   ldx rysuj_level__roz_x
   jsr _sys_mul8
   lda _TEMPW2
   sta _s2___6
   lda _s2__poz_z
   clc
   adc _s2___6
   sta _s2___7
;(105)                poz_z = poz_z + yy * roz_x + xx

   lda _s2___7
   clc
   adc _s1__xx
   sta _s2__poz_z
;(106)                scr_xx = 10 + xx

   lda _s1__xx
   clc
   adc _s1__xx
   sta _s2__scr_xx
;(107)                scr_yy = 10 + yy

   lda rysuj_level__yy
   clc
   adc rysuj_level__yy
   sta _s2__scr_yy
;(108)                znak = lev__mapa(poz_z)

   ldx _s2__poz_z
   lda lev__mapa,x
   sta _s2__znak
   lda _s2__znak
   cmp #9
   jne _lbl12
;(109)                if znak = 9 then znak = losuj_gwiazde

   jsr losuj_gwiazde
   lda losuj_gwiazde__gwiazda
   sta _s2__znak
   jmp _lbl13
_lbl12:
   lda _s2__znak
   cmp #8
   jne _lbl14
;(111)                     znak = rys_spychacza

   lda rys_spychacza
   sta _s2__znak
;(112)                     poz_player_x = scr_xx

   lda _s2__scr_xx
   sta poz_player_x
;(113)                     poz_player_y = scr_yy

   lda _s2__scr_yy
   sta poz_player_y
   jmp _lbl13
_lbl14:
   lda _s2__znak
   cmp #0
   jeq _lbl15
;(114)                else if znak <> 0 then znak = znak + 64

   lda _s2__znak
   clc
   adc #64
   sta _s2__znak
;(115) 

_lbl15:
_lbl13:
;(116)                _scr(scr_xx, scr_yy) = znak

   ldy _s2__scr_yy
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy _s2__scr_xx
   lda _s2__znak
   sta (_arr),y
;(118) 

   inc _s1__xx
_lbl10:
;(103)              while xx < roz_x

   lda _s1__xx
   cmp rysuj_level__roz_x
   jcs _lbl11
   jmp _lbl16
_lbl11:
;(120) 

   inc rysuj_level__yy
_lbl8:
;(101)    while yy < roz_y

   lda rysuj_level__yy
   cmp rysuj_level__roz_y
   jcs _lbl9
   jmp _lbl17
_lbl9:
   rts
.endp
rysuj_tlo .proc
   lda #0
   sta _s3__x
_lbl21:
;(128) 

   lda #0
   sta _s4__y
_lbl20:
;(127)            _scr(x,y) = losuj_gwiazde

   jsr losuj_gwiazde
   ldy _s4__y
   lda _scr_lo,y
   sta _arr
   lda _scr_hi,y
   sta _arr+1
   ldy _s3__x
   lda losuj_gwiazde__gwiazda
   sta (_arr),y
   inc _s4__y
   lda _s4__y
   cmp #25
   jne _lbl20
   inc _s3__x
   lda _s3__x
   cmp #40
   jne _lbl21
   rts
.endp
cycle .proc
;(130)         inc licznik_int

   inc licznik_int
   lda licznik_int
   cmp #50
   jne _lbl22
;(131)         if licznik_int = 50 then licznik_int = 0

   lda #0
   sta licznik_int
;(132) 

_lbl22:
   lda licznik_int
   cmp #0
   jne _lbl23
;(133)         if licznik_int = 0 then set__font fontFN2

   lda #>fontFN2
   sta CHBAS
   jmp _lbl24
_lbl23:
   lda licznik_int
   cmp #25
   jne _lbl25
;(135) 

   lda #>fontFN1
   sta CHBAS
;(136) ;        inc licznik_joya

_lbl25:
_lbl24:
   jmp $e462
.endp
