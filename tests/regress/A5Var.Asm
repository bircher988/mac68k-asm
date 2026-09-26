; A5Var.Asm - regression test: the same writes with the base register assemble
; to d16(A5) - 3B40 FEFE, 426D FEFE, 4A6D FEFE, 526D FEFE, 08ED 0001 FEFE.
	XDEF	Start
Start
	MOVE.W	Var,D1
	MOVE.W	D0,Var(A5)
	CLR.W	Var(A5)
	TST.W	Var(A5)
	ADDQ.W	#1,Var(A5)
	BSET	#1,Var(A5)
	RTS
Var	DS.W	1
	END
