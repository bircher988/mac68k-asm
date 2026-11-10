; IncEquB.Asm - takes theTicks from the include file, too (the file is read only once)
	Include	IncEqu.Inc
	XDEF	Bee
Bee	MOVE.L	theTicks,D1
	RTS
	END
