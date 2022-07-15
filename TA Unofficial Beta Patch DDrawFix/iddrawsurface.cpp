//---------------------------------------------------------------------------
#pragma hdrstop

#include "oddraw.h"
#include <vector>
using namespace std;

#include "tamem.h"
#include "tafunctions.h"

#include "whiteboard.h"
#include "MinimapHandler.h"
#include "dddta.h"
#include "cincome.h"
#include "dialog.h"
#include "tahook.h"
#include "commanderwarp.h"
#include "maprect.h"

#include "unitrotate.h"
#include "changequeue.h"
#include "ExternHotKey.h"
#include "TAbugfix.h"
#include "fullscreenminimap.h"
#include "GUIExpand.h"
#include "gaf.h"

#include "iddrawsurface.h"


#include <stdio.h>
#include "font.h"
#include <time.h>
//#include <conio.h>
#include "pcx.h"
#include "hook/etc.h"
#include "hook/hook.h"
#include "UnicodeSupport.h"

#include "LimitCrack.h"
#include "TAConfig.h"

//---------------------------------------------------------------------------
//#pragma package(smart_init)

extern HINSTANCE HInstance;
short MouseX, MouseY;
bool StartedInRect;


CIncome* IncomeStructureShare;
LPRGNDATA ScreenRegionShare;
LPDIRECTDRAWCLIPPER lpDDClipperShare;




LPDDSURFACEDESC lpFrontDescGlobal;
LPVOID lpFrontSurfaceGlobal;
int lastPitch = -1;



extern bool megamapON;


extern int LAYER;



//#define XPOYDEBUG

/*
IDDrawSurface::~IDDrawSurface()
{
	delete WhiteBoard;
	delete Income;
	delete TAHook;
	delete CommanderWarp;
	delete SharedRect;
	delete SettingsDialog;
	delete ChangeQueue;
	delete DDDTA;
}*/
//#define DEBUG_INFO 

IDDrawSurface::IDDrawSurface(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpTAddsc, LPDIRECTDRAWSURFACE FAR* arg2, IUnknown FAR* arg3, HRESULT* rtn_p,
	bool iWindowed, int iScreenWidth, int iScreenHeight)
{
	HRESULT result;

	HKEY hKey;
	DWORD dwDisposition;
	DWORD Size;

	lpFront = NULL;
	lpBack = NULL;
	lpDDClipper = NULL;

	Windowed = iWindowed;
	ScreenWidth = iScreenWidth;
	ScreenHeight = iScreenHeight;
	LocalShare->ScreenWidth = iScreenWidth;
	LocalShare->ScreenHeight = iScreenHeight;

	LocalShare->DDrawSurfClass = this;
	LocalShare->TADirectDrawFrontSurface = lpFront;


	//check if version is 3.1 standar
	if ((*((unsigned char*)0x4ad494)) == 0x00 && (*((unsigned char*)0x4ad495)) == 0x55 && (*((unsigned char*)0x4ad496)) == 0xe8)
	{
		LocalShare->CompatibleVersion = true;
	}
	else
	{
		LocalShare->CompatibleVersion = false;
		DataShare->ehaOff = 1; //set the ehaofvariable
	}

	PlayingMovie = false;
	DisableDeInterlace = false;


	RegCreateKeyEx(HKEY_CURRENT_USER, TADRREGPATH, NULL, TADRCONFIGREGNAME, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	Size = sizeof(bool);
	if (RegQueryValueEx(hKey, "DisableDeInterlaceMovie", NULL, NULL, (unsigned char*)&DisableDeInterlace, &Size) != ERROR_SUCCESS)
	{
		//value does not exist.. create it
		DisableDeInterlace = true;
		RegSetValueEx(hKey, "DisableDeInterlaceMovie", NULL, REG_BINARY, (unsigned char*)&DisableDeInterlace, sizeof(bool));
	}
	RegCloseKey(hKey);

	LocalShare->OrgLocalPlayerID = (*TAmainStruct_PtrPtr)->LocalHumanPlayer_PlayerID;


	//---------
// 	RegCreateKeyEx(HKEY_CURRENT_USER, TADRREGPATH, NULL, TADRCONFIGREGNAME, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
// 	Size = sizeof(bool);
// 	if(RegQueryValueEx(hKey, "VSync", NULL, NULL, (unsigned char*)&VerticalSync, &Size) != ERROR_SUCCESS)
// 	{
// 		VerticalSync = true;
// 	}

	// fix later (remove these flags altogether)!!! - also where is that other override thingy i made? - mass code overhaul..
	

	//VidMem = MyConfig->GetIniBool("UseVideoMemory", TRUE);

	VidMem = false;

	//if (VidMem)
	//{
	//	lpTAddsc->ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY);
	//	lpTAddsc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	//}
	//else
	//{
	//	lpTAddsc->ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY);
	//	lpTAddsc->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	//}

	result = lpDD->CreateSurface(lpTAddsc, arg2, arg3);
	//if (result != DD_OK)
	//{
	//	//VidMem= ! VidMem;

	//	//MyConfig->SetIniBool ( "UseVideoMemory", VidMem);

	//	if (VidMem)
	//	{
	//		lpTAddsc->ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY);
	//		lpTAddsc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	//	}
	//	else
	//	{
	//		lpTAddsc->ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY);
	//		lpTAddsc->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	//	}
	//	result = lpDD->CreateSurface(lpTAddsc, arg2, arg3);
	//}
	*rtn_p = result;

	lpFront = *arg2;
	LocalShare->TADirectDrawFrontSurface = lpFront;


	SettingsDialog = new Dialog(VidMem);
	WhiteBoard = new AlliesWhiteboard(VidMem);
	Income = new CIncome(VidMem);
	TAHook = new CTAHook(VidMem);
	CommanderWarp = new CWarp(VidMem);
	SharedRect = new CMapRect(VidMem);
	ChangeQueue = new CChangeQueue;
	DDDTA = new CDDDTA;

#ifdef USEMEGAMAP

	if (GUIExpander
		&& (GUIExpander->myMinimap))
	{
		GUIExpander->myMinimap->InitSurface(reinterpret_cast<LPDIRECTDRAW>(LocalShare->TADirectDraw), VidMem);
	}
#endif


	DataShare->IsRunning = 10;
	//SettingsDialog->SetAll();

#ifdef USEMEGAMAP
	if (GUIExpander
		&& GUIExpander->myMinimap)
	{
		GUIExpander->myMinimap->SetVid(VidMem);
	}
#endif

	if (NowSupportUnicode)
	{
		NowSupportUnicode->Set(VidMem);
	}

	lpBackLockOn = false;

	*arg2 = (IDirectDrawSurface*)this;

	OutptTxt("IDDrawSurface Created");
}

HRESULT __stdcall IDDrawSurface::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	OutptTxt("QueryInterface");
	return lpFront->QueryInterface(riid, ppvObj);
}

