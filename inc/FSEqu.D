; FSEqu.D - file system equates (parameter blocks, volumes)
; Part of mac68k-asm (https://github.com/bircher988/mac68k-asm). Names and values as
; documented in Inside Macintosh; generated file, do not edit by hand.

btsPrBlk	EQU	12
strtDir	EQU	2
bufSiz	EQU	512
hdrSiz	EQU	10
bufModBit	EQU	7
bufFNum	EQU	-10
bufFlags	EQU	-6
bufFBlk	EQU	-4
bufTag	EQU	-2

sigWord	EQU	$D2D7

NewMount	EQU	$34A

DrMstrBlk	EQU	$34C
FCBSPtr	EQU	$34E
DefVCBPtr	EQU	$352
VCBQHdr	EQU	$356

FSQHdr	EQU	$360
FSBusy	EQU	$360
FSQHead	EQU	$362
FSQTail	EQU	$366

ErCode	EQU	$3A2

FSQueueHook	EQU	$3E2
ExtFSHook	EQU	$3E6
DskSwtchHook	EQU	$3EA
ReqstVol	EQU	$3EE
ToExtFS	EQU	$3F2

fsVarEnd	EQU	$3F6
fsVarLth	EQU	$B6
fsClrWDLen	EQU	$A2

vcbDirty	EQU	15
vcbWrProt	EQU	7
vcbFlags	EQU	6

vcbDInfoSt	EQU	8
vcbDILen	EQU	64

vcbSigWord	EQU	8
vcbCrDate	EQU	10
vcbLsBkUp	EQU	14
vcbAtrb	EQU	18

vcbNmFls	EQU	20
vcbDirSt	EQU	22
vcbBlLn	EQU	24
vcbNmBlks	EQU	26
vcbNmAlBlks	EQU	26
vcbAlBlkSiz	EQU	28
vcbClpSiz	EQU	32
vcbAlBlSt	EQU	36
vcbNxtFNum	EQU	38
vcbFreeBks	EQU	42
vcbVN	EQU	44
vcbMaxNam	EQU	27

vcbDrvNum	EQU	72
vcbDRefNum	EQU	74
vcbFSID	EQU	76
vcbVRefNum	EQU	78
vcbMAdr	EQU	80
vcbBufAdr	EQU	84
vcbMLen	EQU	88

vcbDirIndex	EQU	90
vcbDirBlk	EQU	92

fcbModBit	EQU	7
fcbRscBit	EQU	1
fcbFlgRBit	EQU	9
fcbFlgWBit	EQU	8
fcbWrtBit	EQU	0
wrRsMask	EQU	$0300

fcbFlNm	EQU	0
fcbFlags	EQU	4
fcbMdRByt	EQU	4
fcbTypByt	EQU	5

fcbSBlk	EQU	6
fcbEOF	EQU	8
fcbPLen	EQU	12
fcbCrPs	EQU	16
fcbVPtr	EQU	20
fcbBfAdr	EQU	24
fcbFlPos	EQU	28

