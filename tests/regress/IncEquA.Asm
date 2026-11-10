; IncEquA.Asm - takes theTicks from the include file
	Include	IncEqu.Inc
	XDEF	Start
Start	MOVE.L	theTicks,D0
	RTS
	END