ULONG __stdcall IDDrawSurface::AddRef()
{
	OutptTxt("AddRef");
	return lpFront->AddRef();
}

ULONG __stdcall IDDrawSurface::Release()
{
	OutptTxt("DDrawSurface::Release");
	if (lpDDClipper)
	{
		lpDDClipper->Release();
		lpDDClipper = NULL;
	}
	ULONG result;
	if (lpFront)
	{
		result = lpFront->Release();
	}


	if (ScreenRegion)
	{
		delete ScreenRegion;
		ScreenRegion = NULL;
	}
	if (BattleFieldRegion)
	{
		delete BattleFieldRegion;
		BattleFieldRegion = NULL;
	}



	delete WhiteBoard;
	WhiteBoard = NULL;
	//delete Income;
	//Income = NULL;
	delete TAHook;
	TAHook = NULL;
	delete CommanderWarp;
	CommanderWarp = NULL;
	delete SharedRect;
	SharedRect = NULL;
	//delete SettingsDialog;
	//SettingsDialog = NULL;
	delete ChangeQueue;
	ChangeQueue = NULL;
	delete DDDTA;
	DDDTA = NULL;
#ifdef USEMEGAMAP
	if (GUIExpander
		&& (GUIExpander->myMinimap))
	{
		GUIExpander->myMinimap->ReleaseSurface();
	}
#endif
	LocalShare->OrgLocalPlayerID = 0xff;
	delete this;
	return result;
}

HRESULT __stdcall IDDrawSurface::AddOverlayDirtyRect(LPRECT arg1)
{
	OutptTxt("AddOverlayDirtyRect");
	return lpFront->AddOverlayDirtyRect(arg1);
}

HRESULT __stdcall IDDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE arg1)
{
	OutptTxt("AddAttachedSurface");
	return lpFront->AddAttachedSurface(arg1);
}

HRESULT __stdcall IDDrawSurface::Blt(LPRECT arg1, LPDIRECTDRAWSURFACE arg2, LPRECT arg3, DWORD arg4, LPDDBLTFX arg5)
{
	OutptTxt("Blt");
	PlayingMovie = true;
	return lpFront->Blt(arg1, arg2, arg3, arg4, arg5);
}

HRESULT __stdcall IDDrawSurface::BltBatch(LPDDBLTBATCH arg1, DWORD arg2, DWORD arg3)
{
	OutptTxt("BltBatch");
	return lpFront->BltBatch(arg1, arg2, arg3);
}

HRESULT __stdcall IDDrawSurface::BltFast(DWORD arg1, DWORD arg2, LPDIRECTDRAWSURFACE arg3, LPRECT arg4, DWORD arg5)
{
	OutptTxt("BltFast");
	return lpFront->BltFast(arg1, arg2, arg3, arg4, arg5);
}

