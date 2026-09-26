; BareVar.Asm - regression test: a DS variable named on its own may be read,
; but where the operand must be alterable the base register is written out.
	XDEF	Start
Start
	MOVE.W	Var,D1			; read: fine
	CMP.W	Var,D1			; read: fine
	BTST	#1,Var			; bit test: fine
	PEA	Var			; address: fine
	MOVE.W	D0,Var			; error: destination
	CLR.W	Var			; error
	TST.W	Var			; error
	ADDQ.W	#1,Var			; error
	BSET	#1,Var			; error
	RTS
Var	DS.W	1
	END
