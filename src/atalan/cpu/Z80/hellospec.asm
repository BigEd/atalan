
	org 30000

tv_flag	equ 5C3Ch

start

	; Directs rst 10h output to main screen.
	xor a
	ld (tv_flag),a

	call system__print_out
	db 7,"Number:"
	db 128+1
	dw num
	db 2, " x"
	db 128
	db 3, "EOL"
	db 0
	
;	ld a, 'Z'
;	call print_char
	ret

num	db 123

	include "z80.asm"

;hello	db "Hello, world.", 0Dh, 0

	end start
