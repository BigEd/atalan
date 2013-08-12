.386
.model flat, stdcall
option casemap :none
include C:\masm32\include\windows.inc
include C:\masm32\include\kernel32.inc
include C:\masm32\include\masm32.inc
includelib C:\masm32\lib\kernel32.lib
includelib C:\masm32\lib\masm32.lib

.data?  ;code_end
   _t  dword ?
.data
   
.code
start:

	mov eax, 12345
	mov _t, eax
	
	call _sys_print
	db 128
	db 7,"Value: "
	db 128+4	; value of variable
	dd _t
	db 128,13, "Hello, World!",128
	db 0
	
	invoke ExitProcess, 0


	include <x86.asm>
end start

