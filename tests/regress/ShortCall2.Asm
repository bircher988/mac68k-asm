; ShortCall2.Asm - second module of the ShortCall test: a short call in a module that
; comes after the code that has grown.
	XDEF	Second
Second	BSR.S	@1			; 6102
	RTS
@1	RTS
	END
