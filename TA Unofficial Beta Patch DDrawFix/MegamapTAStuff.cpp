#include "oddraw.h"

#include "hook/etc.h"
#include "hook/hook.h"
#include "tamem.h"
#include "tafunctions.h"
#include "PCX.H"
#include "ExternHotKey.h"

#include "megamaptastuff.h"
#include "fullscreenminimap.h"


#include "MegamapControl.h"
#include "PCX.H"

#include "hook/etc.h"
#include "hook/hook.h"
#include "tahook.h"
#include "gaf.h"
#include <math.h>
#include <vector>

#include "iddrawsurface.h"
using namespace std;

extern LPBYTE NewCopiedmmpb;
extern LPBYTE TAStuff_Surfc;
extern int mmpbWidth;
extern int mmpbHeight;
extern int lastPitch;


extern int width; 
extern int height;


#ifdef USEMEGAMAP
MegamapTAStuff::MegamapTAStuff (FullScreenMinimap * parent_p, RECT * MegaMapScreen_p, RECT * TAMap_p, RECT * GameScreen_p,
	int MaxIconWidth, int MaxIconHeight, BOOL UseSurfaceCursor_a)
{
	//IDDrawSurface::OutptTxt ( "MegamapTAStuff");
	ScreenBuf= NULL;
	for (int i= 0; i<0x15; ++i)
	{
		Cursor_Surfc[i]= NULL;
	}
	
	TAStuff_Surfc= NULL;
	Init ( parent_p, MegaMapScreen_p, TAMap_p, GameScreen_p, MaxIconWidth, MaxIconHeight, UseSurfaceCursor_a);

	//EnterOption2_hook = new InlineSingleHook(EnterOption_Address, 5, INLINE_5BYTESLAGGERJMP, EnterOption2);
}
MegamapTAStuff::~MegamapTAStuff()
{
	if (ScreenBuf)
	{
		delete ScreenBuf;
		ScreenBuf= NULL;
	}
}


void MegamapTAStuff::Init (FullScreenMinimap * parent_p, RECT * MegaMapScreen_p, RECT * TAMap_p, RECT * GameScreen_p,
	int MaxIconWidth, int MaxIconHeight, BOOL UseSurfaceCursor_a)
{

	
	parent= parent_p;
	TAmainStruct_Ptr= *TAmainStruct_PtrPtr;
	UseSurfaceCursor= UseSurfaceCursor_a;

	ReleaseSurface ( );
	CursorPerLine= 0;
	//OrderType= 
	memcpy ( &MegaMapScreen, MegaMapScreen_p, sizeof(RECT));
	MegaMapWidth= MegaMapScreen.right- MegaMapScreen.left;
	MegaMapHeight= MegaMapScreen.bottom- MegaMapScreen.top;



	memcpy ( &TAMap, TAMap_p, sizeof(RECT));
	TAMapWidth= TAMap.right- TAMap.left;
	TAMapHeight= TAMap.bottom- TAMap.top;

	Screen2MapWidthScale= static_cast<float>(MegaMapWidth)/ static_cast<float>(TAMapWidth);
	Screen2MapHeightScale= static_cast<float>(MegaMapHeight)/ static_cast<float>(TAMapHeight);

	memcpy ( &TAGameScreen, GameScreen_p, sizeof(RECT));


	GameDrawMegaArea.left= MegaMapScreen.left- TAGameScreen.left;
	GameDrawMegaArea.right= MegaMapWidth+ MegaMapScreen.left;
	GameDrawMegaArea.top= MegaMapScreen.top- TAGameScreen.top;
	GameDrawMegaArea.bottom= MegaMapScreen.top+ MegaMapHeight;

	if (ScreenBuf)
	{
		delete ScreenBuf;
		ScreenBuf= NULL;
	}
	ScreenBuf= new BYTE [((*TAmainStruct_PtrPtr)->ScreenWidth+ 3)* (*TAmainStruct_PtrPtr)->ScreenHeight];

	Position_Dword temp;

	ScreenPos2TAPos ( &temp, MaxIconWidth, MaxIconHeight);

	HalfMaxIconWidth_TAPos= temp.X/ 2;
	HalfMaxIconHeight_TAPos= temp.Y/ 2;

	width = MegaMapScreen.right - MegaMapScreen.left;
	height = MegaMapScreen.bottom - MegaMapScreen.top;
	
}

void MegamapTAStuff::LockBlit (LPVOID lpSurfaceMem, int dwWidth,int dwHeight, int lPitch)
{
	GUIBackup= FALSE;

	if (TAmainStruct_Ptr->desktopGUI.TheActive_GUIMEM
		&&TAmainStruct_Ptr->desktopGUI.TheActive_GUIMEM->ControlsAry)
	{
		unsigned int  OnCommand_SOUNDSRT_GUI= 0x0045DA90;
		unsigned int  OnCommand_MUSICRT_GUI= 0x0045D280;
		unsigned int  OnCommand_SPEEDSRT_GUI= 0x045EAD0;
		unsigned int  OnCommand_VISUALRT_GUI = 0x045E100;
		//unsigned int OnCommand_ARMOPT_GUI= 0x4609B0  ;
		GUIMEMSTRUCT * topGUImem= TAmainStruct_Ptr->desktopGUI.TheActive_GUIMEM;

		if (OnCommand_SOUNDSRT_GUI==topGUImem->OnCommand
			||OnCommand_MUSICRT_GUI==topGUImem->OnCommand
			||OnCommand_SPEEDSRT_GUI==topGUImem->OnCommand
			||OnCommand_VISUALRT_GUI==topGUImem->OnCommand
			)
			//||OnCommand_ARMOPT_GUI)
		{

			GUIBackup= TRUE;
			BackupWidth= topGUImem->ControlsAry->width;
			
			BackupXBegin= topGUImem->ControlsAry->xpos;
			BackupYBegin= topGUImem->ControlsAry->ypos;
			BackupYEnd= BackupYBegin+ topGUImem->ControlsAry->height;

			for (int i= BackupYBegin; i< BackupYEnd; ++i)
			{
				int Line= i* lPitch+ BackupXBegin;
				memcpy ( &ScreenBuf[Line], &((LPBYTE)lpSurfaceMem)[Line], BackupWidth);
			}
		}
	}
}

