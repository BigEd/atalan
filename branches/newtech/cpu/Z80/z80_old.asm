
system__print_out PROC
	ld hl, print_char
	ENDP

;IN: HL  address of print char procedure 
z80_print:  PROC

	local token,chars,exit,print_char

	pop bc
	
token:	
	ld a, (bc)
	inc bc
	cp 0
	jr z,exit
	cp 128
	jr nz, string
	call emit_eol
	jr token
string:
	ld d,a
	;print D characters from BC address
chars:			
	ld a,(bc)		;load command
	inc bc
	call emit_char  ;print_char
	dec d
	jr nz,chars
	jr token
	
exit:
	push bc
	ret 
emit_eol:
	ld a, 13	
emit_char:
	jp (hl)
	ENDP

store_char: PROC
	ld (de),a
	inc de
	ENDP
		
;Print one character on screen	
print_char PROC
	push hl
	rst 10h
	pop hl
	ret

	ENDP
