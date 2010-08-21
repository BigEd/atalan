_arr equ 128
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
   org $2e0
   dta a($2000)
   org $2000
   lda #>baloon
   sta CHBAS
   jsr _std_print_out
   dta b(13),c'Hello, World!'
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(128)
   dta b(0)
   jsr _std_print_out
   dta b(36),c'I am ATALAN, written in BALOON font!'
   dta b(128)
   dta b(0)
   jmp *
   icl 'atari.asm'
   .align 1024
baloon:
   ins 'baloon.fnt'