//int InTab = false;
//int InTab2 = false;

void MegamapTAStuff::LockBlit_TA (LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch, BOOL* Blit_b)
{
	OFFSCREEN OffScreen;
	memset ( &OffScreen, 0, sizeof(OFFSCREEN));
	OffScreen.Height= dwHeight;
	OffScreen.Width= dwWidth;
	OffScreen.lPitch= lPitch;
	OffScreen.lpSurface= lpSurfaceMem;

	OffScreen.ScreenRect.left= 0;
	OffScreen.ScreenRect.right=  dwWidth;

	OffScreen.ScreenRect.top= 0;
	OffScreen.ScreenRect.bottom= dwHeight;

	DrawPopupF4Dialog ( &OffScreen);
	memcpy ( &OffScreen.ScreenRect, &(*TAmainStruct_PtrPtr)->GameSreen_Rect, sizeof(RECT));
	
	DrawChatText ( &OffScreen);

	
	//if ((*TAmainStruct_PtrPtr)->IsGamePaused)
	//{//draw pause
	//	POINT Aspect;
	//	Aspect.x= lPitch;
	//	Aspect.y= dwHeight;

	//	CopyGafToBits ( (LPBYTE)lpSurfaceMem, &Aspect, 
	//		TAGameScreen.left+ (TAGameScreen.right- TAGameScreen.left)/ 2, TAGameScreen.top+ (TAGameScreen.bottom- TAGameScreen.top)/ 2, 
	//		(*TAmainStruct_PtrPtr)->igpaused->PtrFrameAry->PtrFrame);
	//}
	

	DrawPopupButtomDialog ( &OffScreen);

	if ((*TAmainStruct_PtrPtr)->SoftwareDebugMode & softwaredebugmode::Clock)
	{//draw clock
		int gametime = (*TAmainStruct_PtrPtr)->GameTime;
		char Textbuf[0x100];
		sprintf_s(
			Textbuf,
			"%s : %02d:%02d:%02d",
			"Game Time",
			gametime / 0x1A5E0,
			(gametime % 0x1A5E0) / 1800,
			(gametime % 0x1A5E0) % 1800 / 30);


		DrawTextInScreen ( &OffScreen, Textbuf, 130, (*TAProgramStruct_PtrPtr)->ScreenHeight - 34 - *((*TAProgramStruct_PtrPtr)->Font_Height), -1);
	}

	if (gameingstate::SKIRMISH & (*TAmainStruct_PtrPtr)->GameStateMask)
	{ //draw FRATE
		
		char Textbuf[0x100];
		int offY = 3 * (*((*TAProgramStruct_PtrPtr)->Font_Height)) - 10;
		sprintf_s ( Textbuf, "FRATE: %d\n", getFrate());
		DrawTextInScreen ( &OffScreen, Textbuf, 131, offY, -1);
	}

	if (gameingstate::MULTI & (*TAmainStruct_PtrPtr)->GameStateMask)
	{ //draw FRATE

		char Textbuf[0x100];
		int offY = 3 * (*((*TAProgramStruct_PtrPtr)->Font_Height)) - 10;
		sprintf_s(Textbuf, "FRATE: %d\n", getFrate());
		DrawTextInScreen(&OffScreen, Textbuf, 131, offY, -1);
	}

	//if (InTab == true && *Blit_b == true)
	//{
	////	*Blit_b = false;
	////	InTab = false;
	if (! GUIBackup)
	{

	////}
	 	OffScreen.ScreenRect.left = 0;
		OffScreen.ScreenRect.right = dwWidth - 1;

		OffScreen.ScreenRect.top = 0;
		OffScreen.ScreenRect.bottom = dwHeight - 1;

		DrawUnitUI(&((*TAmainStruct_PtrPtr)->desktopGUI), &OffScreen, &((*TAmainStruct_PtrPtr)->GameSreen_Rect));

	
		OffScreen.ScreenRect.left= 0;
		OffScreen.ScreenRect.right=  dwWidth- 1;

		OffScreen.ScreenRect.top= 0;
		OffScreen.ScreenRect.bottom= dwHeight- 1;
			
		DrawUnitUI(&((*TAmainStruct_PtrPtr)->desktopGUI), &OffScreen, &((*TAmainStruct_PtrPtr)->GameSreen_Rect));

//// 		BYTE KeepActiveBits[]= {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
//// 
//// 		SingleHook* KeepActive= new SingleHook ( KeepActiveAddr, sizeof(KeepActiveBits), INLINE_UNPROTECTEVINMENT, KeepActiveBits);
//// 		SingleHook* KeepActive1= new SingleHook ( KeepActiveAddr1, sizeof(KeepActiveBits), INLINE_UNPROTECTEVINMENT, KeepActiveBits);
//


//// 		if (KeepActive)
//// 		{
//// 			delete KeepActive;
//// 		}
//// 		if (KeepActive1)
//// 		{
//// 			delete KeepActive1;
//// 		}
//
	}
	else
	{
		for (int i = BackupYBegin; i < BackupYEnd; ++i)
		{
			int Line = i * lPitch + BackupXBegin;
			memcpy(&((LPBYTE)lpSurfaceMem)[Line], &ScreenBuf[Line], BackupWidth);
		}
	}
}



void MegamapTAStuff::LockBlit_MEGA(LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch)
{

	if (parent->Controler->IsDrawRect())
	{
		BlitSelect(lpSurfaceMem, dwWidth, dwHeight, lPitch);
	}

	if (parent->Controler->IsDrawOrder())
	{
		BlitOrder(lpSurfaceMem, dwWidth, dwHeight, lPitch);
	}
}

