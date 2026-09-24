; Hello.Asm - minimal mac68k-asm example
;
; Opens a window from a WIND resource, draws a line of text and quits on
; the next mouse click. Shows the three things you need for any program:
; toolbox traps from Traps.D, equates from ToolEqu.D and DS variables in
; the A5 globals area.

	Include	Traps.D		; toolbox and OS traps (_InitGraf, _DrawString, ...)
	Include	ToolEqu.D	; toolbox equates (windowSize, ...)

WindID	EQU	128		; WIND resource in Hello.R

	XDEF	Start

;---------------------------------------------------------------- main
Start
	PEA	-4(A5)		; QuickDraw globals live just below A5
	_InitGraf
	_InitFonts
	MOVE.L	#$0000FFFF,D0	; flush all pending events
	_FlushEvents
	_InitWindows
	_InitCursor

	; FUNCTION GetNewWindow(windowID: INTEGER; wStorage: Ptr; behind: WindowPtr): WindowPtr
	CLR.L	-(SP)		; room for the result
	MOVE.W	#WindID,-(SP)
	PEA	WindStorage	; DS variable -> assembled as WindStorage(A5)
	MOVE.L	#-1,-(SP)	; in front of all other windows
	_GetNewWindow
	MOVE.L	(SP)+,WindPtr

	MOVE.L	WindPtr,-(SP)
	_SetPort		; draw into our window

	MOVE.W	#20,-(SP)	; MoveTo(h, v)
	MOVE.W	#40,-(SP)
	_MoveTo
	PEA	Message		; code label -> assembled PC-relative
	_DrawString

WaitClick
	_SystemTask		; let desk accessories breathe
	CLR.W	-(SP)		; FUNCTION Button: BOOLEAN
	_Button
	TST.B	(SP)+		; Pascal BOOLEAN sits in the high byte
	BEQ	WaitClick
	_ExitToShell		; back to the Finder

;---------------------------------------------------------------- data
Message	DC.B	MsgEnd-Message-1
	DC.B	'Hello from mac68k-asm. Click to quit.'
MsgEnd

; DS does not reserve space here but in the A5 globals area
WindPtr	DS.L	1
WindStorage	DS.B	windowSize

	END
