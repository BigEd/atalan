	org 30000

tv_flag	equ 5C3Ch

start
	; Directs rst 10h output to main screen.
	xor a
	ld (tv_flag),a

	ld hl, print_char2
	call print
	db 5,"Hello",1," ",5,"World",0
	
	ld a, 'Z'
	call print_char
	ret

;Print characters based on format string stored after the call instruction.

;Uses a,d,hl

;Destination address is in BC
print_mem: PROC
	ld hl, store_char
	ENDP

;IN: HL  address of print char procedure 
print:  PROC

	local token,chars,exit

	pop bc
	
token:	
	ld a, (bc)
	inc bc
	cp 0
	jr z,exit
	ld d,a
	;print D characters from BC address
chars:			
	ld a,(bc)		;load command
	inc bc
	call print_char  ;print_char
	dec d
	jr nz,chars
	jr token
	
exit:
	push bc
	ret 
	
print_char:
	jp (hl)
	ENDP

store_char: PROC
	ld (de),a
	inc de
	ENDP
		
;Print one character on screen	
print_char2 PROC
	push hl
	rst 10h
	pop hl
	ret

	ENDP

;hello	db "Hello, world.", 0Dh, 0

	end start