void MegamapTAStuff::UpdateTAGameStuffTA(LPDDSURFACEDESC realSurface, BOOL DrawTAStuff, BOOL DrawOrder, BOOL* Blit_b)
{
	if (DrawTAStuff)
	{

		//DDSURFACEDESC ddsd;
		//DDRAW_INIT_STRUCT(ddsd);

		//if (NULL != TAStuff_Surfc)
			//{
			//	//if (DD_OK != TAStuff_Surfc->IsLost())
			//	//{
			//	//	TAStuff_Surfc->Restore();
			//	//}
			//	DDBLTFX ddbltfx;
			//	DDRAW_INIT_STRUCT(ddbltfx);
			//	ddbltfx.dwFillColor = 9;


				// fill color with 0x09

				//memset(TAStuff_Surfc, 0x09, width * height);



				//if (TAStuff_Surfc->Blt(NULL, NULL, NULL, DDBLT_ASYNC | DDBLT_COLORFILL, &ddbltfx) != DD_OK)
				//{
				//	TAStuff_Surfc->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
				//}

				//if (DD_OK == TAStuff_Surfc->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL)   // When this sentence success, next sentence will not execute
					//|| DD_OK == TAStuff_Surfc->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))// 
				//{
		if (DrawTAStuff)
		{
			if (realSurface->dwWidth != realSurface->lPitch)
			{
				LockBlit_TA(realSurface, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, (*TAmainStruct_PtrPtr)->ScreenWidth, Blit_b);
			}
			else
			{
				LockBlit_TA(realSurface, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, (*TAmainStruct_PtrPtr)->ScreenWidth, Blit_b);
			}
		}
		//if (DrawOrder)
		//{
		//	LockBlit_MEGA(NewCopiedmmpb, mmpbWidth, mmpbHeight, mmpbWidth);
		//	//LockBlit_MEGA(NewCopiedmmpb, mmpbWidth, mmpbHeight, mmpbWidth);
		//}
	}

			//TAStuff_Surfc->Unlock(NULL);
		//}
}



void MegamapTAStuff::UpdateTAGameStuffMEGA(LPVOID realSurface, BOOL DrawTAStuff, BOOL DrawOrder, BOOL* Blit_b)
{
	if (DrawOrder)
	{

		//DDSURFACEDESC ddsd;
		//DDRAW_INIT_STRUCT(ddsd);

		//if (NULL != TAStuff_Surfc)
			//{
			//	//if (DD_OK != TAStuff_Surfc->IsLost())
			//	//{
			//	//	TAStuff_Surfc->Restore();
			//	//}
			//	DDBLTFX ddbltfx;
			//	DDRAW_INIT_STRUCT(ddbltfx);
			//	ddbltfx.dwFillColor = 9;


				// fill color with 0x09

				//memset(TAStuff_Surfc, 0x09, width * height);



				//if (TAStuff_Surfc->Blt(NULL, NULL, NULL, DDBLT_ASYNC | DDBLT_COLORFILL, &ddbltfx) != DD_OK)
				//{
				//	TAStuff_Surfc->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
				//}

				//if (DD_OK == TAStuff_Surfc->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL)   // When this sentence success, next sentence will not execute
					//|| DD_OK == TAStuff_Surfc->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))// 
				//{
		//if (DrawTAStuff)
		//{
		//	LockBlit_TA(realSurface, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, (*TAmainStruct_PtrPtr)->ScreenWidth, Blit_b);
		//}
		if (DrawOrder)
		{
			if ((*TAmainStruct_PtrPtr)->ScreenWidth != lastPitch)
			{
				LockBlit_MEGA(realSurface, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, (*TAmainStruct_PtrPtr)->ScreenWidth);
				//LockBlit_MEGA(realSurface, mmpbWidth, mmpbHeight, lastPitch); // unsure - test that 1366x768 resolution 
			}
			else
			{
				LockBlit_MEGA(realSurface, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, lastPitch);
				//LockBlit_MEGA(realSurface, mmpbWidth, mmpbHeight, lastPitch);
			}
			//LockBlit_MEGA(NewCopiedmmpb, mmpbWidth, mmpbHeight, mmpbWidth);
		}
	}

	//TAStuff_Surfc->Unlock(NULL);
//}
}

//void MegamapTAStuff::BlitTAGameStuff(LPBYTE DestSurf, LPRECT const gameScreen)
//{
//	if (NULL != TAStuff_Surfc)
//	{
//		//if (DD_OK != TAStuff_Surfc->IsLost())
//		//{
//		//	TAStuff_Surfc->Restore();
//		//}
//		//DDBLTFX ddbltfx;
//		//DDRAW_INIT_STRUCT(ddbltfx);
//		//	RECT GameScreen;
//
////#ifdef LINUS_X
////		ddbltfx.ddckSrcColorkey.dwColorSpaceLowValue = 0; // 9 windows - 0 linux
////		ddbltfx.ddckSrcColorkey.dwColorSpaceHighValue = 9; // 0 windows - 9 linux
////#else
////		ddbltfx.ddckSrcColorkey.dwColorSpaceLowValue = 9; // 9 windows - 0 linux
////		ddbltfx.ddckSrcColorkey.dwColorSpaceHighValue = 0; // 0 windows - 9 linux
////#endif
//
//
//		for (int y = 0; y < gameScreen->bottom - gameScreen->top; y++)
//		{
//			for (int x = 0; x < gameScreen->right - gameScreen->left; x++)
//			{
//				if (*((TAStuff_Surfc + y * (gameScreen->right - gameScreen->left)) + x) != 9)
//				{
//					*(DestSurf + (y * (gameScreen->right - gameScreen->left)) + x) = *(TAStuff_Surfc + (y * (gameScreen->right - gameScreen->left) + x));
//				}
//			}
//			
//		}
//
//
//
////  		GameScreen.left= 0;
////  		GameScreen.right= TAGameScreen.right;
////  		GameScreen.top= 0;
////  		GameScreen.bottom= TAGameScreen.bottom;
//
////		DestSurf->Blt ( NULL, TAStuff_Surfc, gameScreen, DDBLT_ASYNC | DDBLT_KEYSRCOVERRIDE, &ddbltfx);
//// 
//// 
//// 
// 		//if(DestSurf->Blt (NULL, TAStuff_Surfc, gameScreen, DDBLT_ASYNC| DDBLT_KEYSRCOVERRIDE  , &ddbltfx)!=DD_OK)
// 		//{
// 		//	DestSurf->Blt (NULL, TAStuff_Surfc, gameScreen, DDBLT_WAIT| DDBLT_KEYSRCOVERRIDE  , &ddbltfx);
// 		//}
//	}
//}

