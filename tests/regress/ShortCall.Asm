; ShortCall.Asm - regression test for short calls that were widened without need.
; In a pass in which code further up has grown, a branch has moved, while a label further
; down still has its address of the pass before. The distance to such a label has to be
; taken from where the branch stood in that pass. Taken from where it stands now it comes
; out too small by the growth: a BSR.S whose target lies exactly that far ahead seemed to
; call the very next instruction and was widened for good, with a warning; and a branch
; without a size seemed to go backward, was short for one pass and so made the code behind
; it grow again in the next (mac68k-asm up to 1.2.2).
; Every BSR.S here stays short, except the last one. Three warnings: the two BRA.S that
; cannot reach, and the BSR.S to the very next instruction.
	XDEF	Start
Start
; an address that turns out to need a long word: this MOVE grows by two bytes in the second pass
	MOVE.W	Later,D0
	BSR.S	@1			; 6102: two bytes ahead, as much as the code above has grown
	RTS
@1	RTS
; two short branches that cannot reach: four bytes more
	BRA.S	Far			; widened, with a warning
	BRA.S	Far			; widened, with a warning
	DCB.W	80,$4E71
Far	RTS
	BSR.S	@2			; 6106: six bytes ahead, as much as the code above has grown
	RTS
	NOP
	NOP
@2	RTS
; a branch without a size to a label four bytes ahead is long (67000002) in every pass,
; and the call behind it short
	BEQ	@3
@3	BSR.S	@4			; 6102
	RTS
@4	RTS
; a call to the very next instruction cannot be short: widened (4EBA0002), with a warning
	BSR.S	@5
@5	RTS
Later	EQU	$12345
	END
