_arr equ 128
_TEMPW1 equ 50
_TEMPW2 equ 52
timer equ 20
RANDOM equ 53770
CHBAS equ 756
LMARGN equ 82
RMARGN equ 83
VCOUNT equ 54283
WSYNC equ 54282
STRIG equ 644
STICK equ 632
DMACTL equ 559
COLOR0 equ 708
COLPF equ 53270
COLBK equ 53274
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
SDLSTL equ 560
NMIEN equ 54286
VDSLST equ 512
VVBLKD equ 548
DOSVEC equ 10
x equ 132
z equ 133
   org $2e0
   dta a($2000)
   org $2000
   jsr _std_print_out
   dta b(12),c'Test started'
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(16),c'Byte arithmetics'
   dta b(128)
   dta b(0)
   lda #3
   sta x
   jsr _std_print_out
   dta b(3),c'Mul'
   dta b(128)
   dta b(0)
   lda x
   ldx #7
   jsr _sys_mul8
   lda _TEMPW2
   sta z
   cmp #21
   jeq _lbl1
   jmp fail
_lbl1:
   jsr _std_print_out
   dta b(10),c'*** OK ***'
   dta b(128)
   dta b(0)
   jmp done
fail:
   jsr _std_print_out
   dta b(16),c'*** Failure! ***'
   dta b(128)
   dta b(0)
   lda #37
   sta COLOR0+4
done:
   jmp *
   icl 'atari.asm'