void MegamapTAStuff::InitSurface ( LPDIRECTDRAW TADD, BOOL VidMem)
{
	if (!TADD)
	{
		return;// invalid
	}
	ReleaseSurface ( );

	if (UseSurfaceCursor)
	{
			POINT GafSize={0, 0};
			int Width;
			int Height;

			CursorPerLine= (*TAProgramStruct_PtrPtr)->ScreenWidth/ MAXCURSORWIDTH;

			for (int i= cursorattack; i<0x15; ++i)
			{
				int FramesCnt= TAmainStruct_Ptr->cursor_ary[i]->Frames;

				Width= FramesCnt* MAXCURSORWIDTH;
				Height= MAXCURSORHEIGHT;

				if (CursorPerLine<FramesCnt)
				{
					Height= FramesCnt/ CursorPerLine* MAXCURSORHEIGHT;
					Width= CursorPerLine* MAXCURSORWIDTH;
				}

				DDSURFACEDESC ddsd;
				DDRAW_INIT_STRUCT(ddsd);
				ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

				if (VidMem)
				{
					ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
				}
				else
				{
					ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
				}
				ddsd.dwWidth = Width;
				ddsd.dwHeight = Height;

				TADD->CreateSurface ( &ddsd, &Cursor_Surfc[i], NULL);

				DDRAW_INIT_STRUCT(ddsd);

				Cursor_Surfc[i]->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
				GafSize.x= ddsd.lPitch;
				GafSize.y= ddsd.dwHeight;

				DDBLTFX ddbltfx;
				DDRAW_INIT_STRUCT(ddbltfx);
				ddbltfx.dwFillColor= TAmainStruct_Ptr->cursor_ary[i]->PtrFrameAry[0].PtrFrame->Background;
				if(Cursor_Surfc[i]->Blt ( NULL, NULL, NULL, DDBLT_ASYNC| DDBLT_COLORFILL, &ddbltfx)!=DD_OK)
				{
					Cursor_Surfc[i]->Blt ( NULL, NULL, NULL, DDBLT_WAIT| DDBLT_COLORFILL , &ddbltfx);
				}

				if (CursorPerLine<FramesCnt)
				{//

					int Lines= FramesCnt/ CursorPerLine;
					for (int j= 0; j<Lines; ++j)
					{
						int y= 0;
						int x= 0;
						y=  j* MAXCURSORHEIGHT;
						for (int t= 0; t<CursorPerLine; ++t)
						{
							x= t* MAXCURSORWIDTH;

							CopyGafToBits ( (LPBYTE)(ddsd.lpSurface), &GafSize, x, y, TAmainStruct_Ptr->cursor_ary[i]->PtrFrameAry[j* CursorPerLine+ t].PtrFrame);
						}
					}
				}
				else
				{
					for (int j= 0; j<FramesCnt; ++j)
					{
						CopyGafToBits ( (LPBYTE)(ddsd.lpSurface), &GafSize, j* MAXCURSORWIDTH, 0, TAmainStruct_Ptr->cursor_ary[i]->PtrFrameAry[j].PtrFrame);
					}
				}

				Cursor_Surfc[i]->Unlock(NULL);
			}
	}

	
	DDSURFACEDESC ddsd;
	DDRAW_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	if (VidMem)
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	}
	else
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	}
	ddsd.dwWidth = TAGameScreen.right;
	ddsd.dwHeight = TAGameScreen.bottom;

	TAStuff_Surfc = (LPBYTE)malloc(MegaMapScreen.right * MegaMapScreen.bottom);


	//TADD->CreateSurface ( &ddsd, &TAStuff_Surfc, NULL);
}

void MegamapTAStuff::ReleaseSurface (void) 
{
	if (UseSurfaceCursor)
	{
		for (int i= cursorattack; i< 0x15; ++i)
		{
			if (NULL!=Cursor_Surfc[i])
			{
				Cursor_Surfc[i]->Release ( );
				Cursor_Surfc[i]= NULL;
			}
		}

	}
	if (NULL!=TAStuff_Surfc)
	{
		free(TAStuff_Surfc);
		TAStuff_Surfc= NULL;
	}
}

