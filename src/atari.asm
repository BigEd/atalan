;********************************************************
;
;  ATALAN Atari standard library
;
;  Parts of code have been taken from Effectus library.
;
;********************************************************


;Noname
;651-655         028B-028F
;
;More spare bytes that you shouldn't use because future versions of the OS might use them. 
;Locations 651 and 652 are already used by version "B" as part of the interrupt handler routines.

_putchr_proc_adr = $653  ;+$654 
?argptr =   $32		;$33
?varptr	=	$34
?size	=	$15		;15		//$16 BUFADR
?aux2	=	$38
?aux	=	$39

;Following assigns are defined by mc6502.atl

_TEMPL1 = $32
;_TEMPW1 = _TEMPL1
;_TEMPW2 = _TEMPL1+2
_TEMPW3 = $15

_TW1 = $32			;mc6502 defines these labels as _TEMPW1, _TEMPW2.
_TW2 = $34

_stdbuf    = $600		;TODO: use LINBUF ($247-$26E)  (100,101 can point to it)

;Argument list:
;	0	end of list
;	1..127  Constant string
;   128     EOL
;   129     one byte integer reference
;	130     two byte integer reference
;	131     three byte integer eference
;	132     four byte integer reference

_std_print_adr .proc

		lda #<_adr_putchr
		sta _putchr_proc_adr
		lda #>_adr_putchr
		sta _putchr_proc_adr+1
		clc
		bcc _std_print
		.endp
		
_std_print_out .proc
		lda #<_out_putchr
		sta _putchr_proc_adr
		lda #>_out_putchr
		sta _putchr_proc_adr+1
		clc											;TODO: No Jump, if we are directly before the _std_print
		bcc _std_print
		.endp
		
_std_print	.proc

		;Get address of argument from stack
		pla
		sta ?argptr
		pla
		sta ?argptr+1
		jsr _read_byte	;just skip next byte (that is part of return address)

		;Read command from input				
command
		jsr _read_byte
		sta ?size
		tay				; to set the flags
		beq done		;command 0 means end of sequence
		bpl str			;1..127 constant string

		;Write n-byte integer variable

		and #$7f
;		sec
;		sbc #128
		sta ?size
		beq eol

		jsr _read_byte
		sta ?varptr		
		jsr _read_byte
		sta ?varptr+1

		jsr _std_bin_to_bcd
		jsr _std_print_hex
		clc
		bcc command		;jmp command
eol
		lda #155	
		jsr _std_putchr
		clc
		bcc command		;jmp command
		;Write constant string (size is 1..127, already stored)
					
str
		jsr _read_byte
		jsr _std_putchr
		dec ?size
		bne str
		beq	command
						
done		
		jmp (?argptr)	
		rts
		
_read_byte
		ldy #0
		lda (?argptr),y
		inc ?argptr
		bne skip
		inc ?argptr+1
skip
		rts
.endp

;_std_eol .proc
;	lda #155	
;.endp

;BEWARE!!! _std_eol continues directly to _std_putchr

_std_putchr .proc
		jmp (_putchr_proc_adr)
		rts
.endp

_adr_putchr .proc
;Write byte to address specified by _arr and increment the address by 1.
	
	;Convert ASCII to ATARI character screen code
	;
	;The rules are following:
	;0..31   +64
	;32..95  -32
	;96..127 0  (codes are same)
	 	
	tax
	rol 
	rol 
	rol 
	rol
	and #3
	tay
	txa
	eor tbl,y
	
	;write the char to specified address	
	ldy #0
	sta (_arr),y

	inc _arr
	bne skip1
	inc _arr+1
skip1
	rts

tbl	dta b(%01000000)		;%0 00 00000		00..31	+64
		dta b(%00100000)		;%0 01 00000		32..63	-32
		dta b(%01100000)		;%0 10 00000    64..95  -32
		dta b(%00000000)		;%0 11 00000    96..127 0
	
.endp

_out_putchr .proc
;Write character from A to output.
	
		tax
		lda $347	;ICPUTB+1
		pha
		lda $346	;ICPUTB
		pha
		txa
		rts

.endp

.proc _std_print_hex
;Print hexadecimal number of arbitrary length (?size). 
;Leading zeroes are not printed.
;In:
;	?varptr		Pointer to memory containing the number
;	?size		  Number of bytes to print
;Uses:
;	?aux
;	?aux2
	
		lda ?size
		sta ?aux
		lda #0
		sta ?aux2		; number of non-zero digits on output
		beq _loop
_outbyte
		ldy ?aux		
		lda (?varptr),y
		pha
		lsr
		lsr
		lsr
		lsr
		jsr _write_digit
	
		pla
		and #$0f
		jsr _write_digit
_loop	
		dec ?aux
		bpl _outbyte
		
		;If no character has been written, write at least one 0		
		lda ?aux2
		bne _no_empty
		lda #48
		jsr _std_putchr
_no_empty
 
		rts
	
_write_digit
;In: a 4 bit digit

		tax
		bne _non_zero
		lda ?aux2
		beq _done		;this is zero and there has been no char before - no output
_non_zero
		lda hex,x
		jsr _std_putchr
		inc ?aux2
_done
		rts

hex     dta c"0123456789ABCDEF"

.endp

.proc _std_bin_to_bcd
;Convert binary number to BCD. 
;Arbitrary size (up to 127 bytes) are supported.
;In:
;	?varptr	pointer to binary number
;	?size	number of bytes
;Out:
;	?size	on output, returns size of resulting bcd number
;   ?varptr	on output, containg pointer to converted BCD
;Uses:
;	_stdbuf
;	?aux
;	?aux2

		;Compute size of resulting number 
		ldy ?size
		sty ?aux		; used to count later
		iny				;add space to result
		sty ?size
		
		;Zero the destination buffer
		lda #0
zero	sta _stdbuf-1,y
		dey
		bne zero
		
		;**** We convert the number byte a time
		sed
		
		;?aux = varptr(?aux)
bytes
		dec ?aux
		ldy ?aux
		lda (?varptr),y		
		sta ?aux2
		sec				;set top bit to 1
		bcs loop		

shift_byte			
		ldx #0
		ldy ?size
bcd_mul2
		lda _stdbuf,x
		adc	_stdbuf,x			;buf2(x) = buf2(x) * 2 + carry
		sta _stdbuf,x
		inx
		dey
		bne bcd_mul2
			
		clc
loop	rol ?aux2		;divide by two, if result is 0, end
		bne shift_byte		
		
		lda ?aux
		bne bytes
		cld		
		
		lda #<_stdbuf
		sta ?varptr	
		lda #>_stdbuf
		sta ?varptr+1	
		rts

.endp
 
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

	sta _TW1			;input comes in A and X
	stx _TW1+1

  lda #0
  sta _TW2
  ldx #8
loop
  lsr _TW1		;p1_math
  bcc noadd
  clc
  adc _TW1+1	;p2_math
noadd ror @
  ror _TW2
  dex
  bne loop
  sta _TW2+1
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