HRESULT __stdcall IDDrawSurface::DeleteAttachedSurface(DWORD arg1, LPDIRECTDRAWSURFACE arg2)
{
	OutptTxt("DeleteAttachedSurface");
	return lpFront->DeleteAttachedSurface(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::EnumAttachedSurfaces(LPVOID arg1, LPDDENUMSURFACESCALLBACK arg2)
{
	OutptTxt("EnumAttachedSurfaces");
	return lpFront->EnumAttachedSurfaces(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::EnumOverlayZOrders(DWORD arg1, LPVOID arg2, LPDDENUMSURFACESCALLBACK arg3)
{
	OutptTxt("EnumOverlayZOrders");
	return lpFront->EnumOverlayZOrders(arg1, arg2, arg3);
}

HRESULT __stdcall IDDrawSurface::Flip(LPDIRECTDRAWSURFACE arg1, DWORD arg2)
{
	OutptTxt("Flip");
	return lpFront->Flip(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::GetAttachedSurface(LPDDSCAPS arg1, LPDIRECTDRAWSURFACE FAR* arg2)
{
	OutptTxt("GetAttachedSurface");
	HRESULT result = lpFront->GetAttachedSurface(arg1, arg2);

	//memcpy(&onlySurfaceCaps, arg1, sizeof(DDSCAPS));


	lpBack = *arg2;
	LocalShare->TADirectDrawBackSurface = *arg2;
#ifndef XPOYDEBG
	CreateClipplist();
#endif
	return result;
}

HRESULT __stdcall IDDrawSurface::GetBltStatus(DWORD arg1)
{
	OutptTxt("GetBltStatus");
	return lpFront->GetBltStatus(arg1);
}

HRESULT __stdcall IDDrawSurface::GetCaps(LPDDSCAPS arg1)
{
	OutptTxt("GetCaps");
	return lpFront->GetCaps(arg1);
}

HRESULT __stdcall IDDrawSurface::GetClipper(LPDIRECTDRAWCLIPPER FAR* arg1)
{
	OutptTxt("GetClipper");
	return lpFront->GetClipper(arg1);
}

HRESULT __stdcall IDDrawSurface::GetColorKey(DWORD arg1, LPDDCOLORKEY arg2)
{
	OutptTxt("GetColorKey");
	return lpFront->GetColorKey(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::GetDC(HDC FAR* arg1)
{
	OutptTxt("GetDC");
	return lpFront->GetDC(arg1);
}

HRESULT __stdcall IDDrawSurface::GetFlipStatus(DWORD arg1)
{
	OutptTxt("GetFlipStatus");
	return lpFront->GetFlipStatus(arg1);
}

HRESULT __stdcall IDDrawSurface::GetOverlayPosition(LPLONG arg1, LPLONG arg2)
{
	OutptTxt("GetOverlayPosition");
	return lpFront->GetOverlayPosition(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::GetPalette(LPDIRECTDRAWPALETTE FAR* arg1)
{
	OutptTxt("GetPalette");
	return lpFront->GetPalette(arg1);
}

HRESULT __stdcall IDDrawSurface::GetPixelFormat(LPDDPIXELFORMAT arg1)
{
	OutptTxt("GetPixelFormat");
	return lpFront->GetPixelFormat(arg1);
}

HRESULT __stdcall IDDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC arg1)
{
	OutptTxt("GetSurfaceDesc");
	return lpFront->GetSurfaceDesc(arg1);
}

HRESULT __stdcall IDDrawSurface::Initialize(LPDIRECTDRAW arg1, LPDDSURFACEDESC arg2)
{
	OutptTxt("Initialize");
	return lpFront->Initialize(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::IsLost()
{
	OutptTxt("IsLost");
	HRESULT rslt = lpBack->IsLost();
	if (DD_OK != rslt)
	{
		return rslt;
	}
	return lpFront->IsLost();
}
extern BOOL Gb_TempEHA;
HRESULT __stdcall IDDrawSurface::Lock(LPRECT arg1, LPDDSURFACEDESC arg2, DWORD arg3, HANDLE arg4)
{
	HRESULT result;
	//if (Gb_TempEHA)
	//{

	// new (restored)
	//result = lpFront->Lock(arg1, arg2, arg3, arg4);

	//	return result;
	//}

	//result = lpBack->Lock ( arg1, arg2, arg3, arg4);
	// 
	// 
	// original code
	result = lpFront->Lock(arg1, arg2, arg3, arg4);


	if (result == DD_OK)
	{
		lpBackLockOn = true;
		TAHook->TABlit();
		SurfaceMemory = arg2->lpSurface;


		memcpy(&onlySurfaceDescFRONT, arg2, sizeof(DDSURFACEDESC));

		lpFrontDescGlobal = arg2;
		lpFrontSurfaceGlobal = arg2->lpSurface;


		lastPitch = lpFrontDescGlobal->lPitch;
	}
	else
	{
		memset(&onlySurfaceDescFRONT, 0, sizeof(DDSURFACEDESC));
		SurfaceMemory = NULL;
	}

	lPitch = arg2->lPitch;
	dwHeight = arg2->dwHeight;
	dwWidth = arg2->dwWidth;






	return result;

}

HRESULT __stdcall IDDrawSurface::ReleaseDC(HDC arg1)
{
	OutptTxt("ReleaseDC");
	return lpFront->ReleaseDC(arg1);
}

HRESULT __stdcall IDDrawSurface::Restore()
{
	OutptTxt("Restore");
	((CDDDTA*)LocalShare->DDDTA)->FrameUpdate();
	lpFront->Restore(); //

	return lpFront->Restore();
}

HRESULT __stdcall IDDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER arg1)
{
	OutptTxt("SetClipper");
	//lpDDClipper = arg1;
	return lpFront->SetClipper(arg1);
}

HRESULT __stdcall IDDrawSurface::SetColorKey(DWORD arg1, LPDDCOLORKEY arg2)
{
	OutptTxt("SetColorKey");
	return lpFront->SetColorKey(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::SetOverlayPosition(LONG arg1, LONG arg2)
{
	OutptTxt("SetOverlayPosition");
	return lpFront->SetOverlayPosition(arg1, arg2);
}

HRESULT __stdcall IDDrawSurface::SetPalette(LPDIRECTDRAWPALETTE arg1)
{
	OutptTxt("SetPalette");
	Palette = arg1;
	return lpFront->SetPalette(arg1);
}

// fine but flickering
//HRESULT __stdcall IDDrawSurface::Unlock(LPVOID arg1)
//{
////OutptTxt("Unlock");
//	HRESULT result;
//	UpdateTAProcess ( );
//
//	GameingState * GameingState_P= (*TAmainStruct_PtrPtr)->GameingState_Ptr;
//
//	// causes it to break if commented out
//	if (Gb_TempEHA)
//	{
//		result = lpFront->Unlock(arg1);
//		return result;
//	}
//
//
//#ifdef XPOYDEBUG
//	
//#endif
//	if (PlayingMovie) //deinterlace and flip directly
//	{
//		if (!DisableDeInterlace)
//		{
//			DeInterlace();
//			HRESULT result = lpBack->Unlock(arg1);
//			lpBackLockOn = false;
//			lpFront->Flip(NULL, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC);
//			//if(lpFront->Flip(NULL, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC) != DD_OK)
//			//	{
//			// 					if(lpFront->Flip(NULL, DDFLIP_NOVSYNC) != DD_OK)
//			// 						lpFront->Flip(NULL, DDFLIP_WAIT);
//			//				}
//			
//			return result;
//		}
//		else
//			PlayingMovie = false;
//	}
//
//	if (DataShare->ehaOff == 1 && !DataShare->PlayingDemo) //disable everything
//	{//just unlock flip and return
//		lpDDClipper->SetClipList(ScreenRegion, 0);
//
//		result = lpBack->Unlock(arg1);
//		lpBackLockOn = false;
//
//		lpFront->Flip(NULL, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC);
//	}
//	else 
//	{
//		if(SurfaceMemory!=NULL)
//		{
//			WhiteBoard->LockBlit ( (char*)SurfaceMemory, lPitch);
//
//			if (GameingState_P
//				&&(gameingstate::MULTI==GameingState_P->State))
//			{
//				SharedRect->LockBlit ( (char*)SurfaceMemory, lPitch);
//			}
//#ifdef USEMEGAMAP
//			if ((GUIExpander)
//				&&(GUIExpander->myMinimap))
//			{
//				GUIExpander->myMinimap->LockBlit ( (char*)SurfaceMemory, dwWidth, dwHeight, lPitch);
//			}
//#endif
//		}
//
//		result = lpBack->Unlock ( arg1);
//		if(result!=DD_OK)
//		{
//			//lpBackLockOn= false;
//			return result;
//		}
//		lpBackLockOn = false;
//		
//		DDDTA->Blit(lpBack);
//
//		lpDDClipper->SetClipList ( BattleFieldRegion,0);
//		WhiteBoard->Blit(lpBack);
//
//
//		if (GameingState_P
//			&&(gameingstate::MULTI==GameingState_P->State))
//		{
//			CommanderWarp->Blit(lpBack);
//		}
//
//			
//#ifdef USEMEGAMAP
//		if ((GUIExpander)
//			&&(GUIExpander->myMinimap))
//		{
//			GUIExpander->myMinimap->Blit ( lpBack);
//		}
//
//#endif
//		
//		lpDDClipper->SetClipList(ScreenRegion,0);
//		if (GameingState_P
//			&&(gameingstate::MULTI==GameingState_P->State))
//		{
//			Income->BlitIncome(lpBack);
//		}
//			
//		SettingsDialog->BlitDialog(lpBack);
//		
//
//		//////////////////////////////////////////////////////////////////////////
//		//unicode
//		if (NULL!=NowSupportUnicode)
//		{
//			NowSupportUnicode->Blt ( lpBack);
//		}
//
//		if(VerticalSync)
//		{
//			//lpFront->Flip(NULL, DDFLIP_DONOTWAIT | DDBLT_ASYNC);
//
//			if(lpFront->Flip(NULL, DDFLIP_DONOTWAIT|  DDFLIP_NOVSYNC) != DD_OK)
//						{
//							lpFront->Flip(NULL, DDFLIP_WAIT);
//						}
//						
//		}
//		else
//		{
//			//lpFront->Blt(NULL, lpBack, NULL, DDFLIP_DONOTWAIT, NULL);
//  			 	if(lpFront->Blt(NULL, lpBack, NULL, DDFLIP_DONOTWAIT|  DDFLIP_NOVSYNC, NULL) != DD_OK)
//  			 	{
//  			 			lpFront->Blt(NULL, lpBack, NULL, DDFLIP_WAIT, NULL);
//  			 	}
//		}
//	}
//	
//
//	
//	return result;
//}

// not fine, no flicker but game frames not being updated after tabbing in / out
// ^^^^ old notes lol ^^^^
HRESULT __stdcall IDDrawSurface::Unlock(LPVOID arg1)
{


	//if (GUIExpander->myMinimap->IsBliting()/* && LAYER > 0 */ )
	//{
	//	return lpFront->Unlock(arg1);
	//	//GUIExpander->myMinimap->UpdateFrame(lpFront, &onlySurfaceDescFRONT); //front

	//	
	//}
	//else if(GUIExpander->myMinimap->IsBliting())
	//{
		//LAYER++;
	//}


	//OutptTxt("Unlock");
	HRESULT result;
	UpdateTAProcess();

	GameingState* GameingState_P = (*TAmainStruct_PtrPtr)->GameingState_Ptr;

	//if (Gb_TempEHA)
	{
		//result = lpFront->Unlock(arg1);
		//return result;
	//}

	//if (VerticalSync)
	//{
		if (PlayingMovie) //deinterlace and flip directly
		{
			if (!DisableDeInterlace)
			{
				DeInterlace();
				result = lpFront->Unlock(arg1);

				//if (lpFront->Blt(NULL, lpFront, NULL, DDBLT_ASYNC, NULL) != DD_OK)
				//{
				//	lpFront->Blt(NULL, lpFront, NULL, DDBLT_WAIT, NULL);
				//	//OutptTxt("lpBack to lpFront Blit failed");
				//}
				lpBackLockOn = false;
				return result;
			}
			else
				PlayingMovie = false;
		}

		if (DataShare->ehaOff == 1 && !DataShare->PlayingDemo) //disable everything
		{//just unlock flip and return
			lpDDClipper->SetClipList(ScreenRegion, 0);
			//result = lpFront->Unlock(arg1);
			//if (result != DD_OK)
			//{
				//lpBackLockOn = false;
				//return result;
			//}
		}
		else
		{
			if (SurfaceMemory != NULL)
			{


				if(!megamapON)
					WhiteBoard->LockBlit((char*)SurfaceMemory, lPitch);




				if (GameingState_P
					&& (gameingstate::MULTI == GameingState_P->State))
				{
					SharedRect->LockBlit((char*)SurfaceMemory, lPitch);
				}

#ifdef USEMEGAMAP
				if ((GUIExpander)
					&& (GUIExpander->myMinimap))
				{
					GUIExpander->myMinimap->LockBlit((char*)SurfaceMemory, dwWidth, dwHeight, lPitch);
				}
#endif
			}

			//result = lpFront->Unlock(arg1);
			//if (result != DD_OK)
			//{
			//	lpBackLockOn = false;
			//	return result;
			//}



			DDDTA->Blit(lpFront);




			lpDDClipper->SetClipList(BattleFieldRegion, 0);


			if(!megamapON)
				WhiteBoard->Blit((LPBYTE)SurfaceMemory);




			//if (!megamapON)
			//{
			//	if (GameingState_P
			//		&& (gameingstate::MULTI == GameingState_P->State))
			//	{
			//		CommanderWarp->Blit(lpFront);
			//	}
			//}

//#ifdef USEMEGAMAP

			//LPBYTE lpSurfaceMem = NULL;


			//if ((GUIExpander)
			//	&& (GUIExpander->myMinimap))
			//{

			//	//	IncomeStructureShare = Income;
			//	//	ScreenRegionShare = ScreenRegion;
			//	//	lpDDClipperShare = lpDDClipper;

			//	if (GUIExpander->myMinimap->IsBliting())
			//	{
			//		//lpFront->Lock(NULL, &onlySurfaceDescFRONT, DDLOCK_WAIT, 0);

			//		GUIExpander->myMinimap->UpdateFrame(lpFront, &onlySurfaceDescFRONT); //front

			//		//lpFront->Unlock(arg1);

			//		//LAYER = 0;
			//	}
			//}
			//else
			//{
				//lpDDClipper->SetClipList(ScreenRegion, 0);

				if (GameingState_P
					&& (gameingstate::MULTI == GameingState_P->State))
				{
					Income->BlitIncome((LPBYTE)SurfaceMemory);
				}
			//}
//#endif	
//#else


			//lpDDClipper->SetClipList(ScreenRegion, 0);

			//if (GameingState_P
			//	&& (gameingstate::MULTI == GameingState_P->State))
			//{


			//LPBYTE pushvar;


			//__asm
			//{
			//	mov eax, dword ptr ds:[0x0051FBD0]
			//	mov eax, [eax + 0xBC]
			//	mov eax, [eax + 0xC]
			//	mov pushvar, eax
			//}


			//Income->BlitIncome(pushvar);
			
			
			//}

			//#endif

			//#ifdef USEMEGAMAP
			//			if ((GUIExpander)
			//				&& (GUIExpander->myMinimap))
			//			{
			//				GUIExpander->myMinimap->RenderMouseCursor();
			//				GUIExpander->myMinimap->GameDrawerFlip();
			//			}
			//#endif
			//


//
//#ifdef USEMEGAMAP
//
////			
//			if ((GUIExpander)
//				&& (GUIExpander->myMinimap))
//			{
			
				UpdateTAProcess();
				if (DataShare->TAProgress == TAInGame)
				{
					if (SurfaceMemory != NULL)
					{
						//DDSURFACEDESC lpDDSurfaceDesc;

						//this->Lock(NULL, &lpDDSurfaceDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);

						GAFFrame* Cursor_GafP = (*TAProgramStruct_PtrPtr)->Cursor;

						POINT Aspect;
						Aspect.x = lPitch;
						Aspect.y = dwHeight;

						POINT pt;
						GetCursorPos(&pt);

						CopyGafToBits((LPBYTE)SurfaceMemory, &Aspect, pt.x, pt.y, Cursor_GafP);

						//this->Unlock(lpDDSurfaceDesc.lpSurface);
					}
				}
			//}
			
//#endif



			// disabling settings dialog
			// going to file system instead
			// fact of the matter is im lazy


			//LPBYTE pushvar;


			//__asm
			//{
			//	mov eax, dword ptr ds:[0x0051FBD0]
			//	mov eax, [eax + 0xBC]
			//	mov eax, [eax + 0xC]
			//	mov pushvar, eax
			//}

			//SettingsDialog->BlitDialog(pushvar);




			//////////////////////////////////////////////////////////////////////////
			//unicode
			if (NULL != NowSupportUnicode)
			{
				NowSupportUnicode->Blt(lpFront);
			}


			/*
			if (lpFront->Blt(NULL, lpBack, NULL, DDBLT_ASYNC, NULL) != DD_OK)
			{
				lpFront->Blt(NULL, lpBack, NULL, DDBLT_WAIT, NULL);
				//OutptTxt("lpBack to lpFront Blit failed");
			}
			*/


			//DDSURFACEDESC surfacedata1;
			//DDSURFACEDESC surfacedata2;


			//if (lpFront->Lock(NULL, &onlySurfaceDescFRONT, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL) == DD_OK)
			//{
			//	//if (lpBack->Lock(NULL, &surfacedata2, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL) == DD_OK)
			//	//{
			//	memcpy(onlySurfaceDescFRONT.lpSurface, onlySurfaceDescBACK.lpSurface, onlySurfaceDescBACK.dwWidth * onlySurfaceDescBACK.dwHeight);


				//}

				
			//}







			//result = lpFront->Unlock(NULL);
		}

		lpFront->Unlock(arg1);

		//lpFront->Unlock(arg1);


		lpBackLockOn = false;
		return result;
	}
}
//else
//{
//		if (PlayingMovie) //deinterlace and flip directly
//		{
//			if (!DisableDeInterlace)
//			{
//				DeInterlace();
//				HRESULT result = lpBack->Unlock(arg1);
//				if (lpFront->Flip(NULL, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC) != DD_OK)
//				{
//					if (lpFront->Flip(NULL, DDFLIP_NOVSYNC) != DD_OK)
//						lpFront->Flip(NULL, DDFLIP_WAIT);
//				}
//				lpBackLockOn = false;
//				return result;
//			}
//			else
//				PlayingMovie = false;
//		}
//
//
//		if (DataShare->ehaOff == 1 && !DataShare->PlayingDemo) //disable everything
//		{//just unlock flip and return
//			lpDDClipper->SetClipList(ScreenRegion, 0);
//			result = lpBack->Unlock(arg1);
//			if (result != DD_OK)
//			{
//				lpBackLockOn = false;
//				return result;
//			}
//		}
//		else
//		{
//			if (SurfaceMemory != NULL)
//			{
//				WhiteBoard->LockBlit((char*)SurfaceMemory, lPitch);
//
//				if (GameingState_P
//					&& (gameingstate::MULTI == GameingState_P->State))
//				{
//					SharedRect->LockBlit((char*)SurfaceMemory, lPitch);
//				}
//
//#ifdef USEMEGAMAP
//				if ((GUIExpander)
//					&& (GUIExpander->myMinimap))
//				{
//					GUIExpander->myMinimap->LockBlit((char*)SurfaceMemory, dwWidth, dwHeight, lPitch);
//				}
//#endif
//
//			}
//
//			result = lpBack->Unlock(arg1);
//			if (result != DD_OK)
//			{
//				lpBackLockOn = false;
//				return result;
//			}
//
//			DDDTA->Blit(lpBack);
//
//			lpDDClipper->SetClipList(BattleFieldRegion, 0);
//			WhiteBoard->Blit(lpBack);
//
//
//
//			if (GameingState_P
//				&& (gameingstate::MULTI == GameingState_P->State))
//			{
//				CommanderWarp->Blit(lpBack);
//			}
//
//
//
//
//#ifdef USEMEGAMAP
//			//LPBYTE lpSurfaceMem = NULL;
//
//			if ((GUIExpander)
//				&& (GUIExpander->myMinimap))
//			{
//
//				/*IncomeStructureShare = Income;
//				ScreenRegionShare = ScreenRegion;
//				lpDDClipperShare = lpDDClipper;*/
//
//
//				GUIExpander->myMinimap->UpdateFrame(lpBack);
//			}
//			//else
//			//{
//			//	if (GameingState_P
//			//		&& (gameingstate::MULTI == GameingState_P->State))
//			//	{
//			//		Income->BlitIncome(lpBack);
//			//	}
//			//}
////#else
//
//#endif
//
//			lpDDClipper->SetClipList(ScreenRegion, 0);
//
//			if (GameingState_P
//				&& (gameingstate::MULTI == GameingState_P->State))
//			{
//				Income->BlitIncome(lpBack);
//			}
//
//
////#ifdef USEMEGAMAP
////			if ((GUIExpander)
////				&& (GUIExpander->myMinimap))
////			{
////				GUIExpander->myMinimap->RenderMouseCursor();
////				GUIExpander->myMinimap->GameDrawerFlip();
////			}
////#endif
//
//
////#ifdef USEMEGAMAP
////
////			if ((GUIExpander)
////				&& (GUIExpander->myMinimap))
////			{
////				if (lpSurfaceMem != NULL)
////				{
////					DDSURFACEDESC lpDDSurfaceDesc;
////
////					//this->Lock(NULL, &lpDDSurfaceDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
////
////					GAFFrame* Cursor_GafP = (*TAProgramStruct_PtrPtr)->Cursor;
////
////					POINT Aspect;
////					Aspect.x = lPitch;
////					Aspect.y = dwHeight;
////
////					CopyGafToBits((LPBYTE)lpSurfaceMem, &Aspect, MouseX, MouseY, Cursor_GafP);
////
////					this->Unlock(lpDDSurfaceDesc.lpSurface);
////				}
////			}
////#endif
//
//			SettingsDialog->BlitDialog(lpBack);
//
//			//////////////////////////////////////////////////////////////////////////
//			//unicode
//			if (NULL != NowSupportUnicode)
//			{
//				NowSupportUnicode->Blt(lpBack);
//			}
//		}
//
//		if (lpFront->Flip(NULL, DDFLIP_DONOTWAIT) != DD_OK)
//		{
//			lpFront->Flip(NULL, DDFLIP_WAIT);
//		}
//
//
//		if (lpBack->Blt(NULL, lpFront, NULL, DDBLT_ASYNC, NULL) != DD_OK)
//		{
//			lpBack->Blt(NULL, lpFront, NULL, DDBLT_WAIT, NULL);
//			//OutptTxt("lpFront to lpBack Blit failed");
//		}
//	}

	//lpBackLockOn = false;
	//return result;
//}

HRESULT __stdcall IDDrawSurface::UpdateOverlay(LPRECT arg1, LPDIRECTDRAWSURFACE arg2, LPRECT arg3, DWORD arg4, LPDDOVERLAYFX arg5)
{
	//OutptTxt("UpdateOverlay");
	return lpFront->UpdateOverlay(arg1, arg2, arg3, arg4, arg5);
}

HRESULT __stdcall IDDrawSurface::UpdateOverlayDisplay(DWORD arg1)
{
	//OutptTxt("UpdateOverlayDisplay");
	return lpFront->UpdateOverlayDisplay(arg1);
}

HRESULT __stdcall IDDrawSurface::UpdateOverlayZOrder(DWORD arg1, LPDIRECTDRAWSURFACE arg2)
{
	//OutptTxt("UpdateOverlayZOrder");
	return lpFront->UpdateOverlayZOrder(arg1, arg2);
}
#define  DEBUG_INFO
void IDDrawSurface::OutptTxt(char* string)
{
	//#ifdef DEBUG_INFO
	//	//AnsiString CPath = "c:\\taddrawlog.txt";
	//	OutputDebugStringA ( string);
	//
	//	HANDLE file = CreateFileA("C:\\taddrawlog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS	, 0, NULL);
	//	DWORD tempWritten;
	//	SetFilePointer ( file, 0, 0, FILE_END);
	//	WriteFile ( file, string, strlen(string), &tempWritten, NULL);
	//	WriteFile ( file, "\r\n", 2, &tempWritten, NULL);
	//
	//	CloseHandle ( file);
	//#endif //DEBUG
}

void IDDrawSurface::OutptInt(int Int_I)
{
#ifdef DEBUG_INFO
	char Buffer_Int[0x10];
	wsprintf(Buffer_Int, "%x", Int_I);
	//OutptTxt ( Buffer_Int);
#endif //DEBUG
}
void IDDrawSurface::FrontSurface(LPDIRECTDRAWSURFACE lpTASurf)
{
	//IDDrawSurface::OutptTxt ( "FrontSurface");
	lpFront = lpTASurf;
}

void IDDrawSurface::Set(bool EnableVSync)
{
	VerticalSync = EnableVSync;
}

void IDDrawSurface::CreateClipplist()
{
	//IDDrawSurface::OutptTxt ( "CreateClipplist");
	LPDIRECTDRAW TADD = (IDirectDraw*)LocalShare->TADirectDraw;
	if (lpFront)
	{
		if (lpDDClipper)
		{
			lpDDClipper->Release();
		}
		TADD->CreateClipper(0, &lpDDClipper, NULL);

		ScreenRegion = (LPRGNDATA)new char[sizeof(RGNDATAHEADER) + sizeof(RECT)];
		BattleFieldRegion = (LPRGNDATA)new char[sizeof(RGNDATAHEADER) + sizeof(RECT)];

		RECT ScreenRect = { 0,0,ScreenWidth,ScreenHeight };
		memcpy(ScreenRegion->Buffer, &ScreenRect, sizeof(RECT));
		ScreenRegion->rdh.dwSize = sizeof(RGNDATAHEADER);
		ScreenRegion->rdh.iType = RDH_RECTANGLES;
		ScreenRegion->rdh.nCount = 1;
		ScreenRegion->rdh.nRgnSize = sizeof(RECT);
		ScreenRegion->rdh.rcBound.left = 0;
		ScreenRegion->rdh.rcBound.top = 0;
		ScreenRegion->rdh.rcBound.right = ScreenWidth;
		ScreenRegion->rdh.rcBound.bottom = ScreenHeight;

		RECT BattleFieldRect = { 128,32,ScreenWidth,ScreenHeight - 32 };
		memcpy(BattleFieldRegion->Buffer, &BattleFieldRect, sizeof(RECT));
		BattleFieldRegion->rdh.dwSize = sizeof(RGNDATAHEADER);
		BattleFieldRegion->rdh.iType = RDH_RECTANGLES;
		BattleFieldRegion->rdh.nCount = 1;
		BattleFieldRegion->rdh.nRgnSize = sizeof(RECT);
		BattleFieldRegion->rdh.rcBound.left = 128;
		BattleFieldRegion->rdh.rcBound.top = 32;
		BattleFieldRegion->rdh.rcBound.right = ScreenWidth;
		BattleFieldRegion->rdh.rcBound.bottom = ScreenHeight - 32;


		lpFront->SetClipper(lpDDClipper);
	}
}

void IDDrawSurface::ScreenShot()
{
	//create the creenshot
	char ScreenShotName[MAX_PATH * 2];

	int i = 0;
	CreateFileName(ScreenShotName, i);

	WIN32_FIND_DATA fs;
	HANDLE File = FindFirstFile(ScreenShotName, &fs);
	while (File != INVALID_HANDLE_VALUE)
	{
		i++;
		CreateFileName(ScreenShotName, i);
		FindClose(File);
		File = FindFirstFile(ScreenShotName, &fs);
	}
	//FindClose(Handle);

	//OutptTxt(ScreenShotName);

	//while (lpBackLockOn)
	//{
		//Sleep ( 1);
	//}

	PCX PCXScreen = PCX();

	DDSURFACEDESC ddsd;
	DDRAW_INIT_STRUCT(ddsd);
	if (lpFront->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL) == DD_OK)
	{
		char* Buff = new char[LocalShare->ScreenWidth * LocalShare->ScreenHeight];
		for (int j = 0; j < LocalShare->ScreenHeight; j++)
		{
			memcpy(Buff + j * LocalShare->ScreenWidth, ((char*)ddsd.lpSurface) + j * ddsd.lPitch, LocalShare->ScreenWidth);
		}

		PCXScreen.NewBuffer(LocalShare->ScreenWidth, LocalShare->ScreenHeight);
		PCXScreen.SetBuffer((UCHAR*)Buff);
		PCXScreen.CopyPalette(TAPalette);
		if (PCXScreen.Save(ScreenShotName) == false)
		{
		}
		//OutptTxt("error writing screenshot");


		//PCXScreen.SetBuffer(NULL);
		
		lpFront->Unlock(NULL);

		//delete Buff;

	}


}

void IDDrawSurface::CreateFileName(char* Buff, int Num)
{
	int* PTR = (int*)0x00511de8;
	char* RootDir = (char*)(*PTR + 0x38A53);
	lstrcpyA(Buff, RootDir);
	lstrcatA(Buff, "\\screenshots\\");
	CreateDir(Buff); //creates the dir so it exist
	char CNum[10];
	wsprintf(CNum, "%.4i", Num);

	char Date[100];
	struct tm temp_time_now;
	struct tm* time_now = &temp_time_now;
	time_t timer;
	timer = time(NULL);
	localtime_s(&temp_time_now, &timer);
	strftime(Date, 100, "%x - ", time_now);
	CorrectName(Date);
	lstrcatA(Buff, Date);

	if (DataShare->TAProgress != TAInGame)
	{
		lstrcatA(Buff, "SHOT");
		lstrcatA(Buff, CNum);
		lstrcatA(Buff, ".pcx");
		return;
	}

	char TempBuff[100];
	lstrcpyA(TempBuff, DataShare->MapName);
	CorrectName(TempBuff);
	lstrcatA(Buff, TempBuff);
	lstrcatA(Buff, " - ");
	lstrcpyA(TempBuff, DataShare->PlayerNames[0]);
	CorrectName(TempBuff);
	lstrcatA(Buff, TempBuff);
	for (int i = 1; i < 10; i++)
	{
		if (strlen(DataShare->PlayerNames[i]) > 0)
		{
			lstrcatA(Buff, ", ");
			lstrcpyA(TempBuff, DataShare->PlayerNames[i]);
			CorrectName(TempBuff);
			lstrcatA(Buff, TempBuff);
		}
	}
	lstrcatA(Buff, " ");
	lstrcatA(Buff, CNum);
	lstrcatA(Buff, ".pcx");
}

void IDDrawSurface::CreateDir(char* Dir)
{
	char* ptr;
	ptr = strstr(Dir, "\\");
	ptr++;
	/*ptr = strstr(Dir, "\\");
	if(ptr!=NULL)
	ptr++;*/
	while (ptr != NULL)
	{
		char CDir[MAX_PATH];
		memcpy(CDir, Dir, ptr - Dir);
		CDir[(ptr - Dir)] = '\0';
		CreateDirectory(CDir, NULL);
		ptr = strstr(ptr, "\\");
		if (ptr != NULL)
			ptr++;
	}
	CreateDirectory(Dir, NULL);
}

void IDDrawSurface::CorrectName(char* Name)
{
	for (size_t i = 0; i < strlen(Name); i++)
	{
		char C = Name[i];
		/*    if((C<'a' || C>'z') && (C<'A' || C>'Z') && (C<'1' || C>'0') && (C<'!' || C>'&') && C!='(' && C!=')'
		&& C!='[' && C!=']' && C!='\0')
		Name[i] = '_';        */
		if (strchr("\\/:*?\"<>|", C))
			Name[i] = '_';
	}
}

void IDDrawSurface::DeInterlace()
{
	//IDDrawSurface::OutptTxt ( "CreateClipplist");
	int PaletteEntry = 0;

	Palette->GetEntries(NULL, 1, 1, (PALETTEENTRY*)&PaletteEntry);
	if (PaletteEntry != 83886208)
	{
		if (SurfaceMemory != NULL)
		{
			char* SurfMem = (char*)SurfaceMemory;
			for (int i = 1; i < (LocalShare->ScreenHeight); i += 2)
			{
				memcpy(&SurfMem[i * lPitch], &SurfMem[(i + 1) * lPitch], LocalShare->ScreenWidth);
			}
		}
	}
	else
		PlayingMovie = false;
}


int currentTime2 = 0;
int triggerTime2 = 0;
int trigger2 = 0;

float GammaValue1 = 0.9f;
float GammaValue2 = 1.0f;

unsigned int* GammaFunc = (unsigned int*)0x004BA590;

extern int InTab;
extern int InTab2;

LRESULT CALLBACK WinProc(HWND WinProcWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	__try
	{


		//if (DataShare->TAProgress == TAInGame)
		//{
		//	if()
		//}

		


		if (Msg == WM_KEYDOWN)
		{

			if (wParam == VK_F12)
			{

				//
				//ChatText("+gamma 0");
				//ChatText("+gamma 10");

				__asm
				{
					push GammaValue1
					call GammaFunc
				}

				__asm
				{
					push GammaValue2
					call GammaFunc
				}

				//CallInternalCommandHandler("gamma 0", 1);
				//CallInternalCommandHandler("gamma 10", 1);
			}

			/*
			// perm los
			if (wParam == VK_F9)
			{
				DataShare->LockOn = LocalShare->OrgLocalPlayerID;
				DataShare->LosViewOn = DataShare->LockOn;
				ViewPlayerLos_Replay(DataShare->LosViewOn);
			}
			//if (wParam == VK_TAB)
			//{
			//	InTab = true;
			//}


			*/
		}

		//if (Msg == WM_LBUTTONDOWN || Msg == WM_RBUTTONDOWN)
		//{
		//	InTab2 = false;
		//}

		//if (Msg == WM_MOUSEWHEEL)
		//{
		//	InTab = false;
		//}

		//if (Msg == WM_KEYDOWN)
		//{
		//	if (wParam == VK_ESCAPE)
		//	{
		//		InTab = false;
		//	}
		//}


		//if (Msg == WM_SIZE)
		//{
		//	currentTime2 = clock();
		//	triggerTime2 = currentTime2 + 1250;

		//	trigger2 = 1;
		//}


		//if (trigger2 == 1)
		//{
		//	currentTime2 = clock();

		//	if (currentTime2 >= triggerTime2)
		//	{
		//		//CallInternalCommandHandler("gamma 0", 1);
		//		//CallInternalCommandHandler("gamma 10", 1);

		//		__asm
		//		{
		//			push GammaValue1
		//			call GammaFunc
		//		}

		//		__asm
		//		{
		//			push GammaValue2
		//			call GammaFunc
		//		}

		//		trigger2 = 0;
		//	}
		//}


		UpdateTAProcess();

		if (NULL != FixTABug)
		{
			FixTABug->AntiCheat();
		}

		if (DataShare->ehaOff == 1 && !DataShare->PlayingDemo)
		{
			return LocalShare->TAWndProc(WinProcWnd, Msg, wParam, lParam);
		}

		//////////////////////////////////////////////////////////////////////////
		if (NULL != NowCrackLimit)
		{
			if (myExternHotKey->Message(WinProcWnd, Msg, wParam, lParam))
				return 0;

		}
		if ((NULL != NowSupportUnicode)
			&& NowSupportUnicode->Message(WinProcWnd, Msg, wParam, lParam))
		{
			return 0;
		}
		if ((NULL != FixTABug)
			&& FixTABug->Message(WinProcWnd, Msg, wParam, lParam))
		{
			return 0;
		}
		if((Msg == WM_KEYUP)||(WM_KEYDOWN==Msg))
		{
			if(((wParam == 0x78 && (GetAsyncKeyState ( 17) &0x8000)>0) ) // F9 + Ctrl
				|| wParam == VK_SNAPSHOT)
			{
				if (Msg == WM_KEYUP)
				{
					((IDDrawSurface*)LocalShare->DDrawSurfClass)->ScreenShot();
				}

				return 0;
			}
		}
		//////////////////////////////////////////////////////////////////////////

		if ((NULL != LocalShare->Whiteboard)
			&& (((AlliesWhiteboard*)LocalShare->Whiteboard)->Message(WinProcWnd, Msg, wParam, lParam)))
			return 0;

		if ((NULL != LocalShare->Income)
			&& (((CIncome*)LocalShare->Income)->Message(WinProcWnd, Msg, wParam, lParam)))
			return 0;  //message handled by the income class

		//if (NULL != (LocalShare->Dialog))
		//{
		//	if (((Dialog*)LocalShare->Dialog)->Message(WinProcWnd, Msg, wParam, lParam))
		//		return 0;  //message handled by the dialog
		//}


		if ((LocalShare->CommanderWarp)
			&& (((CWarp*)LocalShare->CommanderWarp)->Message(WinProcWnd, Msg, wParam, lParam)))
			return 0;


		if ((NULL != LocalShare->TAHook)
			&& (((CTAHook*)LocalShare->TAHook)->Message(WinProcWnd, Msg, wParam, lParam)))
			return 0;  //message handled by tahook class


		//   if(((CChangeQueue*)LocalShare->ChangeQueue)->Message(WinProcWnd, Msg, wParam, lParam))
		//     return 0;

		if ((NULL != LocalShare->DDDTA)
			&& (((CDDDTA*)LocalShare->DDDTA)->Message(WinProcWnd, Msg, wParam, lParam)))
			return 0;

		//   if(((CUnitRotate*)LocalShare->UnitRotate)->Message(WinProcWnd, Msg, wParam, lParam))
		//     return 0;

		if (DataShare->F1Disable)
			if (Msg == WM_KEYDOWN && wParam == 112)
				return 0;
#ifdef USEMEGAMAP
		if (GUIExpander
			&& (GUIExpander->myMinimap))
		{
			if (GUIExpander->myMinimap->Message(WinProcWnd, Msg, wParam, lParam))
			{
				return 0;
			}
		}
#endif
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		;// return LocalShare->TAWndProc(WinProcWnd, Msg, wParam, lParam);
	}
	return LocalShare->TAWndProc(WinProcWnd, Msg, wParam, lParam);
}