void MegamapTAStuff::DrawCursor(LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch
	, unsigned int X, unsigned int Y)
{
	//if (UseSurfaceCursor)
	{
	//	return ;
	}
	GAFFrame * Cursor_GafP= (*TAProgramStruct_PtrPtr)->Cursor;

	X= X;//+  Cursor_GafP->xPosition; //(*TAProgramStruct_PtrPtr)->CursorX+ Cursor_GafP->xPosition;//= X;
	Y= Y;//+ Cursor_GafP->yPosition;  //(*TAProgramStruct_PtrPtr)->CursorY+ Cursor_GafP->yPosition;//= Y;

	POINT Aspect;
	Aspect.x= lPitch;
	Aspect.y= dwHeight;

	CopyGafToBits ( (LPBYTE)lpSurfaceMem, &Aspect, X, Y, Cursor_GafP);
}
//
//void MegamapTAStuff::DrawCursor (LPBYTE DestSurf, unsigned int X, unsigned int Y)
//{
//	if (! UseSurfaceCursor)
//	{
//		return ;
//	}
//	GAFFrame * Cursor_GafP= (*TAProgramStruct_PtrPtr)->Cursor;
//	X= X;//= X;
//	Y= Y;//= Y;
//	BOOL Update_B= FALSE;;
//
//
//	GAFSequence * Seq_p= TAmainStruct_Ptr->cursor_ary[TAmainStruct_Ptr->CurrentCursora_Index];
//
//	LPDIRECTDRAWSURFACE SrcSurf= Cursor_Surfc[TAmainStruct_Ptr->CurrentCursora_Index];
//	HRESULT LostResult;
//	if (SrcSurf)
//	{
//		if (DD_OK!=(LostResult= SrcSurf->IsLost ( )))
//		{
//			SrcSurf->Restore ( );
//			
//
//			DDSURFACEDESC ddsd;
//			DDRAW_INIT_STRUCT(ddsd);
//			POINT Aspect;
//
//			DDBLTFX ddbltfx;
//			DDRAW_INIT_STRUCT(ddbltfx);
//			ddbltfx.dwFillColor= Cursor_GafP->Background;
//
//			if(SrcSurf->Blt ( NULL, NULL, NULL, DDBLT_ASYNC| DDBLT_COLORFILL, &ddbltfx)!=DD_OK)
//			{
//				SrcSurf->Blt ( NULL, NULL, NULL, DDBLT_WAIT| DDBLT_COLORFILL , &ddbltfx);
//			}
//
//			if (DD_OK==SrcSurf->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))
//			{
//				Aspect.x= ddsd.lPitch;
//				Aspect.y= ddsd.dwHeight;
//
//				
//				int FramesCnt= Seq_p->Frames;
//				
//			
//				if ((CursorPerLine)<(FramesCnt))
//				{//
//					int i= 0;
//					do 
//					{
//						i= FramesCnt/ CursorPerLine;
//						for (int j= 0; j<CursorPerLine; ++j)
//						{
//							CopyGafToBits ( (LPBYTE)(ddsd.lpSurface), &Aspect, j* MAXCURSORWIDTH,  i* MAXCURSORHEIGHT, Seq_p->PtrFrameAry[j].PtrFrame);
//						}
//						FramesCnt-= CursorPerLine;
//					} while (0<FramesCnt);
//				}
//				else
//				{
//					for (int j= 0; j<FramesCnt; ++j)
//					{
//						CopyGafToBits ( (LPBYTE)(ddsd.lpSurface), &Aspect, j* MAXCURSORWIDTH, 0, Seq_p->PtrFrameAry[j].PtrFrame);
//					}
//				}
//				SrcSurf->Unlock ( NULL);
//			}
//		}
//		DDBLTFX ddbltfx;
//		DDRAW_INIT_STRUCT(ddbltfx);
//		RECT Dest;
//		RECT Src;
//
//
//		Dest.left = X- Cursor_GafP->xPosition;
//		Dest.top = Y- Cursor_GafP->yPosition;
//		Dest.right = Dest.left+ Cursor_GafP->Width;
//		Dest.bottom = Dest.top + Cursor_GafP->Height;
//
//		Src.left= MAXCURSORWIDTH* TAmainStruct_Ptr->CurrentCursora_Index;
//		Src.top= 0;
//		
//		for (int j= 0; j<Seq_p->Frames; ++j)
//		{
//			if (Cursor_GafP== Seq_p->PtrFrameAry[j].PtrFrame)
//			{
//				if (j<CursorPerLine)
//				{
//					Src.left= MAXCURSORWIDTH* j;
//					Src.top= 0;
//				}
//				else
//				{
//					Src.left= (j)% CursorPerLine* MAXCURSORWIDTH;
//					Src.top= j/ CursorPerLine* MAXCURSORHEIGHT;
//				}
//			}
//		}
//
//		Src.right= Src.left+ Cursor_GafP->Width;
//		Src.bottom= Src.top+ Cursor_GafP->Height;
//
//		ddbltfx.ddckSrcColorkey.dwColorSpaceLowValue= Cursor_GafP->Background;
//		ddbltfx.ddckSrcColorkey.dwColorSpaceHighValue= 0;
//
//		if(DestSurf->Blt(&Dest, SrcSurf, &Src, DDBLT_ASYNC| DDBLT_KEYSRCOVERRIDE, &ddbltfx)!=DD_OK)
//		{
//			DestSurf->Blt(&Dest, SrcSurf, &Src, DDBLT_WAIT| DDBLT_KEYSRCOVERRIDE, &ddbltfx);
//		}
//	}
//}

void MegamapTAStuff::TADrawRect (OFFSCREEN * offscreen, int TAx1, int TAy1, int TAz1, 
	int TAx2, int TAy2, int TAz2, 
	int color)
{
	POINT Pos;
	TAPos2ScreenPos (  &Pos, TAx1, TAy1, TAz1);

	RECT Rect;
	Rect.left= Pos.x;
	Rect.top= Pos.y;

	if (Rect.left<0)
	{
		Rect.left= 0;
	}
	if (Rect.top<0)
	{
		Rect.top= 0;
	}
	

	TAPos2ScreenPos ( &Pos, TAx2, TAy2, TAz2);

	Rect.right= Pos.x;
	Rect.bottom= Pos.y;
	if ((MegaMapScreen.right- 1)<Rect.right)
	{
		Rect.left= (MegaMapScreen.right- 1)- Rect.right+ Rect.left;
		Rect.right= (MegaMapScreen.right- 1);
		
	}
	if ((MegaMapScreen.bottom- 1)<Rect.bottom)
	{
		Rect.top= (MegaMapScreen.bottom- 1)- Rect.bottom+ Rect.top;
		Rect.bottom= (MegaMapScreen.bottom- 1);
	}

	Rect.left+= MegaMapScreen.left;//- TAGameScreen.left;
	Rect.top+= MegaMapScreen.top;//- TAGameScreen.top;
	Rect.right+= MegaMapScreen.left;//- TAGameScreen.left;
	Rect.bottom+= MegaMapScreen.top;//- TAGameScreen.top;

	::TADrawRect ( offscreen, &Rect, color);
}
void MegamapTAStuff::DrawBuildRect (OFFSCREEN * offscren_p, unsigned char  Color, 
	UnitDefStruct * BuildTargat, int TAx, int TAy, int TAz)
{// draw in TA screen
	POINT Pos;
	TAPos2ScreenPos (  &Pos, TAx, TAy, TAz);
	RECT Rect;
	Rect.left= Pos.x;
	Rect.top= Pos.y;

	TAPos2ScreenPos (  &Pos, BuildTargat->FootX* 16, BuildTargat->FootY* 16, 0);
	Rect.left-= Pos.x/ 2;
	Rect.top-= Pos.y/ 2;

	if (Rect.left<0)
	{
		Rect.left= 0;
	}
	if (Rect.top<0)
	{
		Rect.top= 0;
	}


	
	Rect.left+= MegaMapScreen.left;//- TAGameScreen.left;
	Rect.top+= MegaMapScreen.top;//- TAGameScreen.top;

	Rect.right= Rect.left+ Pos.x;
	Rect.bottom= Rect.top+ Pos.y;


	if (MegaMapScreen.right<Rect.right)
	{
		Rect.right= MegaMapScreen.right;
		Rect.left= Rect.right- Pos.x;
	}

	if (MegaMapScreen.bottom<Rect.bottom)
	{
		Rect.bottom= MegaMapScreen.bottom;
		Rect.top= Rect.bottom- Pos.y;
	}

	::TADrawRect ( offscren_p, &Rect, Color);
}

