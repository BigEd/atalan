.data?  ;code_end
   _sys_x  dword ?
   
.code

_sys_print_int PROC
;Print integer in eax.
;Trash: ebp, edx, ebx, ebp, eax

	mov ebp, esp
	sub esp, 16
digit1:
	mov edx, 0		;eax:ebx
	mov ebx, 10
	div ebx 
	add dl, '0' 
	dec ebp
	mov byte ptr[ebp], dl
	cmp eax, 0
	jnz digit1
		
	mov eax, ebp
	mov ebx, esp
	add ebx, 16
	sub ebx, ebp
	call _sys_print_chars
	
	add esp, 16
	ret
_sys_print_int ENDP

_sys_print_chars PROC
;IN: eax  addr
;    ebx  len
;TRASH: ecx, ebx, eax

	push 0
	lea  ecx, _sys_x
	push ecx	
	push ebx
	push eax   
	invoke GetStdHandle, STD_OUTPUT_HANDLE
	push eax   
	call WriteFile
	ret

_sys_print_chars ENDP
	
system__print PROC
	pop ecx
cmd:
	mov ebx, 0
	mov bl, byte ptr [ecx]		;load code (length)
	inc ecx
	cmp bl, 0
	jz  done		
	jns txt
		
	and bl, 127					;size of the number (we only support 4 signed bytes now)
	jz  eol
num:	
	mov eax, dword ptr [ecx]	;in buffer is the address of the variable with
	add ecx, 4
	mov eax, dword ptr [eax]	;now we have the value from variable in eax

	push ecx
	call _sys_print_int
	pop ecx
	jmp cmd
		
eol:
	lea eax, _eol_txt
	mov ebx, 2
	jmp print
_eol_txt:	
	db 10,13 
txt:	
	mov eax, ecx
	add ecx, ebx
	
print:
	push ecx	
	call _sys_print_chars
	pop  ecx
	jmp cmd
	
done:	
	push ecx
	ret

system__print ENDP

