; QuickEqu.D - QuickDraw equates (grafports, fonts, pictures, regions)
; Part of mac68k-asm (https://github.com/bircher988/mac68k-asm). Names and values as
; documented in Inside Macintosh; generated file, do not edit by hand.

; The block after 'IF	 wholeQuick' is only assembled when the program defines that symbol,
; as in the original files (it keeps the symbol table small).

wholeQuick	EQU	1

srcCopy	EQU	0
srcOr	EQU	1
srcXor	EQU	2
srcBic	EQU	3
notSrcCopy	EQU	4
notSrcOr	EQU	5
notSrcXor	EQU	6
notSrcBic	EQU	7
patCopy	EQU	8
patOr	EQU	9
patXor	EQU	10
patBic	EQU	11
notPatCopy	EQU	12
notPatOr	EQU	13
notPatXor	EQU	14
notPatBic	EQU	15

boldBit	EQU	0
italicBit	EQU	1
ulineBit	EQU	2
outlineBit	EQU	3
shadowBit	EQU	4
condenseBit	EQU	5
extendBit	EQU	6

ascent	EQU	0
descent	EQU	2
widMax	EQU	4
leading	EQU	6

v	EQU	0
h	EQU	2

topLeft	EQU	0
botRight	EQU	4
top	EQU	0
left	EQU	2
bottom	EQU	4
right	EQU	6

baseAddr	EQU	0
rowBytes	EQU	4
bounds	EQU	6
bitmapRec	EQU	14

data	EQU	0
mask	EQU	$20
hotSpot	EQU	$40
cursRec	EQU	68

psLoc	EQU	0
psSize	EQU	4
psMode	EQU	8
psPat	EQU	10
psRec	EQU	18

polySize	EQU	0
polyBBox	EQU	2
polyPoints	EQU	10

rgnSize	EQU	0
rgnBBox	EQU	2
rgnData	EQU	10

picSize	EQU	0
picFrame	EQU	2
picData	EQU	10

textProc	EQU	0
lineProc	EQU	4
rectProc	EQU	8
rRectProc	EQU	$C
ovalProc	EQU	$10
arcProc	EQU	$14
polyProc	EQU	$18
rgnProc	EQU	$1C
bitsProc	EQU	$20
commentProc	EQU	$24
txMeasProc	EQU	$28
getPicProc	EQU	$2C
putPicProc	EQU	$30
qdProcsRec	EQU	$34

device	EQU	$0
portBits	EQU	$2
portBounds	EQU	$8
portRect	EQU	$10
visRgn	EQU	$18
clipRgn	EQU	$1C
bkPat	EQU	$20
fillPat	EQU	$28
pnLoc	EQU	$30
pnSize	EQU	$34
pnMode	EQU	$38
pnPat	EQU	$3A
pnVis	EQU	$42
txFont	EQU	$44
txFace	EQU	$46
txMode	EQU	$48
txSize	EQU	$4A
spExtra	EQU	$4C
fgColor	EQU	$50
bkColor	EQU	$54
colrBit	EQU	$58
patStretch	EQU	$5A
picSave	EQU	$5C
rgnSave	EQU	$60
polySave	EQU	$64
grafProcs	EQU	$68
portRec	EQU	$6C

GrafGlobals	EQU	0

thePort	EQU	$0
white	EQU	$FFFFFFF8
black	EQU	$FFFFFFF0
gray	EQU	$FFFFFFE8
ltGray	EQU	$FFFFFFE0
dkGray	EQU	$FFFFFFD8
arrow	EQU	$FFFFFF94
screenBits	EQU	$FFFFFF86
randSeed	EQU	$FFFFFF82

	IF	 wholeQuick

normalBit	EQU	0
inverseBit	EQU	1
redBit	EQU	4
greenBit	EQU	3
blueBit	EQU	2
cyanBit	EQU	8
magentaBit	EQU	7
yellowBit	EQU	6
blackBit	EQU	5
blackColor	EQU	33
whiteColor	EQU	30
redColor	EQU	205
greenColor	EQU	341
blueColor	EQU	409
cyanColor	EQU	273
magentaColor	EQU	137
yellowColor	EQU	69

picLParen	EQU	0
picRParen	EQU	1

frame	EQU	0
paint	EQU	1
erase	EQU	2
invert	EQU	3
fill	EQU	4

wideOpen	EQU	$FFFFFF7E
wideMaster	EQU	$FFFFFF7A
wideData	EQU	$FFFFFF70
rgnBuf	EQU	$FFFFFF6C
rgnIndex	EQU	$FFFFFF6A
rgnMax	EQU	$FFFFFF68
playPic	EQU	$FFFFFF64
playIndex	EQU	$FFFFFF62
thePoly	EQU	$FFFFFF5E
polyMax	EQU	$FFFFFF5C
patAlign	EQU	$FFFFFF58
fontAdj	EQU	$FFFFFF54
fontPtr	EQU	$FFFFFF50
fontData	EQU	$FFFFFF36
lastGrafGlob	EQU	$FFFFFF36
grafSize	EQU	$CE

	ENDIF