void MegamapTAStuff::DrawTargatOrder (OFFSCREEN * OffScreen, UnitOrdersStruct * Order, PlayerStruct * me)
{
	POINT Pos;

	if (Order->AttackTargat)
	{
		do 
		{
			if (! CheckUnitInPlayerLOS ( me, Order->AttackTargat))
			{
				if (0x200000&Order->Order_State)
				{
					break;
				}
				Order->Order_State|= 0x200000;											
			}
			Order->RemeberX= Order->AttackTargat->XPos;
			Order->RemeberY= Order->AttackTargat->YPos- Order->AttackTargat->ZPos/ 2;

		} while (false);
		TAPos2ScreenPos ( &Pos, Order->RemeberX, Order->RemeberY, 0);
	}
	else
	{
		TAPos2ScreenPos ( &Pos, Order->Pos.X, Order->Pos.Y, Order->Pos.Z);
	}

	// 							RECT MouseRect;
	// 							MouseRect.top= Pos.y+ MegaMapScreen.top;
	// 							MouseRect.left= Pos.x+ MegaMapScreen.left;
	Pos.x+= MegaMapScreen.left;//- TAGameScreen.left;
	Pos.y+= MegaMapScreen.top;//- TAGameScreen.top;
	int Index= (*COBSciptHandler_Begin)[Order->COBHandler_index].cursorIndex;
	if ((cursorattack<=Index)
		&&(Index<=cursorhourglass))
	{
		PGAFSequence GafSeq_p= TAmainStruct_Ptr->cursor_ary[Index];

		PGAFFrame Gaf_p= GafSeq_p->PtrFrameAry[(TAmainStruct_Ptr->GameTime/ (GafSeq_p->PtrFrameAry->Animated* 2))% GafSeq_p->Frames].PtrFrame;

		if (NULL!=Gaf_p)
		{
		//	GafFrame2OFFScreen ( OffScreen, Gaf_p, 
		//		Pos.x, Pos.y);
			CopyGafToContext ( OffScreen, Gaf_p, 
				Pos.x, Pos.y);
			
		}
	}

}

void MegamapTAStuff::DrawOrderPath (OFFSCREEN * OffScreen, UnitOrdersStruct * Order, Position_Dword * UnitPos)
{
	//Order->Pos;
	double x;
	double y; 
	int X1;
	int X2;
	int Y1;
	int Y2;

	if (Order->AttackTargat)
	{
		X1= Order->RemeberX;
		X2= UnitPos->X;
		Y1= Order->RemeberY;
		Y2=  UnitPos->Y- UnitPos->Z/ 2;
	}
	else
	{
		X1= Order->Pos.X;
		X2= UnitPos->X;
		Y1=  Order->Pos.Y- Order->Pos.Z/ 2;
		Y2=  UnitPos->Y- UnitPos->Z/ 2;
	}



	if (X1<TAMap.left)
	{
		X1= TAMap.left;
	}

	if (X2<TAMap.left)
	{
		X2= TAMap.left;
	}
	if (TAMap.right<X1)
	{
		X1= TAMap.right;
	}

	if (TAMap.right<X2)
	{
		X2= TAMap.right;
	}



	if (Y1<TAMap.top)
	{
		Y1= TAMap.top;
	}
	if (Y2<TAMap.top)
	{
		Y2= TAMap.top;
	}

	if (TAMap.bottom<Y1)
	{
		Y1= TAMap.bottom;
	}

	if (TAMap.bottom<Y2)
	{
		Y2= TAMap.bottom;
	}


	x= X1- X2;
	y= Y1- Y2;
	double Sqrt= sqrt (x* x+ y* y);
	Position_Dword Pos;
	ScreenPos2TAPos ( &Pos, ORDERPATHSPACING, ORDERPATHSPACING);
	double SpacingSqrt= sqrt( (double)(Pos.X* Pos.X)+ (double)(Pos.Y* Pos.Y));

	if (SpacingSqrt<Sqrt)
	{//
		int TimeDelay= TAmainStruct_Ptr->GameTime- Order->StartTime;
		PGAFSequence path= TAmainStruct_Ptr->pathicon;

		if (TimeDelay<0)
		{
			TimeDelay= 0;
		}
		int FrameIndex= (TimeDelay/ path->PtrFrameAry->Animated)% path->Frames;

		
		double Curt= (SpacingSqrt* (TimeDelay% ORDERPATHSPACING))/ ORDERPATHSPACING;
		
		double Ratio;
		POINT ScreenPos;
		while (Curt<Sqrt)
		{
			Ratio= Curt/ Sqrt;

			TAPos2ScreenPos ( &ScreenPos, UnitPos->X+ static_cast<int>(x* Ratio), UnitPos->Y+ static_cast<int>(y* Ratio)- UnitPos->Z/ 2, 0);
// 			GafFrame2OFFScreen ( OffScreen, path->PtrFrameAry[FrameIndex].PtrFrame, 
// 				ScreenPos.x+ MegaMapScreen.left/*- TAGameScreen.left*/, ScreenPos.y+ MegaMapScreen.top/*- TAGameScreen.top*/);
			CopyGafToContext ( OffScreen, path->PtrFrameAry[FrameIndex].PtrFrame, 
				ScreenPos.x+ MegaMapScreen.left, ScreenPos.y+ MegaMapScreen.top);
			Curt+= SpacingSqrt;

			FrameIndex= (++FrameIndex)% path->Frames;
		}
	}
}


