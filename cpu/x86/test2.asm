.386
.model flat, stdcall
option casemap :none
include C:\masm32\include\windows.inc
include C:\masm32\include\kernel32.inc
include C:\masm32\include\masm32.inc
includelib C:\masm32\lib\kernel32.lib
includelib C:\masm32\lib\masm32.lib
.data?
	_x dword ?
	_y byte ?
.data
	HelloWorld db "Hello World!", 0

.code
start:
	mov eax, 34
	mov _x, eax
	invoke StdOut, addr HelloWorld
	invoke ExitProcess, 0
end start
