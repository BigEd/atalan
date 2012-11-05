platform__new_line_char  EQU 13

;Print one character on screen	
platform__print_char PROC
	push hl
	rst 10h
	pop hl
	ret

	ENDP