void MegamapTAStuff::BlitOrder (LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch)
{
	//IDDrawSurface::OutptTxt ( "BlitOrder");
	int ColorIndex;
	OFFSCREEN OffScreen;
	memset ( &OffScreen, 0, sizeof(OFFSCREEN));
	OffScreen.Height= dwHeight;
	OffScreen.Width = dwWidth;
	OffScreen.lPitch= lPitch;
	OffScreen.lpSurface= lpSurfaceMem;

	//RECT newRect;
	//newRect.left = 0;
	//newRect.top = 0;
	//newRect.right = dwWidth;
	//newRect.bottom = dwHeight;

	//memcpy ( &OffScreen.ScreenRect, &newRect, sizeof(RECT));

	OffScreen.ScreenRect.left= 0;
	OffScreen.ScreenRect.right=  dwWidth;

	OffScreen.ScreenRect.top= 0;
	OffScreen.ScreenRect.bottom= dwHeight;



	BOOL Draw= FALSE, OtherBuilder= FALSE;
	PlayerStruct * me= &TAmainStruct_Ptr->Players[TAmainStruct_Ptr->LocalHumanPlayer_PlayerID];
	UnitStruct * Begin= me->Units;
	UnitStruct * End= me->UnitsAry_End;
	unsigned int MouseID= TAmainStruct_Ptr->MouseOverUnit;
	unsigned int ShowRange= TAmainStruct_Ptr->ShowRangeUnitIndex;
	UnitStruct *Camera= TAmainStruct_Ptr->CameraToUnit;
	POINT Aspect;
	


	if (Camera)
	{
		if (IsPlayerAllyUnit ( Camera->UnitInGameIndex, TAmainStruct_Ptr->LocalHumanPlayer_PlayerID))
		{
			if ( Camera->UnitType->CANBUILD_ptr)
			{
				OtherBuilder= TRUE;
			}	
		}
					
	}
	if (ShowRange)
	{
		if (IsPlayerAllyUnit ( ShowRange, TAmainStruct_Ptr->LocalHumanPlayer_PlayerID))
		{
			if ( TAmainStruct_Ptr->BeginUnitsArray_p [ShowRange].UnitType->CANBUILD_ptr)
			{
				OtherBuilder= TRUE;
			}
		}

	}
	if (MouseID)
	{
		if (IsPlayerAllyUnit ( MouseID, TAmainStruct_Ptr->LocalHumanPlayer_PlayerID))
		{
			if (TAmainStruct_Ptr->BeginUnitsArray_p [MouseID].UnitType->CANBUILD_ptr)
			{
				OtherBuilder= TRUE;
			}
		}
	}

	vector<Position_Dword> DrawedTargat;
	Position_Dword TargatPos;
	Position_Dword UnitPos;
	memset ( &TargatPos, 0, sizeof(Position_Dword));
	memset ( &UnitPos, 0, sizeof(Position_Dword));
	for (; Begin!=End; Begin= &Begin[1])
	{
		if ((0x10000000&Begin->UnitSelected)
			&&(0==(0x4000&Begin->UnitSelected))
			)

		{	
			Draw= FALSE;
			if ((Begin->UnitInGameIndex==MouseID)
				||(Begin==Camera)
				||(Begin->UnitInGameIndex==ShowRange)
				)
			{//
				Draw= TRUE;
			}	

			//
			if (Draw
				||(UnitSelected_State& Begin->UnitSelected)
				||OtherBuilder)
			{			
				if (Begin->UnitOrders)
				{
					UnitOrdersStruct *Order= Begin->UnitOrders;
	 
					UnitPos.X= Begin->XPos;
					UnitPos.Y= Begin->YPos;
					UnitPos.Z= Begin->ZPos;
					do 
					{
						DWORD OrderMask= (*COBSciptHandler_Begin)[Order->COBHandler_index].COBScripMask;

						if (OrderMask& 1)
						{// build
							if (UnitSelected_State& Begin->UnitSelected)
							{
								ColorIndex= TAmainStruct_Ptr->desktopGUI.RadarObjecColor[10];
							}
							else
							{
								ColorIndex= TAmainStruct_Ptr->desktopGUI.RadarObjecColor[1];
							}
							DrawBuildRect ( &OffScreen, ColorIndex, 
								&(TAmainStruct_Ptr->UnitDef[Order->BuildUnitID]), Order->Pos.X, Order->Pos.Y, Order->Pos.Z);

							if (Draw)
							{
								DrawOrderPath ( &OffScreen, Order, &UnitPos);
							}

							UnitPos.X= Order->Pos.X;
							UnitPos.Y= Order->Pos.Y;
							UnitPos.Z= Order->Pos.Z;
						}

						if (OrderMask& 2)
						{
							if (Order->AttackTargat)
							{
								TargatPos.X= Order->RemeberX;
								TargatPos.Y= Order->RemeberY;
								TargatPos.Z= 0;
							}
							else
							{
								TargatPos.X= Order->Pos.X;
								TargatPos.Y= Order->Pos.Y;
								TargatPos.Z= Order->Pos.Z;
							}

							if (Draw
								||(UnitSelected_State& Begin->UnitSelected))
							{
			
								BOOL Drawed= FALSE;

								for (vector<Position_Dword>::iterator P= DrawedTargat.begin ( ); P!= DrawedTargat.end ( ); ++P)
								{
									if ((P->X==TargatPos.X)
										&&(P->Y==TargatPos.Y)
										&&(P->Z==TargatPos.Z))
									{
										Drawed= TRUE;
										break;
									}
								}

								if (! Drawed)
								{
									DrawTargatOrder ( &OffScreen, Order, me);
									DrawedTargat.push_back ( TargatPos);
								}

								
								if (Draw)
								{
									DrawOrderPath ( &OffScreen, Order, &UnitPos);
								}
							}
							UnitPos.X= TargatPos.X;
							UnitPos.Y= TargatPos.Y;
							UnitPos.Z= TargatPos.Z;
						}

						if (OrderMask& 8)
						{// cursor

							if (Order->AttackTargat)
							{
								TargatPos.X= Order->RemeberX;
								TargatPos.Y= Order->RemeberY;
								TargatPos.Z= 0;
							}
							else
							{
								TargatPos.X= Order->Pos.X;
								TargatPos.Y= Order->Pos.Y;
								TargatPos.Z= Order->Pos.Z;
							}

							if (Draw
								||(UnitSelected_State& Begin->UnitSelected))
							{
							
								BOOL Drawed= FALSE;

								for (vector<Position_Dword>::iterator P= DrawedTargat.begin ( ); P!= DrawedTargat.end ( ); ++P)
								{
									if (P->X==TargatPos.X
										&&P->Y==TargatPos.Y
										&&P->Z==TargatPos.Z)
									{
										Drawed= TRUE;
										break;
									}
								}
								if (! Drawed)
								{
									DrawTargatOrder ( &OffScreen, Order, me);
									DrawedTargat.push_back ( TargatPos);
								}
							}
							
							UnitPos.X= TargatPos.X;
							UnitPos.Y= TargatPos.Y;
							UnitPos.Z= TargatPos.Z;
	
						}

						if (OrderMask& 0x10)
						{
							if (Draw)
							{
								if (4&Begin->cIsCloaked)
								{
									DWORD Radius= static_cast<int>(static_cast<float>(Begin->UnitType->mincloakdistance)* Screen2MapWidthScale);
									int TAx;
									int TAy;

									POINT Pos;

									TAPos2ScreenPos ( &Pos, Begin->XPos, Begin->YPos, Begin->ZPos);




									TAx= Pos.x+ MegaMapScreen.left;//- TAGameScreen.left;
									TAy= Pos.y+ MegaMapScreen.top;//- TAGameScreen.top;

									Aspect.x= dwWidth;
									Aspect.y= dwHeight;

									DrawRadarCircle ( (LPBYTE)lpSurfaceMem, &Aspect,
										TAx, TAy, 
										Radius, 
										(*TAmainStruct_PtrPtr)->desktopGUI.RadarObjecColor[0xf] );
								}
							}
						}

					} while (Order= Order->NextOrder);
				}
			}
		}
	}
}


