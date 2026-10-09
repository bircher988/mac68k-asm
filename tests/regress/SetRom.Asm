; SetRom.Asm - SET may redefine a symbol; traps carry the ROM that introduced them
	.TRAP	_Plus	$A063	128K
	.TRAP	_Sys41	$A090	256K
Y	SET	1
Y	SET	2
	DC.W	Y
	_Plus
	_Sys41
	END
