; ToolEqu.D - Toolbox equates (windows, controls, menus, dialogs, text edit)
; Part of mac68k-asm (https://github.com/bircher988/mac68k-asm). Names and values as
; documented in Inside Macintosh; generated file, do not edit by hand.

; The block after 'IF	 wholeTools' is only assembled when the program defines that symbol,
; as in the original files (it keeps the symbol table small).

wholeTools	EQU	1

resSysRef	EQU	7
resSysHeap	EQU	6
resPurgeable	EQU	5
resLocked	EQU	4
resProtected	EQU	3
resPreload	EQU	2
resChanged	EQU	1

rcbMask	EQU	$FD

mapReadOnly	EQU	7
mapCompact	EQU	6
mapChanged	EQU	5

TopMapHndl	EQU	$A50
SysMapHndl	EQU	$A54
SysMap	EQU	$A58
CurMap	EQU	$A5A
ResReadOnly	EQU	$A5C
ResLoad	EQU	$A5E
ResErr	EQU	$A60
ResErrProc	EQU	$AF2
SysResName	EQU	$AD8

sysFont	EQU	0
applFont	EQU	1

newYork	EQU	2
geneva	EQU	3
monaco	EQU	4
venice	EQU	5
london	EQU	6
athens	EQU	7
sanFran	EQU	8
toronto	EQU	9
cairo	EQU	11
losAngeles	EQU	12

ApFontID	EQU	$984
FMDefaultSize	EQU	$987
CurFMInput	EQU	$988
FMgrOutRec	EQU	$998
FScaleDisable	EQU	$A63

dialogKind	EQU	2
userKind	EQU	8

wNoHit	EQU	0
wInContent	EQU	1
wInDrag	EQU	2
wInGrow	EQU	3
wInGoAway	EQU	4

inDesk	EQU	0
inMenuBar	EQU	1
inSysWindow	EQU	2
inContent	EQU	3
inDrag	EQU	4
inGrow	EQU	5
inGoAway	EQU	6

deskPatID	EQU	16

documentProc	EQU	0
dBoxProc	EQU	1
plainDBox	EQU	2
altDBoxProc	EQU	3
noGrowDocProc	EQU	4
rDocProc	EQU	16

windowPort	EQU	0
windowKind	EQU	$6C
wVisible	EQU	$6E
wHilited	EQU	$6F
wGoAway	EQU	$70
structRgn	EQU	$72
contRgn	EQU	$76
updateRgn	EQU	$7A
windowDef	EQU	$7E
wDataHandle	EQU	$82
wTitleHandle	EQU	$86
wTitleWidth	EQU	$8A
wControlList	EQU	$8C
nextWindow	EQU	$90
windowPic	EQU	$94
wRefCon	EQU	$98
windowSize	EQU	$9C

WindowList	EQU	$9D6
PaintWhite	EQU	$9DC
WMgrPort	EQU	$9DE
GrayRgn	EQU	$9EE
CurActivate	EQU	$A64
CurDeactive	EQU	$A68

DragHook	EQU	$9F6
DeskPattern	EQU	$A3C
DeskHook	EQU	$A6C
GhostWindow	EQU	$A84

noMark	EQU	0
commandMark	EQU	$11
checkMark	EQU	$12
diamondMark	EQU	$13
appleMark	EQU	$14

lastMenu	EQU	0
lastRight	EQU	2

menuoH	EQU	0
menuLeft	EQU	4

menuID	EQU	0
menuWidth	EQU	2
menuHeight	EQU	4
menuDefHandle	EQU	6
menuEnable	EQU	$A
menuData	EQU	$E
menuBlkSize	EQU	$E

itemIcon	EQU	0
itemCmd	EQU	1
itemMark	EQU	2
itemStyle	EQU	3

MenuList	EQU	$A1C
MenuFlash	EQU	$A24
MrMacHook	EQU	$A2C
MenuHook	EQU	$A30

inButton	EQU	10
inCheckBox	EQU	11
inUpButton	EQU	20
inDownButton	EQU	21
inPageUp	EQU	22
inPageDown	EQU	23
inThumb	EQU	129

noConstraint	EQU	0
hAxisOnly	EQU	1
vAxisOnly	EQU	2

pushButProc	EQU	0
checkBoxProc	EQU	1
radioButProc	EQU	2
scrollBarProc	EQU	16
useWFont	EQU	8

sBarPatID	EQU	17

nextControl	EQU	$0
contrlOwner	EQU	$4
contrlRect	EQU	$8
contrlVis	EQU	$10
contrlHilite	EQU	$11
contrlValue	EQU	$12
contrlMin	EQU	$14
contrlMax	EQU	$16
contrlDefHandle	EQU	$18
contrlData	EQU	$1C
contrlAction	EQU	$20
contrlRFcon	EQU	$24
contrlTitle	EQU	$28
contrlSize	EQU	$28

DragPattern	EQU	$A34
DragFlag	EQU	$A44
CurDragAction	EQU	$A46

teJustLeft	EQU	0
teJustRight	EQU	-1
teJustCenter	EQU	1

teDestRect	EQU	$0
teViewRect	EQU	$8

teSelRect	EQU	$10
teLineHite	EQU	$18
teAscent	EQU	$1A
teSelPoint	EQU	$1C

teSelStart	EQU	$20
teSelEnd	EQU	$22

teActive	EQU	$24

teWordBreak	EQU	$26
teClikProc	EQU	$2A

teClikTime	EQU	$2E
teClikLoc	EQU	$32

teCarTime	EQU	$34
teCarOn	EQU	$38
teCarAct	EQU	$39
teJust	EQU	$3A

teLength	EQU	$3C
teTextH	EQU	$3E

teRecBack	EQU	$42
teRecLine	EQU	$44
teLftClick	EQU	$46
teLftCaret	EQU	$47

teCROnly	EQU	$48

teFontStuff	EQU	$4A
teFont	EQU	$4A
teFace	EQU	$4C
teMode	EQU	$4E
teSize	EQU	$50
teGrafPort	EQU	$52

teHiHook	EQU	$56
teCarHook	EQU	$5A

teNLines	EQU	$5E
teLines	EQU	$60

teRecSize	EQU	$68

TEScrpLength	EQU	$AB0
TEScrpHandle	EQU	$AB4
TEWdBreak	EQU	$AF6

okButton	EQU	1
cancelButton	EQU	2

stopIcon	EQU	0
noteIcon	EQU	1
cautionIcon	EQU	2

dBounds	EQU	$0
dWindProc	EQU	$8
dVisible	EQU	$A
dGoAway	EQU	$C
dRefCon	EQU	$E
dItems	EQU	$12
dTitle	EQU	$14

aBounds	EQU	$0
aItems	EQU	$8
aStages	EQU	$A

dWindow	EQU	$0
items	EQU	$9C
teHandle	EQU	$A0
editField	EQU	$A4
editOpen	EQU	$A6
aDefItem	EQU	$A8
dWindLen	EQU	$AA

itmHndl	EQU	0
itmRect	EQU	$4
itmType	EQU	$C
itmData	EQU	$D

ANumber	EQU	$A98
ACount	EQU	$A9A
DABeeper	EQU	$A9C
DAStrings	EQU	$AA0
DlgFont	EQU	$AFA

AppPacks	EQU	$AB8
FPState	EQU	$A4A

FinderName	EQU	$2E0
AppParmHandle	EQU	$AEC

ApplScratch	EQU	$A78
ToolScratch	EQU	$9CE
TempRect	EQU	$9FA

	IF	 wholeTools

mCCMask	EQU	$60
mChMask	EQU	$20
mCoMask	EQU	$40

propFont	EQU	$9000
fixedFont	EQU	$B000
fontWid	EQU	$ACB0

fMgrCtl1	EQU	8

fontDecError	EQU	-64
fontNotDeclared	EQU	-65
fontSubErr	EQU	-66

fFontType	EQU	0
fFirstChar	EQU	2
fLastChar	EQU	4
fWidMax	EQU	6
fKernMax	EQU	8
fNDescent	EQU	10
fFRectWidth	EQU	12
fFRectHeight	EQU	14
fOWTLoc	EQU	16
fAscent	EQU	18
fDescent	EQU	20
fLeading	EQU	22
fRowWords	EQU	24

fmInFamily	EQU	0
fmInSize	EQU	2
fmInFace	EQU	4
fmInNeedBits	EQU	5
fmInDevice	EQU	6
fmInNumer	EQU	8
fmInDenom	EQU	12

fmOutError	EQU	0
fmOutFontH	EQU	2
fmOutBold	EQU	6
fmOutItalic	EQU	7
fmOutULOffset	EQU	8
fmOutULShadow	EQU	9
fmOutULThick	EQU	10
fmOutShadow	EQU	11
fmOutExtra	EQU	12
fmOutAscent	EQU	13
fmOutDescent	EQU	14
fmOutWidMax	EQU	15
fmOutLeading	EQU	16
fmOutNumer	EQU	18
fmOutDenom	EQU	22

dpiVert	EQU	0
dpiHoriz	EQU	2
boldChr	EQU	4
italChr	EQU	7

outlineChr	EQU	13
shadowChr	EQU	16
condChr	EQU	19
extendChr	EQU	22
underChr	EQU	25

CurFMFamily	EQU	$988
CurFMSize	EQU	$98A
CurFMFace	EQU	$98C
CurFMNeedBits	EQU	$98D
CurFMDevice	EQU	$98E
CurFMNumer	EQU	$990
CurFMDenom	EQU	$994

FOutRec	EQU	$998

FMDotsPerInch	EQU	$9B2
FMStyleTab	EQU	$9B6

RomFont0	EQU	$980
GotStrike	EQU	$986

wDrawMsg	EQU	0
wHitMsg	EQU	1
wCalcRgnMsg	EQU	2
wInitMsg	EQU	3
wDisposeMsg	EQU	4
wGrowMsg	EQU	5
wGIconMsg	EQU	6

OldStructure	EQU	$9E6
OldContent	EQU	$9EA
SaveVisRgn	EQU	$9F2
CurDeKind	EQU	$A22
SaveUpdate	EQU	$9DA

mDrawMsg	EQU	0
mChooseMsg	EQU	1
mSizeMsg	EQU	2

textMenuProc	EQU	0

maxMenu	EQU	$60
mListSize	EQU	$66

MBarEnable	EQU	$A20
TheMenu	EQU	$A26
SavedHandle	EQU	$A28

drawCtlMsg	EQU	0
hitCtlMsg	EQU	1
calcCtlMsg	EQU	2
newCtlMsg	EQU	3
dispCtlMsg	EQU	4
posCtlMsg	EQU	5
thumbCtlMsg	EQU	6
dragCtlMsg	EQU	7
trackCtlMsg	EQU	8

TEDoText	EQU	$A70
TERecal	EQU	$A74

userItem	EQU	0
ctrlItem	EQU	4
btnCtrl	EQU	0
chkCtrl	EQU	1
radCtrl	EQU	2
resCtrl	EQU	3
statText	EQU	8
editText	EQU	16
iconItem	EQU	32
picItem	EQU	64
itemDisable	EQU	128

volBits	EQU	3
alBit	EQU	4
okDismissal	EQU	8

dlgMaxIndex	EQU	0

SaveProc	EQU	$A90
SaveSP	EQU	$A94

screenRadius	EQU	$00100010
gPortSize	EQU	108

IconBitmap	EQU	$A0E
TaskLock	EQU	$A62
CloseOrnHook	EQU	$A88
MicroSoft	EQU	ApplScratch

	ENDIF