void MegamapTAStuff::BlitSelect (LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch)
{
	if (parent->Controler->IsDrawRect ())
	{
		int ColorIndex;
		OFFSCREEN OffScreen;
		memset ( &OffScreen, 0, sizeof(OFFSCREEN));
		OffScreen.Height = dwHeight;
		OffScreen.Width= dwWidth;
		OffScreen.lPitch= lPitch;
		OffScreen.lpSurface= lpSurfaceMem;

		//RECT newRect;
		//newRect.left = 0;
		//newRect.top = 0;
		//newRect.right = dwWidth;
		//newRect.bottom = dwHeight;

		//memcpy(&OffScreen.ScreenRect, &newRect, sizeof(RECT));

		memcpy ( &OffScreen.ScreenRect, &GameDrawMegaArea, sizeof(RECT));
/*
		OffScreen.ScreenRect.left= 0;
		OffScreen.ScreenRect.right=  dwWidth;;

		OffScreen.ScreenRect.top= 0;
		OffScreen.ScreenRect.bottom= dwHeight;*/

		RECT SelectInScreen;
		RECT SelectScreenRect;
		if (selectbuttom::select==parent->Controler->ReadSelectState ())
		{// draw
			parent->Controler->ReadSelectRect ( &SelectScreenRect);
			SelectInScreen.left= SelectScreenRect.left+ MegaMapScreen.left;//- TAGameScreen.left;
			SelectInScreen.right= SelectScreenRect.right+ MegaMapScreen.left;//- TAGameScreen.left;
			SelectInScreen.top= SelectScreenRect.top+ MegaMapScreen.top;//- TAGameScreen.top;
			SelectInScreen.bottom= SelectScreenRect.bottom+ MegaMapScreen.top;//- TAGameScreen.top;

			ColorIndex= 0xf;

			::TADrawRect ( &OffScreen, &SelectInScreen, TAmainStruct_Ptr->desktopGUI.RadarObjecColor[ColorIndex]);
		}
		else if (ordertype::BUILD==TAmainStruct_Ptr->PrepareOrder_Type)
		{// draw build rect
			if (((CTAHook*)LocalShare->TAHook)->IsLineBuilding())
			{
				
				((CTAHook*)LocalShare->TAHook)->VisualizeRow_ForME_megamap ( &OffScreen);
	
			}
			else
			{
				if (! parent->Controler->IsInMap ( ))
				{
					return ;
				}

				ColorIndex = ((TAmainStruct_Ptr->BuildSpotState) & 0x40 )!= 0 ? 10 : 4;
				DrawBuildRect ( &OffScreen, TAmainStruct_Ptr->desktopGUI.RadarObjecColor[ColorIndex], &(TAmainStruct_Ptr->UnitDef[TAmainStruct_Ptr->BuildUnitID]), TAmainStruct_Ptr->MouseMapPos.X, TAmainStruct_Ptr->MouseMapPos.Y, TAmainStruct_Ptr->MouseMapPos.Z);
			}
		}
	}
}



POINT * MegamapTAStuff::TAPos2ScreenPos (POINT * ScreenPos, unsigned int TAX, unsigned int TAY, unsigned int TAZ)
{
	if (NULL==ScreenPos)
	{
		return NULL;
	}
	int TAx= TAX; 
	int TAy= TAY- TAZ/ 2;


	ScreenPos->x= static_cast<int>(static_cast<float>(TAx)* Screen2MapWidthScale);
	ScreenPos->y= static_cast<int>(static_cast<float>(TAy)* Screen2MapHeightScale);

	return ScreenPos;
}

Position_Dword * MegamapTAStuff::ScreenPos2TAPos (Position_Dword * TAPos, int x, int y, BOOL UseTAHeight)
{
	if (NULL==TAPos)
	{
		return NULL;
	}
	TAPos->X= static_cast<int>(static_cast<float>(x)/ Screen2MapWidthScale);
	TAPos->Y= static_cast<int>(static_cast<float>(y)/ Screen2MapHeightScale);
	if (UseTAHeight&&
		TAmainStruct_Ptr->Features)
	{
		TAPos->Z= GetPosHeight ( TAPos);
	}
	else
	{
		TAPos->Z= TAmainStruct_Ptr->SeaLevel;
	}


	//GetPosition_Dword ( TAPos->X, TAPos->Y, TAPos);
	return TAPos;
}

LPBYTE MegamapTAStuff::GetTAStuffSurface()
{
	return TAStuff_Surfc;
}


#endif