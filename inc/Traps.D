; Traps.D - Toolbox and Operating System trap words
; Part of mac68k-asm (https://github.com/bircher988/mac68k-asm). Names and values as
; documented in Inside Macintosh; generated file, do not edit by hand.

; Usage: _Name, OS traps also _Name ,ASYNC / ,SYS / ,IMMED / ,CLEAR. The third field of
; every .TRAP names the ROM that first provided it: 64K (Macintosh 128K/512K) or
; 128K (Macintosh Plus, 512Ke). The assembler warns when a program built for the 64K
; ROM uses a 128K-ROM trap (MAC68K_ROM=128 turns the warning off).

; option bits for OS trap modifiers
immed	EQU	$200
clear	EQU	$200
async	EQU	$400
sys	EQU	$400
autoPop	EQU	$400
marks	EQU	$200
case	EQU	$400
newTool	EQU	$0600
newOS	EQU	$0200


; ---- QuickDraw ----
.TRAP	_CopyMask	$A817	128K
.TRAP	_MeasureText	$A837	128K
.TRAP	_GetMaskTable	$A836	128K
.TRAP	_CalcMask	$A838	128K
.TRAP	_SeedFill	$A839	128K
.TRAP	_InitCursor	$A850	64K
.TRAP	_SetCursor	$A851	64K
.TRAP	_HideCursor	$A852	64K
.TRAP	_ShowCursor	$A853	64K
.TRAP	_ShieldCursor	$A855	64K
.TRAP	_ObscureCursor	$A856	64K
.TRAP	_BitAnd	$A858	64K
.TRAP	_BitXOr	$A859	64K
.TRAP	_BitNot	$A85A	64K
.TRAP	_BitOr	$A85B	64K
.TRAP	_BitShift	$A85C	64K
.TRAP	_BitTst	$A85D	64K
.TRAP	_BitSet	$A85E	64K
.TRAP	_BitClr	$A85F	64K
.TRAP	_Random	$A861	64K
.TRAP	_ForeColor	$A862	64K
.TRAP	_BackColor	$A863	64K
.TRAP	_ColorBit	$A864	64K
.TRAP	_GetPixel	$A865	64K
.TRAP	_StuffHex	$A866	64K
.TRAP	_LongMul	$A867	64K
.TRAP	_FixMul	$A868	64K
.TRAP	_FixRatio	$A869	64K
.TRAP	_HiWord	$A86A	64K
.TRAP	_LoWord	$A86B	64K
.TRAP	_FixRound	$A86C	64K
.TRAP	_InitPort	$A86D	64K
.TRAP	_InitGraf	$A86E	64K
.TRAP	_OpenPort	$A86F	64K
.TRAP	_LocalToGlobal	$A870	64K
.TRAP	_GlobalToLocal	$A871	64K
.TRAP	_GrafDevice	$A872	64K
.TRAP	_SetPort	$A873	64K
.TRAP	_GetPort	$A874	64K
.TRAP	_SetPBits	$A875	64K
.TRAP	_PortSize	$A876	64K
.TRAP	_MovePortTo	$A877	64K
.TRAP	_SetOrigin	$A878	64K
.TRAP	_SetClip	$A879	64K
.TRAP	_GetClip	$A87A	64K
.TRAP	_ClipRect	$A87B	64K
.TRAP	_BackPat	$A87C	64K
.TRAP	_ClosePort	$A87D	64K
.TRAP	_AddPt	$A87E	64K
.TRAP	_SubPt	$A87F	64K
.TRAP	_SetPt	$A880	64K
.TRAP	_EqualPt	$A881	64K
.TRAP	_StdText	$A882	64K
.TRAP	_DrawChar	$A883	64K
.TRAP	_DrawString	$A884	64K
.TRAP	_DrawText	$A885	64K
.TRAP	_TextWidth	$A886	64K
.TRAP	_TextFont	$A887	64K
.TRAP	_TextFace	$A888	64K
.TRAP	_TextMode	$A889	64K
.TRAP	_TextSize	$A88A	64K
.TRAP	_GetFontInfo	$A88B	64K
.TRAP	_StringWidth	$A88C	64K
.TRAP	_CharWidth	$A88D	64K
.TRAP	_SpaceExtra	$A88E	64K
.TRAP	_StdLine	$A890	64K
.TRAP	_LineTo	$A891	64K
.TRAP	_Line	$A892	64K
.TRAP	_MoveTo	$A893	64K
.TRAP	_Move	$A894	64K
.TRAP	_Moov	$A894	128K
.TRAP	_HidePen	$A896	64K
.TRAP	_ShowPen	$A897	64K
.TRAP	_GetPenState	$A898	64K
.TRAP	_SetPenState	$A899	64K
.TRAP	_GetPen	$A89A	64K
.TRAP	_PenSize	$A89B	64K
.TRAP	_PenMode	$A89C	64K
.TRAP	_PenPat	$A89D	64K
.TRAP	_PenNormal	$A89E	64K
.TRAP	_StdRect	$A8A0	64K
.TRAP	_FrameRect	$A8A1	64K
.TRAP	_PaintRect	$A8A2	64K
.TRAP	_EraseRect	$A8A3	64K
.TRAP	_InverRect	$A8A4	64K
.TRAP	_FillRect	$A8A5	64K
.TRAP	_EqualRect	$A8A6	64K
.TRAP	_SetRect	$A8A7	64K
.TRAP	_OffSetRect	$A8A8	64K
.TRAP	_InSetRect	$A8A9	64K
.TRAP	_SectRect	$A8AA	64K
.TRAP	_UnionRect	$A8AB	64K
.TRAP	_Pt2Rect	$A8AC	64K
.TRAP	_PtInRect	$A8AD	64K
.TRAP	_EmptyRect	$A8AE	64K
.TRAP	_StdRRect	$A8AF	64K
.TRAP	_FrameRoundRect	$A8B0	64K
.TRAP	_PaintRoundRect	$A8B1	64K
.TRAP	_EraseRoundRect	$A8B2	64K
.TRAP	_InverRoundRect	$A8B3	64K
.TRAP	_FillRoundRect	$A8B4	64K
.TRAP	_StdOval	$A8B6	64K
.TRAP	_FrameOval	$A8B7	64K
.TRAP	_PaintOval	$A8B8	64K
.TRAP	_EraseOval	$A8B9	64K
.TRAP	_InvertOval	$A8BA	64K
.TRAP	_FillOval	$A8BB	64K
.TRAP	_SlopeFromAngle	$A8BC	64K
.TRAP	_StdArc	$A8BD	64K
.TRAP	_FrameArc	$A8BE	64K
.TRAP	_PaintArc	$A8BF	64K
.TRAP	_EraseArc	$A8C0	64K
.TRAP	_InvertArc	$A8C1	64K
.TRAP	_FillArc	$A8C2	64K
.TRAP	_PtToAngle	$A8C3	64K
.TRAP	_AngleFromSlope	$A8C4	64K
.TRAP	_StdPoly	$A8C5	64K
.TRAP	_FramePoly	$A8C6	64K
.TRAP	_PaintPoly	$A8C7	64K
.TRAP	_ErasePoly	$A8C8	64K
.TRAP	_InvertPoly	$A8C9	64K
.TRAP	_FillPoly	$A8CA	64K
.TRAP	_OpenPoly	$A8CB	64K
.TRAP	_ClosePgon	$A8CC	64K
.TRAP	_KillPoly	$A8CD	64K
.TRAP	_OffSetPoly	$A8CE	64K
.TRAP	_PackBits	$A8CF	64K
.TRAP	_UnpackBits	$A8D0	64K
.TRAP	_StdRgn	$A8D1	64K
.TRAP	_FrameRgn	$A8D2	64K
.TRAP	_PaintRgn	$A8D3	64K
.TRAP	_EraseRgn	$A8D4	64K
.TRAP	_InverRgn	$A8D5	64K
.TRAP	_FillRgn	$A8D6	64K
.TRAP	_NewRgn	$A8D8	64K
.TRAP	_DisposRgn	$A8D9	64K
.TRAP	_OpenRgn	$A8DA	64K
.TRAP	_CloseRgn	$A8DB	64K
.TRAP	_CopyRgn	$A8DC	64K
.TRAP	_SetEmptyRgn	$A8DD	64K
.TRAP	_SetRecRgn	$A8DE	64K
.TRAP	_RectRgn	$A8DF	64K
.TRAP	_OfSetRgn	$A8E0	64K
.TRAP	_InSetRgn	$A8E1	64K
.TRAP	_EmptyRgn	$A8E2	64K
.TRAP	_EqualRgn	$A8E3	64K
.TRAP	_SectRgn	$A8E4	64K
.TRAP	_UnionRgn	$A8E5	64K
.TRAP	_DiffRgn	$A8E6	64K
.TRAP	_XOrRgn	$A8E7	64K
.TRAP	_PtInRgn	$A8E8	64K
.TRAP	_RectInRgn	$A8E9	64K
.TRAP	_SetStdProcs	$A8EA	64K
.TRAP	_StdBits	$A8EB	64K
.TRAP	_CopyBits	$A8EC	64K
.TRAP	_StdTxMeas	$A8ED	64K
.TRAP	_StdGetPic	$A8EE	64K
.TRAP	_ScrollRect	$A8EF	64K
.TRAP	_StdPutPic	$A8F0	64K
.TRAP	_StdComment	$A8F1	64K
.TRAP	_PicComment	$A8F2	64K
.TRAP	_OpenPicture	$A8F3	64K
.TRAP	_ClosePicture	$A8F4	64K
.TRAP	_KillPicture	$A8F5	64K
.TRAP	_DrawPicture	$A8F6	64K
.TRAP	_ScalePt	$A8F8	64K
.TRAP	_MapPt	$A8F9	64K
.TRAP	_MapRect	$A8FA	64K
.TRAP	_MapRgn	$A8FB	64K
.TRAP	_MapPoly	$A8FC	64K

; ---- Toolbox ----
.TRAP	_Count1Resources	$A80D	128K
.TRAP	_Get1IxResource	$A80E	128K
.TRAP	_Get1IxType	$A80F	128K
.TRAP	_Unique1ID	$A810	128K
.TRAP	_TESelView	$A811	128K
.TRAP	_TEPinScroll	$A812	128K
.TRAP	_TEAutoView	$A813	128K
.TRAP	_Pack8	$A816	128K
.TRAP	_FixATan2	$A818	128K
.TRAP	_XMunger	$A819	128K
.TRAP	_Count1Types	$A81C	128K
.TRAP	_Get1Resource	$A81F	128K
.TRAP	_Get1NamedResource	$A820	128K
.TRAP	_MaxSizeRsrc	$A821	128K
.TRAP	_InsMenuItem	$A826	128K
.TRAP	_HideDItem	$A827	128K
.TRAP	_ShowDItem	$A828	128K
.TRAP	_Pack9	$A82B	128K
.TRAP	_Pack10	$A82C	128K
.TRAP	_Pack11	$A82D	128K
.TRAP	_Pack12	$A82E	128K
.TRAP	_Pack13	$A82F	128K
.TRAP	_Pack14	$A830	128K
.TRAP	_Pack15	$A831	128K
.TRAP	_ScrnBitMap	$A833	128K
.TRAP	_SetFScaleDisable	$A834	128K
.TRAP	_FontMetrics	$A835	128K
.TRAP	_ZoomWindow	$A83A	128K
.TRAP	_TrackBox	$A83B	128K
.TRAP	_InitFonts	$A8FE	64K
.TRAP	_GetFName	$A8FF	64K
.TRAP	_GetFNum	$A900	64K
.TRAP	_FMSwapFont	$A901	64K
.TRAP	_RealFont	$A902	64K
.TRAP	_SetFontLock	$A903	64K
.TRAP	_DrawGrowIcon	$A904	64K
.TRAP	_DragGrayRgn	$A905	64K
.TRAP	_NewString	$A906	64K
.TRAP	_SetString	$A907	64K
.TRAP	_ShowHide	$A908	64K
.TRAP	_CalcVis	$A909	64K
.TRAP	_CalcVBehind	$A90A	64K
.TRAP	_ClipAbove	$A90B	64K
.TRAP	_PaintOne	$A90C	64K
.TRAP	_PaintBehind	$A90D	64K
.TRAP	_SaveOld	$A90E	64K
.TRAP	_DrawNew	$A90F	64K
.TRAP	_GetWMgrPort	$A910	64K
.TRAP	_CheckUpDate	$A911	64K
.TRAP	_InitWindows	$A912	64K
.TRAP	_NewWindow	$A913	64K
.TRAP	_DisposWindow	$A914	64K
.TRAP	_ShowWindow	$A915	64K
.TRAP	_HideWindow	$A916	64K
.TRAP	_GetWRefCon	$A917	64K
.TRAP	_SetWRefCon	$A918	64K
.TRAP	_GetWTitle	$A919	64K
.TRAP	_SetWTitle	$A91A	64K
.TRAP	_MoveWindow	$A91B	64K
.TRAP	_HiliteWindow	$A91C	64K
.TRAP	_SizeWindow	$A91D	64K
.TRAP	_TrackGoAway	$A91E	64K
.TRAP	_SelectWindow	$A91F	64K
.TRAP	_BringToFront	$A920	64K
.TRAP	_SendBehind	$A921	64K
.TRAP	_BeginUpDate	$A922	64K
.TRAP	_EndUpDate	$A923	64K
.TRAP	_FrontWindow	$A924	64K
.TRAP	_DragWindow	$A925	64K
.TRAP	_DragTheRgn	$A926	64K
.TRAP	_InvalRgn	$A927	64K
.TRAP	_InvalRect	$A928	64K
.TRAP	_ValidRgn	$A929	64K
.TRAP	_ValidRect	$A92A	64K
.TRAP	_GrowWindow	$A92B	64K
.TRAP	_FindWindow	$A92C	64K
.TRAP	_CloseWindow	$A92D	64K
.TRAP	_SetWindowPic	$A92E	64K
.TRAP	_GetWindowPic	$A92F	64K
.TRAP	_InitMenus	$A930	64K
.TRAP	_NewMenu	$A931	64K
.TRAP	_DisposMenu	$A932	64K
.TRAP	_AppendMenu	$A933	64K
.TRAP	_ClearMenuBar	$A934	64K
.TRAP	_InsertMenu	$A935	64K
.TRAP	_DeleteMenu	$A936	64K
.TRAP	_DrawMenuBar	$A937	64K
.TRAP	_HiliteMenu	$A938	64K
.TRAP	_EnableItem	$A939	64K
.TRAP	_DisableItem	$A93A	64K
.TRAP	_GetMenuBar	$A93B	64K
.TRAP	_SetMenuBar	$A93C	64K
.TRAP	_MenuSelect	$A93D	64K
.TRAP	_MenuKey	$A93E	64K
.TRAP	_GetItmIcon	$A93F	64K
.TRAP	_SetItmIcon	$A940	64K
.TRAP	_GetItmStyle	$A941	64K
.TRAP	_SetItmStyle	$A942	64K
.TRAP	_GetItmMark	$A943	64K
.TRAP	_SetItmMark	$A944	64K
.TRAP	_CheckItem	$A945	64K
.TRAP	_GetItem	$A946	64K
.TRAP	_SetItem	$A947	64K
.TRAP	_CalcMenuSize	$A948	64K
.TRAP	_GetMHandle	$A949	64K
.TRAP	_SetMFlash	$A94A	64K
.TRAP	_PlotIcon	$A94B	64K
.TRAP	_FlashMenuBar	$A94C	64K
.TRAP	_AddResMenu	$A94D	64K
.TRAP	_PinRect	$A94E	64K
.TRAP	_DeltaPoint	$A94F	64K
.TRAP	_CountMItems	$A950	64K
.TRAP	_InsertResMenu	$A951	64K
.TRAP	_DelMenuItem	$A952	128K
.TRAP	_UpdtControl	$A953	128K
.TRAP	_NewControl	$A954	64K
.TRAP	_DisposControl	$A955	64K
.TRAP	_KillControls	$A956	64K
.TRAP	_ShowControl	$A957	64K
.TRAP	_HideControl	$A958	64K
.TRAP	_MoveControl	$A959	64K
.TRAP	_GetCRefCon	$A95A	64K
.TRAP	_SetCRefCon	$A95B	64K
.TRAP	_SizeControl	$A95C	64K
.TRAP	_HiliteControl	$A95D	64K
.TRAP	_GetCTitle	$A95E	64K
.TRAP	_SetCTitle	$A95F	64K
.TRAP	_GetCtlValue	$A960	64K
.TRAP	_GetMinCtl	$A961	64K
.TRAP	_GetMaxCtl	$A962	64K
.TRAP	_SetCtlValue	$A963	64K
.TRAP	_SetMinCtl	$A964	64K
.TRAP	_SetMaxCtl	$A965	64K
.TRAP	_TestControl	$A966	64K
.TRAP	_DragControl	$A967	64K
.TRAP	_TrackControl	$A968	64K
.TRAP	_DrawControls	$A969	64K
.TRAP	_GetCtlAction	$A96A	64K
.TRAP	_SetCtlAction	$A96B	64K
.TRAP	_FindControl	$A96C	64K
.TRAP	_Draw1Control	$A96D	128K
.TRAP	_DeQueue	$A96E	64K
.TRAP	_EnQueue	$A96F	64K
.TRAP	_GetNextEvent	$A970	64K
.TRAP	_EventAvail	$A971	64K
.TRAP	_GetMouse	$A972	64K
.TRAP	_StillDown	$A973	64K
.TRAP	_Button	$A974	64K
.TRAP	_TickCount	$A975	64K
.TRAP	_GetKeys	$A976	64K
.TRAP	_WaitMouseUp	$A977	64K
.TRAP	_UpdtDialog	$A978	128K
.TRAP	_CouldDialog	$A979	64K
.TRAP	_FreeDialog	$A97A	64K
.TRAP	_InitDialogs	$A97B	64K
.TRAP	_GetNewDialog	$A97C	64K
.TRAP	_NewDialog	$A97D	64K
.TRAP	_SelIText	$A97E	64K
.TRAP	_IsDialogEvent	$A97F	64K
.TRAP	_DialogSelect	$A980	64K
.TRAP	_DrawDialog	$A981	64K
.TRAP	_CloseDialog	$A982	64K
.TRAP	_DisposDialog	$A983	64K
.TRAP	_FindDItem	$A984	128K
.TRAP	_Alert	$A985	64K
.TRAP	_StopAlert	$A986	64K
.TRAP	_NoteAlert	$A987	64K
.TRAP	_CautionAlert	$A988	64K
.TRAP	_CouldAlert	$A989	64K
.TRAP	_FreeAlert	$A98A	64K
.TRAP	_ParamText	$A98B	64K
.TRAP	_ErrorSound	$A98C	64K
.TRAP	_GetDItem	$A98D	64K
.TRAP	_SetDItem	$A98E	64K
.TRAP	_SetIText	$A98F	64K
.TRAP	_GetIText	$A990	64K
.TRAP	_ModalDialog	$A991	64K
.TRAP	_DetachResource	$A992	64K
.TRAP	_SetResPurge	$A993	64K
.TRAP	_CurResFile	$A994	64K
.TRAP	_InitResources	$A995	64K
.TRAP	_RsrcZoneInit	$A996	64K
.TRAP	_OpenResFile	$A997	64K
.TRAP	_UseResFile	$A998	64K
.TRAP	_UpdateResFile	$A999	64K
.TRAP	_CloseResFile	$A99A	64K
.TRAP	_SetResLoad	$A99B	64K
.TRAP	_CountResources	$A99C	64K
.TRAP	_GetIndResource	$A99D	64K
.TRAP	_CountTypes	$A99E	64K
.TRAP	_GetIndType	$A99F	64K
.TRAP	_GetResource	$A9A0	64K
.TRAP	_GetNamedResource	$A9A1	64K
.TRAP	_LoadResource	$A9A2	64K
.TRAP	_ReleaseResource	$A9A3	64K
.TRAP	_HomeResFile	$A9A4	64K
.TRAP	_SizeRsrc	$A9A5	64K
.TRAP	_GetResAttrs	$A9A6	64K
.TRAP	_SetResAttrs	$A9A7	64K
.TRAP	_GetResInfo	$A9A8	64K
.TRAP	_SetResInfo	$A9A9	64K
.TRAP	_ChangedResource	$A9AA	64K
.TRAP	_AddResource	$A9AB	64K
.TRAP	_AddReference	$A9AC	128K
.TRAP	_RmveResource	$A9AD	64K
.TRAP	_RmveReference	$A9AE	128K
.TRAP	_ResError	$A9AF	64K
.TRAP	_WriteResource	$A9B0	64K
.TRAP	_CreateResFile	$A9B1	64K
.TRAP	_SystemEvent	$A9B2	64K
.TRAP	_SystemClick	$A9B3	64K
.TRAP	_SystemTask	$A9B4	64K
.TRAP	_SystemMenu	$A9B5	64K
.TRAP	_OpenDeskAcc	$A9B6	64K
.TRAP	_CloseDeskAcc	$A9B7	64K
.TRAP	_GetPattern	$A9B8	64K
.TRAP	_GetCursor	$A9B9	64K
.TRAP	_GetString	$A9BA	64K
.TRAP	_GetIcon	$A9BB	64K
.TRAP	_GetPicture	$A9BC	64K
.TRAP	_GetNewWindow	$A9BD	64K
.TRAP	_GetNewControl	$A9BE	64K
.TRAP	_GetRMenu	$A9BF	64K
.TRAP	_GetNewMBar	$A9C0	64K
.TRAP	_UniqueID	$A9C1	64K
.TRAP	_SysEdit	$A9C2	64K
.TRAP	_OpenRFPerm	$A9C4	128K
.TRAP	_RsrcMapEntry	$A9C5	128K
.TRAP	_Secs2Date	$A9C6	64K
.TRAP	_Date2Secs	$A9C7	64K
.TRAP	_SysBeep	$A9C8	64K
.TRAP	_SysError	$A9C9	64K
.TRAP	_TEGetText	$A9CB	64K
.TRAP	_TEInit	$A9CC	64K
.TRAP	_TEDispose	$A9CD	64K
.TRAP	_TextBox	$A9CE	64K
.TRAP	_TESetText	$A9CF	64K
.TRAP	_TECalText	$A9D0	64K
.TRAP	_TESetSelect	$A9D1	64K
.TRAP	_TENew	$A9D2	64K
.TRAP	_TEUpdate	$A9D3	64K
.TRAP	_TEClick	$A9D4	64K
.TRAP	_TECopy	$A9D5	64K
.TRAP	_TECut	$A9D6	64K
.TRAP	_TEDelete	$A9D7	64K
.TRAP	_TEActivate	$A9D8	64K
.TRAP	_TEDeactivate	$A9D9	64K
.TRAP	_TEIdle	$A9DA	64K
.TRAP	_TEPaste	$A9DB	64K
.TRAP	_TEKey	$A9DC	64K
.TRAP	_TEScroll	$A9DD	64K
.TRAP	_TEInsert	$A9DE	64K
.TRAP	_TESetJust	$A9DF	64K
.TRAP	_Munger	$A9E0	64K
.TRAP	_HandToHand	$A9E1	64K
.TRAP	_PtrToXHand	$A9E2	64K
.TRAP	_PtrToHand	$A9E3	64K
.TRAP	_HandAndHand	$A9E4	64K
.TRAP	_InitPack	$A9E5	64K
.TRAP	_InitAllPacks	$A9E6	64K
.TRAP	_Pack0	$A9E7	128K
.TRAP	_Pack1	$A9E8	128K
.TRAP	_Pack2	$A9E9	64K
.TRAP	_Pack3	$A9EA	64K
.TRAP	_FP68K	$A9EB	64K
.TRAP	_Pack4	$A9EB	64K
.TRAP	_Elems68K	$A9EC	64K
.TRAP	_Pack5	$A9EC	64K
.TRAP	_Pack6	$A9ED	64K
.TRAP	_DECSTR68K	$A9EE	128K
.TRAP	_Pack7	$A9EE	64K
.TRAP	_PtrAndHand	$A9EF	64K
.TRAP	_LoadSeg	$A9F0	64K
.TRAP	_UnLoadSeg	$A9F1	64K
.TRAP	_Launch	$A9F2	64K
.TRAP	_Chain	$A9F3	64K
.TRAP	_ExitToShell	$A9F4	64K
.TRAP	_GetAppParms	$A9F5	64K
.TRAP	_GetResFileAttrs	$A9F6	64K
.TRAP	_SetResFileAttrs	$A9F7	64K
.TRAP	_MethodDispatch	$A9F8	128K
.TRAP	_InfoScrap	$A9F9	64K
.TRAP	_UnlodeScrap	$A9FA	64K
.TRAP	_LodeScrap	$A9FB	64K
.TRAP	_ZeroScrap	$A9FC	64K
.TRAP	_GetScrap	$A9FD	64K
.TRAP	_PutScrap	$A9FE	64K
.TRAP	_Debugger	$A9FF	128K

; ---- Operating System: I/O and file system ----
.TRAP	_Open	$A000	64K
.TRAP	_Close	$A001	64K
.TRAP	_Read	$A002	64K
.TRAP	_Write	$A003	64K
.TRAP	_Control	$A004	64K
.TRAP	_Status	$A005	64K
.TRAP	_KillIO	$A006	64K
.TRAP	_GetVolInfo	$A007	64K
.TRAP	_Create	$A008	64K
.TRAP	_Delete	$A009	64K
.TRAP	_OpenRF	$A00A	64K
.TRAP	_ReName	$A00B	64K
.TRAP	_GetFileInfo	$A00C	64K
.TRAP	_SetFileInfo	$A00D	64K
.TRAP	_UnMountVol	$A00E	64K
.TRAP	_MountVol	$A00F	64K
.TRAP	_Allocate	$A010	64K
.TRAP	_GetEOF	$A011	64K
.TRAP	_SetEOF	$A012	64K
.TRAP	_FlushVol	$A013	64K
.TRAP	_GetVol	$A014	64K
.TRAP	_SetVol	$A015	64K
.TRAP	_FInitQueue	$A016	64K
.TRAP	_Eject	$A017	64K
.TRAP	_GetFPos	$A018	64K

; ---- Operating System: HFS (128K ROM) ----
.TRAP	_HSetVol	$A215	128K
.TRAP	_HGetVol	$A214	128K
.TRAP	_HOpen	$A200	128K
.TRAP	_HGetVInfo	$A207	128K
.TRAP	_HCreate	$A208	128K
.TRAP	_HDelete	$A209	128K
.TRAP	_HOpenRF	$A20A	128K
.TRAP	_HRename	$A20B	128K
.TRAP	_HGetFileInfo	$A20C	128K
.TRAP	_HSetFileInfo	$A20D	128K
.TRAP	_AllocContig	$A210	128K
.TRAP	_SetPEOF	$A212	128K
.TRAP	_HSetFLock	$A241	128K
.TRAP	_HRstFLock	$A242	128K

; ---- Operating System: memory manager ----
.TRAP	_InitZone	$A019	64K
.TRAP	_GetZone	$A11A	64K
.TRAP	_SetZone	$A01B	64K
.TRAP	_FreeMem	$A01C	64K
.TRAP	_MaxMem	$A11D	64K
.TRAP	_NewPtr	$A11E	64K
.TRAP	_DisposPtr	$A01F	64K
.TRAP	_SetPtrSize	$A020	64K
.TRAP	_GetPtrSize	$A021	64K
.TRAP	_NewHandle	$A122	64K
.TRAP	_DisposHandle	$A023	64K
.TRAP	_SetHandleSize	$A024	64K
.TRAP	_GetHandleSize	$A025	64K
.TRAP	_HandleZone	$A126	64K
.TRAP	_ReAllocHandle	$A027	64K
.TRAP	_RecoverHandle	$A128	64K
.TRAP	_HLock	$A029	64K
.TRAP	_HUnLock	$A02A	64K
.TRAP	_EmptyHandle	$A02B	64K
.TRAP	_InitApplZone	$A02C	64K
.TRAP	_SetApplLimit	$A02D	64K
.TRAP	_BlockMove	$A02E	64K

; ---- Operating System: event manager ----
.TRAP	_PostEvent	$A02F	64K
.TRAP	_PPostEvent	$A12F	128K
.TRAP	_OSEventAvail	$A030	64K
.TRAP	_GetOSEvent	$A031	64K
.TRAP	_FlushEvents	$A032	64K

; ---- Operating System: utilities ----
.TRAP	_VInstall	$A033	64K
.TRAP	_VRemove	$A034	64K
.TRAP	_OffLine	$A035	64K
.TRAP	_MoreMasters	$A036	64K
.TRAP	_WriteParam	$A038	64K
.TRAP	_ReadDateTime	$A039	64K
.TRAP	_SetDateTime	$A03A	64K
.TRAP	_Delay	$A03B	64K
.TRAP	_CmpString	$A03C	64K
.TRAP	_DrvrInstall	$A03D	64K
.TRAP	_DrvrRemove	$A03E	64K
.TRAP	_InitUtil	$A03F	64K
.TRAP	_ResrvMem	$A040	64K
.TRAP	_SetFilLock	$A041	64K
.TRAP	_RstFilLock	$A042	64K
.TRAP	_SetFilType	$A043	64K
.TRAP	_SetFPos	$A044	64K
.TRAP	_FlushFile	$A045	64K
.TRAP	_GetTrapAddress	$A146	64K
.TRAP	_SetTrapAddress	$A047	64K
.TRAP	_PtrZone	$A148	64K
.TRAP	_HPurge	$A049	64K
.TRAP	_HNoPurge	$A04A	64K
.TRAP	_SetGrowZone	$A04B	64K
.TRAP	_CompactMem	$A04C	64K
.TRAP	_PurgeMem	$A04D	64K
.TRAP	_AddDrive	$A04E	64K
.TRAP	_RDrvrInstall	$A04F	64K
.TRAP	_UprString	$A054	64K
.TRAP	_SetAppBase	$A057	64K

; ---- Operating System: 128K ROM additions ----
.TRAP	_RelString	$A050	128K
.TRAP	_ReadXPRam	$A051	128K
.TRAP	_WriteXPRam	$A052	128K
.TRAP	_InsTime	$A058	128K
.TRAP	_RmvTime	$A059	128K
.TRAP	_PrimeTime	$A05A	128K
.TRAP	_MaxBlock	$A061	128K
.TRAP	_PurgeSpace	$A162	128K
.TRAP	_MaxApplZone	$A063	128K
.TRAP	_MoveHHi	$A064	128K
.TRAP	_StackSpace	$A065	128K
.TRAP	_NewEmptyHandle	$A166	128K
.TRAP	_HSetRBit	$A067	128K
.TRAP	_HClrRBit	$A068	128K
.TRAP	_HGetState	$A069	128K
.TRAP	_HSetState	$A06A	128K
.TRAP	_InitFS	$A06C	128K
.TRAP	_InitEvents	$A06D	128K

; ---- HFS calls (128K ROM): parameter-block calls through _TFSCore ----
; _Name [pb][,mode]: pb is loaded into A0 when given; mode REGS, ASYNC, SYS or IMMED
; selects the trap variant; the routine number goes to D0.
	.MACRO	_TFSCore
	IF '%1' <> ''
	LEA	%1,A0
	ENDIF
	IF '%3' = ''
	DC.W	$A200+%2
	ENDIF
	IF '%3' = 'REGS'
	DC.W	$A300+%2
	ENDIF
	IF '%3' = 'ASYNC'
	DC.W	$A600+%2
	ENDIF
	IF '%3' = 'SYS'
	DC.W	$A600+%2
	ENDIF
	IF '%3' = 'IMMED'
	DC.W	$A200+%2
	ENDIF
	.ENDM

	.MACRO	_FSControl
	MOVEQ	#0,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_OpenWD
	MOVEQ	#1,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_CloseWD
	MOVEQ	#2,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_CatMove
	MOVEQ	#5,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_DirCreate
	MOVEQ	#6,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_GetWDInfo
	MOVEQ	#7,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_GetFCBInfo
	MOVEQ	#8,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_GetCatInfo
	MOVEQ	#9,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_SetCatInfo
	MOVEQ	#10,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_SetVolInfo
	MOVEQ	#11,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_SetPMSP
	MOVEQ	#12,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_LockRng
	MOVEQ	#16,D0
	_TFSCore	%1,96,%2
	.ENDM
	.MACRO	_UnlockRng
	MOVEQ	#17,D0
	_TFSCore	%1,96,%2
	.ENDM
