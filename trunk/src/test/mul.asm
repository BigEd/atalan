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
x equ 131
y equ 132
z equ 133
   org $2e0
   dta a($2000)
   org $2000
   lda #5
   sta x
   lda #12
   sta y
   lda #5
   ldx #12
   jsr _sys_mul8
   lda _TEMPW2
   sta z
   lda _TEMPW2+1
   sta z+1
   jsr _std_print_out
   dta b(129),a(x)
   dta b(3),c' * '
   dta b(129),a(y)
   dta b(3),c' = '
   dta b(130),a(z)
   dta b(128)
   dta b(0)
   jmp *
   icl 'atari.asm'
