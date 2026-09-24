; SysErr.D - system error codes
; Part of mac68k-asm (https://github.com/bircher988/mac68k-asm). Names and values as
; documented in Inside Macintosh; generated file, do not edit by hand.

; The block after 'IF	 wholeErrors' is only assembled when the program defines that symbol,
; as in the original files (it keeps the symbol table small).

wholeErrors	EQU	1

noErr	EQU	0
qErr	EQU	-1
vTypErr	EQU	-2
corErr	EQU	-3
unimpErr	EQU	-4

controlErr	EQU	-17
statusErr	EQU	-18
readErr	EQU	-19
writErr	EQU	-20
badUnitErr	EQU	-21
unitEmptyErr	EQU	-22
openErr	EQU	-23
closErr	EQU	-24
dRemovErr	EQU	-25
dInstErr	EQU	-26
abortErr	EQU	-27
notOpenErr	EQU	-28

dirFulErr	EQU	-33
dskFulErr	EQU	-34
nsvErr	EQU	-35
ioErr	EQU	-36
bdNamErr	EQU	-37
fnOpnErr	EQU	-38
eofErr	EQU	-39
posErr	EQU	-40
mFulErr	EQU	-41
tmfoErr	EQU	-42
fnfErr	EQU	-43

wPrErr	EQU	-44
fLckdErr	EQU	-45
vLckdErr	EQU	-46
fBsyErr	EQU	-47
dupFNErr	EQU	-48
opWrErr	EQU	-49
paramErr	EQU	-50
rfNumErr	EQU	-51
gfpErr	EQU	-52
volOffLinErr	EQU	-53
permErr	EQU	-54
volOnLinErr	EQU	-55
nsDrvErr	EQU	-56
noMacDskErr	EQU	-57
extFSErr	EQU	-58
fsRnErr	EQU	-59

badMDBErr	EQU	-60
wrPermErr	EQU	-61

firstDskErr	EQU	-84
lastDskErr	EQU	-64

noDriveErr	EQU	-64
offLinErr	EQU	-65

noNybErr	EQU	-66
noAdrMkErr	EQU	-67
dataVerErr	EQU	-68
badCkSmErr	EQU	-69
badBtSlpErr	EQU	-70
noDtaMkErr	EQU	-71
badDCkSum	EQU	-72
badDBtSlp	EQU	-73
wrUnderRun	EQU	-74

cantStepErr	EQU	-75
tk0BadErr	EQU	-76
initIWMErr	EQU	-77
twoSideErr	EQU	-78
spdAdjErr	EQU	-79
seekErr	EQU	-80
sectNFErr	EQU	-81

clkRdErr	EQU	-85
clkWrErr	EQU	-86
prWrErr	EQU	-87
prInitErr	EQU	-88

rcvrErr	EQU	-89
breakRecd	EQU	-90

ddpSktErr	EQU	-91
ddpLenErr	EQU	-92
noBridgeErr	EQU	-93
lapProtErr	EQU	-94
excessCollsns	EQU	-95
portInUse	EQU	-97
portNotCf	EQU	-98

memFullErr	EQU	-108
nilHandleErr	EQU	-109
memWZErr	EQU	-111
memPurErr	EQU	-112

memAdrErr	EQU	-110
memAZErr	EQU	-113
memPCErr	EQU	-114
memBCErr	EQU	-115
memSCErr	EQU	-116
memLockedErr	EQU	-117

resNotFound	EQU	-192
resFNotFound	EQU	-193
addResFailed	EQU	-194
addRefFailed	EQU	-195
rmvResFailed	EQU	-196
rmvRefFailed	EQU	-197

noScrapErr	EQU	-100
noTypeErr	EQU	-102

evtNotEnb	EQU	1

	IF	 wholeErrors

dsSysErr	EQU	32767
dsBusError	EQU	1
dsAddressErr	EQU	2
dsIllInstErr	EQU	3
dsZeroDivErr	EQU	4
dsChkErr	EQU	5
dsOvFlowErr	EQU	6
dsPrivErr	EQU	7
dsTraceErr	EQU	8
dsLineAErr	EQU	9
dsLineFErr	EQU	10
dsMiscErr	EQU	11
dsCoreErr	EQU	12
dsIrqErr	EQU	13

dsIOCoreErr	EQU	14
dsLoadErr	EQU	15
dsFPErr	EQU	16

dsNoPackErr	EQU	17
dsNoPk1	EQU	18
dsNoPk2	EQU	19
dsNoPk3	EQU	20
dsNoPk4	EQU	21
dsNoPk5	EQU	22
dsNoPk6	EQU	23
dsNoPk7	EQU	24

dsMemFullErr	EQU	25
dsBadLaunch	EQU	26

dsFSErr	EQU	27
dsStknHeap	EQU	28
dsReinsert	EQU	30
dsNotThe1	EQU	31

memTrbBase	EQU	32
mtSetLog	EQU	32
mtAdjFre	EQU	33
mtAdjCnt	EQU	34
mtMkeBkf	EQU	35
mtSetSiz	EQU	36
mtInitMem	EQU	37
mtBCerr	EQU	38
mtCZerr	EQU	39
mtCZ1err	EQU	40
mtCZ2err	EQU	41
mtCZ3err	EQU	42
mtEqCerr	EQU	43
mtEvCerr	EQU	44
mtHCerr	EQU	45
mtPCerr	EQU	46
mtSCerr	EQU	47
mtRC1err	EQU	48
mtRC2err	EQU	49
mtSABerr	EQU	50
mtACerr	EQU	51
mtIZCerr	EQU	52
mtPrCerr	EQU	53

	ENDIF

