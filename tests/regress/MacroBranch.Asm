; MacroBranch.Asm - regression test for short branches inside a macro.
; A macro call is one source line, but its expansion holds several branches.
; The unsized forward BMI becomes .W; that must not widen the BLE.S behind it,
; which reaches its target (6F02). mac68k-asm 1.2 kept "this branch is long"
; per source line, so every BLE.S here was widened with a warning.
	XDEF	Start
	.MACRO	Abs2
	TST.W	D0
	BMI	@neg
	BLE.S	@done
	NEG.W	D0
@done	NOP
@neg	NOP
	.ENDM
Start
	Abs2
Second	Abs2
Third	Abs2
	RTS
	END
