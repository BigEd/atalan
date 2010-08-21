RANDOM equ 53770
VCOUNT equ 54283
WSYNC equ 54282
button__state__pressed equ 0
STRIG equ 644
DMACTL equ 559
player_size equ 53256
player_x equ 53248
player_gfx equ 53261
player_col2 equ 53266
GRACTL equ 53277
i equ 128
_s0___3 equ 129
_7 equ 130
   org $2e0
   dta a($2000)
   org $2000
;(13) DMACTL = 0

   lda #0
   sta DMACTL
;(14) GRACTL = 0

   sta GRACTL
;(24) 

   sta i
   tax
_lbl2:
;(19) 	hpos(i)  = RANDOM

   lda RANDOM
   sta hpos,x
   and #3
   tay
;(20) 	speed(i) = (RANDOM and 3) + 1

   clc
   adc #1
   sta speed,x
;(21) 	col(i)   = RANDOM or 3			;make the star color light enough 

   lda RANDOM
   ora #3
   sta col,x
   inx
   txa
   cpx #243
   jne _lbl2
   sty _s0___3
   stx i
;(25) player_gfx(0) = 1

   lda #1
   sta player_gfx
;(26) player_size(0) = 0

   lda #0
   sta player_size
   ldy _7
   jmp _lbl3
_lbl10:
   jmp _lbl5
_lbl7:
;(32) 		

   sta WSYNC
_lbl5:
;(30) 	until VCOUNT = 4 

   lda VCOUNT
   cmp #4
   jeq _lbl6
   jmp _lbl7
_lbl6:
   lda #0
   sta i
   tax
_lbl9:
;(34) 		player_col2(0) = col(i)

   lda col,x
   sta player_col2
;(35) 		player_x(0) = hpos(i)

   lda hpos,x
   sta player_x
;(36) 		hpos(i) = hpos(i) + speed(i)

   lda hpos,x
   clc
   adc speed,x
   sta hpos,x
;(40) 

   sta WSYNC
   inx
   txa
   cpx #243
   jne _lbl9
   stx i
_lbl3:
;(28) until STRIG(0) = pressed

   lda STRIG
   tay
   cmp #button__state__pressed
   jeq _lbl4
   jmp _lbl10
_lbl4:
;(41) player_gfx(0) = 0

   lda #0
   sta player_gfx
;(42) player_gfx(1) = 0

   sta player_gfx+1
;(43) GRACTL = 0

   sta GRACTL
;(44) DMACTL = 34

;(45) 
   jmp *
   icl 'atari.asm'
hpos:
   .ds 243
speed:
   .ds 243
col:
   .ds 243
