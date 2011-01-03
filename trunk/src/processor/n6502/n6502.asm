;********************************************************
;
;  ATALAN 6502 processor definition support routines
;
;  Parts of code have been taken from Effectus library.
;
;********************************************************


;Noname
;651-655         028B-028F
;
;More spare bytes that you shouldn't use because future versions of the OS might use them. 
;Locations 651 and 652 are already used by version "B" as part of the interrupt handler routines.

;OS registers we use as temporary:
;
;BUFRLO,BUFRHI   50,51 $32,$33
;BFENLO,BFENHI   52,53 $34,$35

;ADRESS  100,101   $64,$65  A temporary storage location used by the display handler for so many things that it made my mind spin and I forgot what they were.
;MLTTMP  102,103   $66,$67  More temporary storage, with aliases OPNTMP and TOADR.
;SAVADR  104,105   $68,$69  Also know as FRMADR. Also used for temporary storage. 
;                           Also not significant enough to explain (look at the OS listing if, for some reason, you really care).

;Registers used when drawing line
;ROWAC   112,113   $70,$71
;COLAC   114,115   $72,$73
;ENDPT   116,117   $74,$75
;COUNTR  126,127   $7E,$7F  COUNTR tells how many points have to be plotted before the line is finished.

;
_SYS_PRINT_SIGNED = 1

_putchr_proc_adr = $653  ;+$654 
?argptr =   $32		;$33
?varptr	=	$34
?size	=	$15		;15		//$16 BUFADR
		.IF _SYS_PRINT_SIGNED = 1 
?sign = $16
		.ENDIF
?aux2	=	$38
?aux	=	$39

;Following assigns are defined by mc6502.atl

_TW1 = $32	;$33  mc6502.atl defines these labels as _TEMPW1, _TEMPW2.
_TW2 = $34  ;$35
_TW3 = $15	;$16
_TL1 = $70	;four byte register ($70-$73) defined under name _TEMPL1 in mc6502.atl		

_stdbuf    = $600		;TODO: use LINBUF ($247-$26E)  (100,101 can point to it)

;Argument list:
;	0	end of list
;	1..127  Constant string of specified lenght
; 128     EOL

;   7 6 5 4 3 2 1 0 
;  |0| | | | | | | |		String of length 0..127
;  |1|0|0| | | | | |    Pointer to unsigned integer of length 1..31 bytes
;  |1|0|1| | | | | |    Pointer to signed integer of length 1..31 bytes

;         ---- ptr to unsigned integer ---- (7-bit set)
; 129     one byte integer reference
;	130     two byte integer reference
;	131     three byte integer eference
;	132     four byte integer reference
;         ---- ptr to signed -----  (5-bit set)
; 161     one byte
; 162     two byte
; 163     three byte
; 164     four byte



 
/*
  Mul8 - 8-bit multiplication routine
  
  Original source:
  Book Atari Roots (Chapter Ten - Assembly Language Math)
  Hyperlink: http://www.atariarchives.org/roots/chapter_10.php
  
  Parameters:
	  a First multiplicant
	  x Second multiplicant
  Result: 
		TEMPW2,TEMPW2+1	high byte of the result
*/

_sys_mul8  .proc

MUL1 = _TW1
MUL2 = _TW1+1
RES  = _TW2		;_TW2+1

		sta MUL1			;input comes in A and X
		stx MUL2
		
		lda #0			;RES = 0
		sta RES
		
		ldx #8  
loop
		lsr MUL1		;MUL1 = MUL1 / 2
		bcc noadd
		
		clc					;RES = RES + (MUL2 * $ff)
		adc MUL2	  
noadd 
		ror @				;RES = RES / 2		(Carry is 0, when there was no add)
		ror RES

		dex
		bne loop
		
		sta RES+1
		rts

		.endp

/*
  Mul16 - 16-bit multiplication routine
    
  Parameters:
	  _TW1 First multiplicant (we use only two bytes now)
	  _TW2 Second multiplicant
  Result: 
		_TL1 Result
*/

_sys_mul16  .proc

MUL1 = _TW1
MUL2 = _TW2
RES  = _TL1
		
		lda #0      ;RES = 0;
		sta RES
		sta RES+1
		sta RES+2
		sta RES+3
		
		ldx #16  
