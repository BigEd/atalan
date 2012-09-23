print  PROC

	local token,chars,exit

	pop hl
	
token	
	ld a, (hl)
	inc hl
	cp 0
	jr z,exit
	ld d,a
	;print D characters from HL address
chars			
	ld a,(hl)		;load command
	inc hl
	call print_char
	dec d
	jr nz,chars
	jr token
	
exit
	push hl
	ret 
	
	ENDP
