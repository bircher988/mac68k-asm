; SysEqu.D - Operating System equates (low-memory globals, queues, parameter blocks)
; Part of mac68k-asm (https://github.com/bircher988/mac68k-asm). Names and values as
; documented in Inside Macintosh; generated file, do not edit by hand.

; The block after 'IF wholeSystem=1' is only assembled when the program defines that symbol,
; as in the original files (it keeps the symbol table small).

wholeSystem	EQU	1

ROM85	EQU	$28E
ROMMapHndl	EQU	$B06

ScrVRes	EQU	$102
ScrHRes	EQU	$104
ScrnBase	EQU	$824
ScreenRow	EQU	$106

MBTicks	EQU	$16E
JKybdTask	EQU	$21A
KeyLast	EQU	$184
KeyTime	EQU	$186
KeyRepTime	EQU	$18A

SPConfig	EQU	$1FB
SPPortA	EQU	$1FC
SPPortB	EQU	$1FE

useFree	EQU	0
useATalk	EQU	1
useAsync	EQU	2
useExtClk	EQU	3

DoubleTime	EQU	$2F0
CaretTime	EQU	$2F4
KeyThresh	EQU	$18E
KeyRepThresh	EQU	$190
SdVolume	EQU	$260

Ticks	EQU	$16A
Time	EQU	$20C

iBeamCursor	EQU	1
crossCursor	EQU	2
plusCursor	EQU	3
watchCursor	EQU	4

sortsBefore	EQU	-1
sortsEqual	EQU	0
sortsAfter	EQU	1

qInUse	EQU	7

qHeadSize	EQU	$A
qFlags	EQU	0
qHead	EQU	2
qTail	EQU	6

qLink	EQU	0
qType	EQU	4

evType	EQU	4

nullEvt	EQU	0
mButDwnEvt	EQU	1
mButUpEvt	EQU	2
keyDwnEvt	EQU	3
keyUpEvt	EQU	4
autoKeyEvt	EQU	5
updatEvt	EQU	6
diskInsertEvt	EQU	7
activateEvt	EQU	8
netWorkEvt	EQU	$A
ioDrvrEvt	EQU	$B
app1Evt	EQU	$C
app2Evt	EQU	$D
app3Evt	EQU	$E
app4Evt	EQU	$F

activeFlag	EQU	$0
btnState	EQU	$7
cmdKey	EQU	$8
shiftKey	EQU	$9
alphaLock	EQU	$A
optionKey	EQU	$B

evtNum	EQU	0
evtMessage	EQU	2
evtTicks	EQU	6
evtMouse	EQU	$A
evtMeta	EQU	$E
evtMBut	EQU	$F
evtBlkSize	EQU	$10

MonkeyLives	EQU	$100
SEvtEnb	EQU	$15C
JournalFlag	EQU	$8DE
JournalRef	EQU	$8E8

lock	EQU	7
purge	EQU	6
resourc	EQU	5

BufPtr	EQU	$10C
StkLowPt	EQU	$110
TheZone	EQU	$118
ApplLimit	EQU	$130
SysZone	EQU	$2A6
ApplZone	EQU	$2AA
HeapEnd	EQU	$114
HiHeapMark	EQU	$BAE

MemErr	EQU	$220

maxSize	EQU	$800000
dfltStackSize	EQU	$00002000
mnStackSize	EQU	$00000400

startPtr	EQU	0
limitPtr	EQU	4
cMoreMasters	EQU	8
pGrowZone	EQU	10

killCode	EQU	1
drvStsCode	EQU	8
ejectCode	EQU	7
tgBuffCode	EQU	8

dReadEnable	EQU	0
dWritEnable	EQU	1
dCtlEnable	EQU	2
dStatEnable	EQU	3
dNeedGoodBye	EQU	4
dNeedTime	EQU	5
dNeedLock	EQU	6

dOpened	EQU	5
dRAMBased	EQU	6
drvrActive	EQU	7

dQDrive	EQU	$6
dQRefNum	EQU	$8
dQFSID	EQU	$A
dQDrvSz	EQU	$C
dQDrvSz2	EQU	$E

ioQType	EQU	2
drvQType	EQU	3
fsQType	EQU	5

dCtlEntrySize	EQU	$28
dCtlDriver	EQU	0
dCtlFlags	EQU	4
dCtlQueue	EQU	6
dCtlQHead	EQU	8
dCtlQTail	EQU	$C
dCtlPosition	EQU	$10
dCtlStorage	EQU	$14
dCtlRefNum	EQU	$18
dCtlCurTicks	EQU	$1A
dCtlWindow	EQU	$1E
dCtlDelay	EQU	$22
dCtlEMask	EQU	$24
dCtlMenu	EQU	$26

UTableBase	EQU	$11C
UnitNtryCnt	EQU	$1D2

JFetch	EQU	$8F4
JStash	EQU	$8F8
JIODone	EQU	$8FC

chooserID	EQU	1

wDev	EQU	0

fsAtMark	EQU	0
fsFromStart	EQU	1
fsFromLEOF	EQU	2
fsFromMark	EQU	3
rdVerify	EQU	$40

fsCurPerm	EQU	0
fsRdPerm	EQU	1
fsWrPerm	EQU	2
fsRdWrPerm	EQU	3
fsRdWrShPerm	EQU	4

ioQElSize	EQU	$32

ioLink	EQU	$0
ioType	EQU	$4
ioTrap	EQU	$6
ioCmdAddr	EQU	$8

ioCompletion	EQU	$C
ioResult	EQU	$10
ioFileName	EQU	$12
ioVRefNum	EQU	$16
ioDrvNum	EQU	$16
ioRefNum	EQU	$18

ioFileType	EQU	$1A

ioBuffer	EQU	$20
ioByteCount	EQU	$24
ioNumDone	EQU	$28

ioPosMode	EQU	$2C
ioPosOffset	EQU	$2E

ioReqCount	EQU	$24
ioActCount	EQU	$28

ioPermssn	EQU	$1B
ioOwnBuf	EQU	$1C

ioNewName	EQU	$1C

ioFQElSize	EQU	$50

ioFDirIndex	EQU	$1C
ioFlAttrib	EQU	$1E
ioFFlType	EQU	$1F
ioFlUsrWds	EQU	$20
ioFFlNum	EQU	$30
ioDirID	EQU	$30

ioFlStBlk	EQU	$34
ioFlLgLen	EQU	$36
ioFlPyLen	EQU	$3A
ioFlRStBlk	EQU	$3E
ioFlRLgLen	EQU	$40
ioFlRPyLen	EQU	$44

ioFlCrDat	EQU	$48
ioFlMdDat	EQU	$4C

ioLEOF	EQU	$1C

ioNewType	EQU	$1C

ioVQElSize	EQU	$40
ioVDrvNum	EQU	$16
ioVNPtr	EQU	$12
ioVolIndex	EQU	$1C

ioVCrDate	EQU	$1E
ioVLsBkUp	EQU	$22
ioVAtrb	EQU	$26
ioVNmFls	EQU	$28
ioVDirSt	EQU	$2A
ioVBlLn	EQU	$2C
ioVNmAlBlks	EQU	$2E
ioVAlBlkSiz	EQU	$30
ioVClpSiz	EQU	$34
ioAlBlSt	EQU	$38
ioVNxtFNum	EQU	$3A
ioVFrBlk	EQU	$3E

fsRtParID	EQU	1
fsRtDirID	EQU	2
fsXTCNID	EQU	3
fsCTCNID	EQU	4
fsUsrCNID	EQU	$10

ioFlBkDat	EQU	$50
ioFlxFndrInfo	EQU	$54
ioFlParID	EQU	$64
ioFlClpSiz	EQU	$68

ioDirFlg	EQU	4
ioDrUsrWds	EQU	$20
ioDrDirID	EQU	$30
ioDrNmFls	EQU	$34
ioDrCrDat	EQU	$48
ioDrMdDat	EQU	$4C
ioDrBkDat	EQU	$50
ioDrFndrInfo	EQU	$54
ioDrParID	EQU	$64

ioHFQElSiz	EQU	$6C

ioVLsMod	EQU	$22
ioVSigWord	EQU	$40
ioVCBVBMst	EQU	$2A
ioVNxtCNID	EQU	$3A
ioVDrvInfo	EQU	$42
ioVDRefNum	EQU	$44
ioVFSID	EQU	$46
ioVBkup	EQU	$48
ioVSeqNum	EQU	$4C
ioVWrCnt	EQU	$4E
ioVFilCnt	EQU	$52
ioVDirCnt	EQU	$56
ioVFndrInfo	EQU	$5A

ioHVQElSize	EQU	$7A

ioFCBIndx	EQU	$1C
ioFCBFlNm	EQU	$20
ioFCBFlags	EQU	$24
ioFCBStBlk	EQU	$26
ioFCBEOF	EQU	$28
ioFCBPLen	EQU	$2C
ioFCBCrPs	EQU	$30
ioFCBVRefNum	EQU	$34
ioFCBClpSiz	EQU	$36
ioFCBParID	EQU	$3A

ioWDIndex	EQU	$1A
ioWDProcID	EQU	$1C
ioWDVRefNum	EQU	$20
ioWDDirID	EQU	$30

ioFSVrsn	EQU	$20

ioNewDirID	EQU	$24

csCode	EQU	$1A
csParam	EQU	$1C

fdType	EQU	$0
fdCreator	EQU	$4
fdFlags	EQU	$8
fdLocation	EQU	$A
fdFldr	EQU	$E

fdIconID	EQU	$0
fdUnused	EQU	$2
fdComment	EQU	$A
fdPutAway	EQU	$C

frRect	EQU	$0
frFlags	EQU	$8
frLocation	EQU	$A
frView	EQU	$E

frScroll	EQU	$0
frOpenChain	EQU	$4
frUnused	EQU	$8
frComment	EQU	$A
frPutAway	EQU	$C

fHasBundle	EQU	13
fInvisible	EQU	14
fTrash	EQU	-3
fDeskTop	EQU	-2
fDisk	EQU	0

DrvQHdr	EQU	$308
BootDrive	EQU	$210
EjectNotify	EQU	$338
IAZNotify	EQU	$33C
SFSaveDisk	EQU	$214
CurDirStore	EQU	46+$36A
ROMDoEject	EQU	$1E

dtYear	EQU	$0
dtMonth	EQU	$2
dtDay	EQU	$4
dtHour	EQU	$6
dtMinute	EQU	$8
dtSecond	EQU	$A
dtDayOfWeek	EQU	$C

OneOne	EQU	$A02
MinusOne	EQU	$A06
Lo3Bytes	EQU	$31A

ROMBase	EQU	$2AE
RAMBase	EQU	$2B2
SysVersion	EQU	$15A
RndSeed	EQU	$156

scratch20	EQU	$1E4
scratch8	EQU	$9FA

ScrapSize	EQU	$960
ScrapHandle	EQU	$964
ScrapCount	EQU	$968
ScrapState	EQU	$96A
ScrapName	EQU	$96C

accEvent	EQU	$40
accRun	EQU	$41
accCursor	EQU	$42
accMenu	EQU	$43
accUndo	EQU	$44
accCut	EQU	$46
accCopy	EQU	$47
accPaste	EQU	$48
accClear	EQU	$49

goodBye	EQU	-1

	IF wholeSystem=1

IntlSpec	EQU	$BA0

SwitcherTPtr	EQU	$286

tSysOrCurZone	EQU	10

clearBit	EQU	9

CPUFlag	EQU	$12F

sndBufWLen	EQU	$0172

VIA	EQU	$1D4

vBufB	EQU	0
vBufAH	EQU	$200
vDIRB	EQU	$400
vDIRA	EQU	$600
vT1C	EQU	$800
vT1CH	EQU	$A00
vT1L	EQU	$C00
vT1LH	EQU	$E00
vT2C	EQU	$1000
vT2CH	EQU	$1200
vSR	EQU	$1400
vACR	EQU	$1600
vPCR	EQU	$1800
vIFR	EQU	$1A00
vIER	EQU	$1C00
vBufA	EQU	$1E00

vBufD	EQU	$1E00

vAOut	EQU	$7F
vAInit	EQU	$7B
vSound	EQU	$7
vSndPg2	EQU	3
vOverlay	EQU	4
vHeadSel	EQU	5
vPage2	EQU	6
vSCCWrReq	EQU	7

vBOut	EQU	$87
vBInit	EQU	$07
rtcData	EQU	0
rtcClk	EQU	1
rtcEnb	EQU	2
vSW	EQU	3
vX2	EQU	4
vY2	EQU	5
vH4	EQU	6
vSndEnb	EQU	7

SCCRd	EQU	$1D8
SCCWr	EQU	$1DC

aData	EQU	6
aCtl	EQU	2
bData	EQU	4
bCtl	EQU	0

sccData	EQU	4

rxBF	EQU	0
txBE	EQU	2

IWM	EQU	$1E0

ph0L	EQU	0
ph0H	EQU	$200
ph1L	EQU	$400
ph1h	EQU	$600
ph2L	EQU	$800
ph2H	EQU	$A00
ph3L	EQU	$C00
ph3H	EQU	$E00
mtrOff	EQU	$1000
mtrOn	EQU	$1200
intDrive	EQU	$1400
extDrive	EQU	$1600
q6L	EQU	$1800
q6H	EQU	$1A00
q7L	EQU	$1C00
q7H	EQU	$1E00

Lvl1DT	EQU	$192
Lvl2DT	EQU	$1B2
ExtStsDT	EQU	$2BE

SPValid	EQU	$1F8
SPATalkA	EQU	$1F9
SPATalkB	EQU	$1FA
SPAlarm	EQU	$200
SPFont	EQU	$204
SPKbd	EQU	$206

SPPrint	EQU	$207
SPVolCtl	EQU	$208
SPClikCaret	EQU	$209

SPMisc1	EQU	$20A
SPMisc2	EQU	$20B

GetParam	EQU	$1E4
SysParam	EQU	$1F8

CrsrThresh	EQU	$8EC
JCrsrTask	EQU	$8EE
MTemp	EQU	$828
RawMouse	EQU	$82C
CrsrRect	EQU	$83C
TheCrsr	EQU	$844
CrsrAddr	EQU	$888
CrsrSave	EQU	$88C
CrsrVis	EQU	$8CC
CrsrBusy	EQU	$8CD
CrsrNew	EQU	$8CE
CrsrState	EQU	$8D0
CrsrObscure	EQU	$8D2

KbdVars	EQU	$216
KbdType	EQU	$21E
MBState	EQU	$172
KeyMap	EQU	$174
KeypadMap	EQU	$17C
Key1Trans	EQU	$29E
Key2Trans	EQU	$2A2
JGNEFilter	EQU	$29A
KeyMVars	EQU	$B04

Mouse	EQU	$830
CrsrPin	EQU	$834
CrsrCouple	EQU	$8CF
CrsrScale	EQU	$8D3
MouseMask	EQU	$8D6
MouseOffset	EQU	$8DA

AlarmState	EQU	$21F

vType	EQU	1
inVbl	EQU	6

vblink	EQU	0
vblType	EQU	4
vblAddr	EQU	6
vblCount	EQU	$A
vblPhase	EQU	$C

VBLQueue	EQU	$160

jPlayCtl	EQU	16
jRecordCtl	EQU	17
jcTickCount	EQU	0
jcGetMouse	EQU	1
jcButton	EQU	2
jcGetKeys	EQU	3
jcEvent	EQU	4

SysEvtMask	EQU	$144
SysEvtBuf	EQU	$146
EventQueue	EQU	$14A
EvtBufCnt	EQU	$154

evtQWhat	EQU	6
evtQMessage	EQU	8
evtQWhen	EQU	$C
evtQWhere	EQU	$10
evtQMeta	EQU	$14
evtQMBut	EQU	$15
evtQBlkSize	EQU	$16

fOnCheck	EQU	0
fChecking	EQU	1
fNSelCompct	EQU	4
fNoRvrAlloc	EQU	5
fNSelPurge	EQU	6
fRelAtEnd	EQU	7

ROZ	EQU	$0

tybkMask	EQU	3
tybkFree	EQU	0
tybkNRel	EQU	1
tybkRel	EQU	2

tagBC	EQU	0
handle	EQU	4
blkData	EQU	8

bkLim	EQU	$0
purgePtr	EQU	$4
hFstFree	EQU	$8
zcbFree	EQU	$C
gzProc	EQU	$10
mAllocCnt	EQU	$14
flags	EQU	$16
cntRel	EQU	$18
maxRel	EQU	$1A
cntNRel	EQU	$1C
maxNRel	EQU	$1E
cntEmpty	EQU	$20
cntHandles	EQU	$22
minCBFree	EQU	$24
purgeProc	EQU	$28
allocPtr	EQU	$30
heapData	EQU	$34

GZRootHnd	EQU	$328
GZRootPtr	EQU	$32C
GZMoveHnd	EQU	$330
MemTop	EQU	$108
MmInOK	EQU	$12E
HpChk	EQU	$316
MaskBC	EQU	$31A
MaskHandle	EQU	$31A
MaskPtr	EQU	$31A
MinStack	EQU	$31E
DefltStack	EQU	$322
MMDefFlags	EQU	$326

DSAlertTab	EQU	$2BA
DSAlertRect	EQU	$3F8
DSDrawProc	EQU	$334
DSWndUpdate	EQU	$15D
WWExist	EQU	$8F2
QDExist	EQU	$8F3
ResumeProc	EQU	$A8C
DSErrCode	EQU	$AF0

dskRfN	EQU	$FFFB
IntFlag	EQU	$15F

SerialVars	EQU	$2D0

PortAUse	EQU	$290

PortBUse	EQU	$291
SCCASts	EQU	$2CE
SCCBSts	EQU	$2CF

shFXOn	EQU	$0
shFCTS	EQU	$1
shXOn	EQU	$2
shXOff	EQU	$3
shErrs	EQU	$4
shEvts	EQU	$5
shFInX	EQU	$6
shNull	EQU	$7

ssCumErrs	EQU	$0
ssXOffSent	EQU	$1
ssRdPend	EQU	$2
ssWrPend	EQU	$3
ssCTSHold	EQU	$4
ssXOffHold	EQU	$5

drvrFlags	EQU	$0
drvrDelay	EQU	$2
drvrEMask	EQU	$4
drvrMenu	EQU	$6
drvrOpen	EQU	$8
drvrPrime	EQU	$A
drvrCtl	EQU	$C
drvrStatus	EQU	$E
drvrClose	EQU	$10
drvrName	EQU	$12

dsTrack	EQU	$0
dsWriteProt	EQU	$2
dsDiskInPlace	EQU	$3
dsInstalled	EQU	$4
dsSides	EQU	$5
dsQLink	EQU	$6
dsDQVers	EQU	$A
dsDQDrive	EQU	$C
dsDQRefNum	EQU	$E
dsDQFSID	EQU	$10
dsTwoSideFmt	EQU	$12
dsDiskErrs	EQU	$14

dsDrvSize	EQU	$12
dsDrvS1	EQU	$14
dsDrvType	EQU	$16
dsDrvManf	EQU	$18
dsDrvChar	EQU	$1A
dsDrvMisc	EQU	$1C

DskErr	EQU	$142
PWMBuf2	EQU	$312

dcRead	EQU	0
dcWrite	EQU	1
dcStatus	EQU	3
dcInit	EQU	25
dcScan	EQU	26

SoundPtr	EQU	$262
SoundBase	EQU	$266
SoundVBL	EQU	$26A
SoundDCE	EQU	$27A
SoundActive	EQU	$27E
SoundLevel	EQU	$27F
CurPitch	EQU	$280

noQueueBit	EQU	$9
asyncTrpBit	EQU	$A

toExtFS	EQU	$3F2

DskVerify	EQU	$12C
TagData	EQU	$2FA
BufTgFNum	EQU	$2FC
BufTgFFlg	EQU	$300
BufTgFBkNum	EQU	$302
BufTgDate	EQU	$304

aRdCmd	EQU	2
aWrCmd	EQU	3
aCtlCmd	EQU	4
aStsCmd	EQU	5

ioPMSPFlg	EQU	$1A
ioPMSPHook	EQU	$1C

ScrDmpEnb	EQU	$2F8
ScrDmpType	EQU	$2F9

ScrapVars	EQU	$960
ScrapInfo	EQU	$960
ScrapEnd	EQU	$980
ScrapTag	EQU	$970

LaunchFlag	EQU	$902
SaveSegHandle	EQU	$930
CurJTOffset	EQU	$934
CurPageOption	EQU	$936
LoaderPBlock	EQU	$93A
CurApRefNum	EQU	$900
CurrentA5	EQU	$904
CurStackBase	EQU	$908
CurApName	EQU	$910
LoadTrap	EQU	$12D

SegHiEnable	EQU	$BB2

newSelMsg	EQU	12
fillListMsg	EQU	13
getSelMsg	EQU	14
selectMsg	EQU	15
deselectMsg	EQU	16
terminateMsg	EQU	17
buttonMsg	EQU	19

TheChooser	EQU	1

	ENDIF