loop
		lsr MUL1+1		;MUL1 = MUL1 / 2
		ror MUL1
		bcc noadd
		
		lda RES+2			;RES = RES + (MUL2 * $ffff)  (only upper half) 
		clc		
		adc MUL2
		sta RES+2
		lda RES+3
		adc MUL2+1
		sta RES+3
noadd
		ror RES+3				;RES = RES / 2		(Carry is 0, when there was no add)
		ror RES+2
		ror RES+1
		ror RES+0
		 
		dex
		bne loop
		
		rts

		.endp
 
/*
  Div8 - 8-bit division routine
 
  Original source:
  Book Atari Roots (Chapter Ten - Assembly Language Math)
  Hyperlink: http://www.atariarchives.org/roots/chapter_10.php
 
  Parameters:
  _TEMPW1		16-bit dividend
  a					 p1_math:  8-bit divisor
  x          number of bits of an divisir (8 max)
 
  Result:
  x STORE1: 8-bit quotient
  a STORE2: 8-bit remainder
*/

_sys_div8  .proc
  ldx #08  ; For an 8-bit divisor
	
_sys_div

divisor  = _TW2
quotient = _TW2+1

  sta divisor
	lda #0     				; for non-8 divis
	sta quotient
  lda _TW1+1
  sec 
  sbc divisor
dloop 
  php  					; The loop that divides 
  rol quotient
  asl _TW1
  rol @
  plp
  bcc addit
  sbc divisor
  jmp next
addit 
  adc divisor
next  
  dex
  bne dloop
  bcs fini
  adc divisor
  clc
fini
	tax            ; x = remainder
  lda quotient   ; a = quotient
  rol
  rts
 
  .endp

/*

  Square Root

  Calculates the 8 bit root and 9 bit remainder of a 16 bit unsigned integer in
  Numberl/Numberh. The result is always in the range 0 to 255 and is held in
  Root, the remainder is in the range 0 to 511 and is held in Reml/Remh

  partial results are held in templ/temph

  Destroys A, X registers.

	Arguments:
	
	_TEMPW1  16-bit number to compute root of
	
	<a        Square root
	<_TEMPW2  Remainder of square
	
*/

_sys_sqrt16 .proc

Numberl		= _TW1		; number to find square root of low byte
Numberh		= _TW1+1	; number to find square root of high byte
Reml		  = _TW2		; remainder low byte
Remh		  = _TW2+1	; remainder high byte
templ		  = _TL1		; temp partial low byte
temph		  = _TL1+1	; temp partial high byte
Root		  = _TL1+2	; square root

	LDA	#$00		; clear A
	STA	Reml		; clear remainder low byte
	STA	Remh		; clear remainder high byte
	STA	Root		; clear Root
	LDX	#$08		; 8 pairs of bits to do
Loop
	ASL	Root		; Root = Root * 2

	ASL	Numberl		; shift highest bit of number ..
	ROL	Numberh		;
	ROL	Reml		; .. into remainder
	ROL	Remh		;

	ASL	Numberl		; shift highest bit of number ..
	ROL	Numberh		;
	ROL	Reml		; .. into remainder
	ROL	Remh		;

	LDA	Root		; copy Root ..
	STA	templ		; .. to templ
	LDA	#$00		; clear byte
	STA	temph		; clear temp high byte

	SEC			; +1
	ROL	templ		; temp = temp * 2 + 1
	ROL	temph		;

	LDA	Remh		; get remainder high byte
	CMP	temph		; comapre with partial high byte
	BCC	Next		; skip sub if remainder high byte smaller

	BNE	Subtr		; do sub if <> (must be remainder>partial !)

	LDA	Reml		; get remainder low byte
	CMP	templ		; comapre with partial low byte
	BCC	Next		; skip sub if remainder low byte smaller

				; else remainder>=partial so subtract then
				; and add 1 to root. carry is always set here
Subtr
	LDA	Reml		; get remainder low byte
	SBC	templ		; subtract partial low byte
	STA	Reml		; save remainder low byte
	LDA	Remh		; get remainder high byte
	SBC	temph		; subtract partial high byte
	STA	Remh		; save remainder high byte

	INC	Root		; increment Root
Next
	DEX			; decrement bit pair count
	BNE	Loop		; loop if not all done

	lda Root
	
	RTS
.endp