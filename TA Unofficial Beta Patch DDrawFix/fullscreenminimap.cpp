// .text:00483638 0B4 8B D8                                                           mov     ebx, eax
// 	.text:0048363A 0B4 8B 03                                                           mov     eax, [ebx]
// .text:0048363C 0B4 3D 20 10 00 00                                                  cmp     eax, 4128

//#include "PFix.hpp"

#include "oddraw.h"



#include "hook/hook.h"
#include "hook/etc.h"
#include "tamem.h"
#include "tafunctions.h"
#include "mapParse.h"
#include "gameredrawer.h"
#include "UnitDrawer.h"
#include "mappedmap.h"
#include "PCX.H"

#include "megamaptastuff.h "
#include "ProjectilesMap.h"
#include "MegamapControl.h"
#include "fullscreenminimap.h"
#include "dialog.h"
#include "gaf.h"
#include "cincome.h"

#include "iddrawsurface.h"

#include <ctime>

#include <vector>
using namespace std;
#include "TAConfig.h"


//#include "newglobals.h"

#ifdef USEMEGAMAP


LPBYTE MiniMapPixelBits; // real map picture - copy from this!!

LPBYTE savedMEGAMAPsurface;

LPBYTE NewCopiedmmpb;
int mmpbWidth;
int mmpbHeight;

extern LPBYTE MappedBits;


LPBYTE TAStuff_Surfc;

int width;
int height;


bool megamapON;
DWORD nextMEGAtickcount;
DWORD nextMEGAtickcountDRAWBOX;




LPBYTE FinalRenderForKeyframe;



//
//extern LPBYTE MiniMapPixelBits;
//extern LPBYTE NewCopiedmmpb;
//extern int mmpbWidth;
//extern int mmpbHeight;
//
//
//
//extern LPBYTE MappedBits;
//extern int Width_mapped;
//extern int Height_mapped;
//
//
//
//extern LPBYTE UnitsMapSfc;
//extern int surfwidthunits;
//extern int surfheightunits;


extern int LAYER;


extern LPDDSURFACEDESC lpFrontDescGlobal;
extern LPVOID lpFrontSurfaceGlobal;
LPVOID ThisClass;
extern int lastPitch;







MappedMap* Mapped_p_Global;
ProjectileMap* ProjectilesMap_p_Global;
UnitsMinimap* UnitsMap_Global;
MegamapTAStuff* TAStuff_Global;
MegaMapControl* MegamapControls_Global;
TAGameAreaReDrawer* GameDrawer_Global;



bool mutexlock;
bool gamehasticked;
bool hasframeupdated;

bool firsttime = true;


__declspec(naked) void IfRenderMegaThenBlock()
{
	__asm
	{
		mov al, byte ptr [megamapON]
		test al, al
		jz dontskip


		push lpFrontDescGlobal

		mov eax, dword ptr ds:[0x0051FBD0]
		mov eax, [eax+0xBC]
		mov eax, [eax+0xC]

		push eax
		push ThisClass
		call FullScreenMinimap::UpdateFrame
		


		//skip:
		mov eax, 0x00469F23
		jmp eax

		dontskip:
		mov ecx, dword ptr ds:[0x511de8]
		mov eax, 0x0046962A
		jmp eax
	}
}




__declspec(naked) void IfRenderMegaThenBlock2()
{
	__asm
	{
		mov al, byte ptr [megamapON]
		test al, al
		jz dontskip

		//skip:
		add esp, 4





		mov eax, 0x00468E35
		jmp eax


		dontskip:
		mov eax, 0x00483FA0
		call eax


		mov eax, 0x00468DB5
		jmp eax
	}
}






__declspec(naked) void gametimeupdate()
{
	__asm
	{
		mov ecx, [eax+0x38a47]

		cmp byte ptr [mutexlock], 1
		je locked
		mov byte ptr [gamehasticked], 1
		locked:

		push 0x004954BD
		ret
	}
}




//__declspec(naked) void IfRenderThenDrawTopGUI()
//{
//	__asm
//	{
//		mov al, byte ptr [megamapON]
//		test al, al
//		jz dontskip
//
//		//skip:
//		add esp, 4
//
//		mov eax, 0x00468E35
//		jmp eax
//
//
//		dontskip:
//		mov eax, 0x00483FA0
//		call eax
//
//
//		mov eax, 0x00468DB5
//		jmp eax}
//
//}
//
//




extern CIncome* IncomeStructureShare;
extern LPRGNDATA ScreenRegionShare;
extern LPDIRECTDRAWCLIPPER lpDDClipperShare;



DWORD WINAPI MegamapFrameThd(
	
);



int __stdcall LoadMap_Routine (PInlineX86StackBuffer X86StrackBuffer)
{
	TNTHeaderStruct * TNTPtr= (TNTHeaderStruct *)X86StrackBuffer->Eax;
	FullScreenMinimap * thethis= (FullScreenMinimap * )(X86StrackBuffer->myInlineHookClass_Pish->ParamOfHook);

	
	thethis->InitMinimap ( TNTPtr);



	return 0;
}

int currentTime;
int triggerTime;
int trigger;

DWORD WINAPI MegamapFrameThd(
	
)
{

	//FullScreenMinimap * cls_p = static_cast<FullScreenMinimap *>(megamapPtr);

	//do 
	//{
	//	
	//	currentTime = clock();

	//	if (currentTime >= triggerTime)
	//	{
	//		//cls_p->UpdateFrame();

	//		triggerTime = currentTime + (1000 / cls_p->megamapFps) - 1;
	//	}
	//
	//	//Sleep ( 1000/ cls_p->megamapFps);

	//} while (cls_p->Working_B);

	POINT Aspect;
	bool localgamehasticked;
	bool boolhasframeupdated;
	bool localmegamapON;

	do
	{
		if (gameingstate::EXITGAME == (*TAmainStruct_PtrPtr)->GameStateMask)
		{
			return TRUE;
		}


		// stupid vs2022 compiler optimizer problems makes this not really happen
		localgamehasticked = gamehasticked;
		boolhasframeupdated = hasframeupdated;
		localmegamapON = megamapON;

		//if (firsttime == true)
		//{
		//	firsttime = false;
		//	goto firsttimelabel;
		//}


		if ((localgamehasticked == true && boolhasframeupdated == true && localmegamapON == true) || firsttime == true)
		{ // run megamap picture update
			//firsttimelabel:
			firsttime = false;

			// fix this crap later lol
			//RECT in;
			//in.left = 0;
			//in.right = Aspect.x;
			//in.top = 0;
			//in.bottom = Aspect.y;

			//RECT in2;
			//in2.left = 0;
			//in2.right = in.right;
			//in2.top = 0;
			//in2.bottom = in.bottom;



			//surfwidth = in2.right - in2.left;
			//surfheight = in2.bottom - in2.top;


			if (!NewCopiedmmpb || !savedMEGAMAPsurface)
			{



				NewCopiedmmpb = (LPBYTE)malloc(mmpbWidth * mmpbHeight + 1);
				savedMEGAMAPsurface = (LPBYTE)malloc(mmpbWidth * mmpbHeight + 1);


			}



 			memcpy(NewCopiedmmpb, MiniMapPixelBits, mmpbWidth * mmpbHeight);


			Aspect.x = mmpbWidth;
			Aspect.y = mmpbHeight;


			Mapped_p_Global->NowDrawMapped(NewCopiedmmpb, &Aspect);



			NewCopiedmmpb = MappedBits;


			ProjectilesMap_p_Global->NowDrawProjectile(NewCopiedmmpb, &Aspect);

			UnitsMap_Global->NowDrawUnits(NewCopiedmmpb, &Aspect);


			if (savedMEGAMAPsurface && NewCopiedmmpb && (firsttime || hasframeupdated) /* && TAStuff->GetTAStuffSurface()*/)
				memcpy(savedMEGAMAPsurface, NewCopiedmmpb, mmpbWidth * mmpbHeight);



			mutexlock = true;
			gamehasticked = false;
			hasframeupdated = false;
			mutexlock = false;
		}
		else
		{
			Sleep((int)(1000.0 / 30));
		}






		//UpdateTAProcess();
	} while (true);












	return TRUE;
}

FullScreenMinimap::FullScreenMinimap (BOOL Doit, int FPSlimit)
{
	//IDDrawSurface::OutptTxt ( "FullScreenMinimap init");
	LoadMap_hook= NULL;
	MyMinimap_p= NULL;
	GameDrawer= NULL;
	UnitsMap= NULL;
	Mapped_p= NULL;
	ProjectilesMap_p= NULL;
	TAStuff= NULL;

	KeepActive= NULL;
	KeepActive1= NULL;
	DrawTAScreen_hok= NULL;
	DrawTAScreenEnd_hok= NULL;


	MegamapVirtualKey= VK_TAB;

	savedMEGAMAPsurface = 0;

	Blit_b= FALSE;
	megamapON = false;

	Flipping= FALSE;

	

	Controler= NULL;


	VidMem= FALSE;

	DrawBackground=  TRUE;
	DrawMapped= TRUE;
	DrawProjectile=  TRUE;
	DrawUnits= TRUE;
	DrawMegamapRect=  TRUE;
	DrawMegamapBlit=  TRUE;
	DrawSelectAndOrder= TRUE;
	DrawMegamapCursor=  TRUE;
	DrawMegamapTAStuff= TRUE;

	DoubleClickMoveMegamap=  FALSE;
	WheelMoveMegaMap= TRUE;
	WheelZoom=  TRUE;

	UseSurfaceCursor= FALSE;

	MaxIconWidth= ICONMAXWIDTH;
	MaxIconHeight= ICONMAXHEIGHT;


	surfwidth = -1;
	surfheight = -1;
	nextMEGAtickcount = 0;
	LAYER = 0;
	

	memset ( &MegamapRect, 0, sizeof(RECT) );
	memset ( &MegaMapInscren, 0, sizeof(RECT) );
	memset ( &TAMAPTAPos, 0, sizeof(RECT) );
	
	MegamapWidth= 0;
	MegamapHeight= 0;

	Do_b= Doit;


	//megamapFps = FPSlimit;
	if (Doit)
	{
		LoadMap_hook= new InlineSingleHook ( LoadMap_Addr , 5, INLINE_5BYTESLAGGERJMP, LoadMap_Routine);
		
		LoadMap_hook->SetParamOfHook ( (LPVOID)this);
		GameDrawer= new TAGameAreaReDrawer;
		

		DWORD junk;


		if (VirtualProtect((LPVOID)0x004C29C5, 3, PAGE_EXECUTE_READWRITE, &junk))
		{
			memset((void*)0x004C29C5, 0x90, 3);

			VirtualProtect((LPVOID)0x004C29C5, 3, PAGE_EXECUTE_READ, &junk);
		}


		if (VirtualProtect((LPVOID)0x004954B7, 5, PAGE_EXECUTE_READWRITE, &junk))
		{
			writejmp(0x004954B7, (unsigned int)gametimeupdate);

			VirtualProtect((LPVOID)0x004954B7, 5, PAGE_EXECUTE_READ, &junk);
		}




	}

	//Working_B = TRUE;// work!! <-- ok xpoy xD



	
	 //DrawTAScreen_hok= new InlineSingleHook ( (unsigned int)DrawGameScreen_Addr, 5, 
		//INLINE_5BYTESLAGGERJMP, BlockTADraw);

	//DrawTAScreen_hok->SetParamOfHook ( reinterpret_cast<LPVOID>(this));


	writejmp(0x00469624, (unsigned int)IfRenderMegaThenBlock);
	writejmp(0x00468DB0, (unsigned int)IfRenderMegaThenBlock2);

	 
	//DrawTAScreenBlit_hok= new InlineSingleHook ( (unsigned int)DrawTAScreenBlitAddr, 5, 
	//	INLINE_5BYTESLAGGERJMP, ForceTADrawBlit);

	//DrawTAScreenBlit_hok->SetParamOfHook ( reinterpret_cast<LPVOID>(this));


	

	//DrawTAScreenEnd_hok= new InlineSingleHook ( (unsigned int)DrawGameScreenEnd_Addr, 5, 
		//INLINE_5BYTESLAGGERJMP, DischargeTADraw);

	//DrawTAScreenEnd_hok->SetParamOfHook ( reinterpret_cast<LPVOID>(this));
	
	//create thread in here
	//worker_thd= 
	
	//IDDrawSurface::OutptTxt ( "Megamap Inited!");


}


FullScreenMinimap::~FullScreenMinimap (void)
{
	Working_B = FALSE;

	//WaitForSingleObject ( worker_thd, 1000);//do not wait   forever ever the FPS==1

	if (NULL!=LoadMap_hook)
	{
		delete LoadMap_hook;
	}
	if (NULL!=MyMinimap_p)
	{
		delete MyMinimap_p;
	}
	if (NULL!=GameDrawer)
	{
		delete GameDrawer;
	}

	if (NULL!=Mapped_p)
	{
		delete Mapped_p;
	}

	if (NULL!=ProjectilesMap_p)
	{
		delete ProjectilesMap_p;
	}

	if (NULL!=Controler)
	{
		delete Controler;
	}

	if (TAStuff)
	{
		delete TAStuff;

	}
	if(DrawTAScreen_hok)
	{
		delete DrawTAScreen_hok;
		DrawTAScreen_hok= NULL;
	}
	if(DrawTAScreenBlit_hok)
	{
		delete DrawTAScreenBlit_hok;
		DrawTAScreenBlit_hok= NULL;
	}

	
// 
// 	if (DrawTAScreenEnd_hok)
// 	{
// 		delete DrawTAScreenEnd_hok;
// 		DrawTAScreenEnd_hok= NULL;
// 	}

	if (KeepActive)
	{
		delete KeepActive;
		KeepActive= NULL;

	}
	if (KeepActive1)
	{
		delete KeepActive1;
		KeepActive1= NULL;
	}
}

void FullScreenMinimap::InitMinimap (TNTHeaderStruct * TNTPtr, RECT * GameScreen)
{
	if (!(IDirectDraw*)LocalShare->TADirectDraw)
	{
		return ;
	}

	QuitMegaMap ( );
	if (MyMinimap_p)
	{
		delete MyMinimap_p;
		MyMinimap_p= NULL;
	}

	if (UnitsMap)
	{
		delete UnitsMap;
		UnitsMap = NULL;
	}


// 	if (NULL==GameDrawer->GameAreaSurfaceFront_ptr)
// 	{
// 		GameDrawer->InitOwnSurface ( (IDirectDraw*)LocalShare->TADirectDraw, ! VsyncOn);
// 	}
// 	
	if (NULL==GameScreen)
	{
		GameScreen= GameDrawer->TAWGameAreaRect ( NULL);
	}

	int GameWidth= GameScreen->right- GameScreen->left;
	int GameHeight= GameScreen->bottom- GameScreen->top;

	POINT MinimapAspect;

	MyMinimap_p= new TNTtoMiniMap ( GameWidth, GameHeight);

	int PlayerDotColors[PLAYERNUM];


	if (MyMinimap_p)
	{
		MyMinimap_p->MapFromTNTInMem ( reinterpret_cast<LPVOID>(TNTPtr));

		MyMinimap_p->PictureInfo ( NULL, &MinimapAspect);

		GameDrawer->TAWGameAreaRect ( &MegaMapInscren);

		MegamapWidth= MinimapAspect.x;
		MegamapHeight= MinimapAspect.y;

		MegamapRect.right= MegaMapInscren.right- MegaMapInscren.left;
		MegamapRect.bottom= MegaMapInscren.bottom- MegaMapInscren.top;
		MegamapRect.top= 0;
		MegamapRect.left= 0;

		if ((MegamapWidth<MegamapRect.right)
			&&(2<(MegamapRect.right- MegamapWidth)))
		{
			MegamapRect.left= (MegamapRect.right- MegamapWidth)/ 2;
			MegamapRect.right= MegamapRect.left+ MegamapWidth;
		}

		if ((MegamapHeight<MegamapRect.bottom)
			&&(2<(MegamapRect.bottom- MegamapHeight)))
		{
			MegamapRect.top= (MegamapRect.bottom- MegamapHeight)/ 2;
			MegamapRect.bottom= MegamapRect.top+ MegamapHeight;
		}

		MegaMapInscren.top= MegaMapInscren.top+ MegamapRect.top;
		MegaMapInscren.bottom= MegaMapInscren.top+ MegamapHeight;
		MegaMapInscren.left= MegaMapInscren.left+ MegamapRect.left;
		MegaMapInscren.right= MegaMapInscren.left+ MegamapWidth;

		TAMAPTAPos.left= 0;
		TAMAPTAPos.top= 0;
		TAMAPTAPos.right= (TNTPtr->Width- 1)* 16;
		TAMAPTAPos.bottom= (TNTPtr->Height- 4)* 16;




		DrawBackground= MyConfig->GetIniBool ( "DrawBackground", TRUE);
		DrawMapped= MyConfig->GetIniBool ( "DrawMapped", TRUE);
		DrawProjectile= MyConfig->GetIniBool ( "DrawProjectile", TRUE);
		DrawUnits= MyConfig->GetIniBool ( "DrawUnits", TRUE);
		DrawMegamapRect= MyConfig->GetIniBool ( "DrawMegamapRect", TRUE);
		DrawMegamapBlit= MyConfig->GetIniBool ( "DrawMegamapBlit", TRUE);
		DrawSelectAndOrder= MyConfig->GetIniBool ( "DrawSelectAndOrder", TRUE);
		DrawMegamapCursor= MyConfig->GetIniBool ( "DrawMegamapCursor", TRUE);

		DrawMegamapTAStuff= MyConfig->GetIniBool ( "DrawMegamapTAStuff", TRUE);

		DoubleClickMoveMegamap=  MyConfig->GetIniBool ( "DoubleClickMoveMegamap", FALSE);
		WheelMoveMegaMap= MyConfig->GetIniBool ( "WheelMoveMegaMap", TRUE);
		WheelZoom= MyConfig->GetIniBool ( "WheelZoom", TRUE);


		UseSurfaceCursor= MyConfig->GetIniBool ( "UseSurfaceCursor", FALSE);


		MaxIconWidth= MyConfig->GetIniInt ( "MaxIconWidth", ICONMAXWIDTH) ;
		MaxIconHeight=  MyConfig->GetIniInt ( "MaxIconHeight", ICONMAXHEIGHT) ;

		
		PlayerDotColors[0]= MyConfig->GetIniInt ( "Player1DotColors", 227);
		PlayerDotColors[1]= MyConfig->GetIniInt ( "Player2DotColors", 212);
		PlayerDotColors[2]= MyConfig->GetIniInt ( "Player3DotColors", 80);
		PlayerDotColors[3]= MyConfig->GetIniInt ( "Player4DotColors", 235);
		PlayerDotColors[4]= MyConfig->GetIniInt ( "Player5DotColors", 108);
		PlayerDotColors[5]= MyConfig->GetIniInt ( "Player6DotColors", 219);
		PlayerDotColors[6]= MyConfig->GetIniInt ( "Player7DotColors", 208);
		PlayerDotColors[7]= MyConfig->GetIniInt ( "Player8DotColors", 93);
		PlayerDotColors[8]= MyConfig->GetIniInt ( "Player9DotColors", 130);
		PlayerDotColors[9]= MyConfig->GetIniInt ( "Player10DotColors", 67);

		
		UnitsMap= new UnitsMinimap (  this, MinimapAspect.x, MinimapAspect.y,
			MyConfig->GetIniInt ( "MegamapRadarMinimum", -1),
			MyConfig->GetIniInt ( "MegamapSonarMinimum", -1),
			MyConfig->GetIniInt ( "MegamapSonarJamMinimum", -1),
			MyConfig->GetIniInt ( "MegamapRadarJamMinimum", -1),
			MyConfig->GetIniInt ( "MegamapAntiNukeMinimum", -1),
			MyConfig->GetIniInt ( "MegamapWeapon1Color", -1),
			MyConfig->GetIniInt ( "MegamapWeapon2Color", -1),
			MyConfig->GetIniInt ( "MegamapWeapon3Color", -1),
			MyConfig->GetIniInt ( "MegamapRadarColor", -1),
			MyConfig->GetIniInt ( "MegamapSonarColor", -1),
			MyConfig->GetIniInt ( "MegamapRadarJamColor", -1),
			MyConfig->GetIniInt ( "MegamapSonarJamColor", -1),
			MyConfig->GetIniInt ( "MegamapAntinukeColor", -1),
			MyConfig->GetIniBool ( "UnderAttackFlash", FALSE),
			PlayerDotColors);
		//UnitsMap->InitSurface ( (IDirectDraw*)LocalShare->TADirectDraw, !VsyncOn);


		if (Mapped_p)
		{
			delete Mapped_p;
			Mapped_p= NULL;
		}
		Mapped_p= new MappedMap ( MinimapAspect.x, MinimapAspect.y);

		//RECT mmpbRECT;
		//mmpbRECT.left = MegaMapInscren.left;
		//mmpbRECT.top = 0;// MegaMapInscren.top;
		//mmpbRECT.right = mmpbRECT.left + mmpbWidth;
		//mmpbRECT.bottom = mmpbRECT.left + mmpbHeight;

		if (NULL!=Controler)
		{
			Controler->Init ( this, &MegaMapInscren, &TAMAPTAPos, GameScreen, MaxIconWidth, MaxIconHeight, MegamapVirtualKey, WheelMoveMegaMap, DoubleClickMoveMegamap, WheelZoom);
		}
		else
		{
			// arg2 - &MegaMapInscren
			Controler= new MegaMapControl ( this, &MegaMapInscren, &TAMAPTAPos, GameScreen, MaxIconWidth, MaxIconHeight, MegamapVirtualKey, WheelMoveMegaMap, DoubleClickMoveMegamap, WheelZoom);
		}

		if (TAStuff)
		{
			TAStuff->Init ( this, &MegaMapInscren, &TAMAPTAPos, GameScreen, MaxIconWidth, MaxIconHeight, UseSurfaceCursor);
		}
		else
		{
			TAStuff= new MegamapTAStuff ( this, &MegaMapInscren, &TAMAPTAPos, GameScreen, MaxIconWidth, MaxIconHeight, UseSurfaceCursor);
		}

		if (ProjectilesMap_p)
		{
			delete ProjectilesMap_p;
		}
		if (UnitsMap)
		{
			ProjectilesMap_p= new ProjectileMap ( this, UnitsMap);
		}

		//
		InitSurface ( (IDirectDraw*)LocalShare->TADirectDraw);





		Mapped_p_Global = Mapped_p;
		ProjectilesMap_p_Global = ProjectilesMap_p;
		UnitsMap_Global = UnitsMap;
		TAStuff_Global = TAStuff;
		MegamapControls_Global = Controler;
		GameDrawer_Global = GameDrawer;









		//IDDrawSurface::OutptTxt("Megamap TNT loaded!");
	}
}

void FullScreenMinimap::Set (int VirtualKey)
{
	MegamapVirtualKey= VirtualKey;
	//VidMem= VidMem_a;

	if (Controler)
	{
		Controler->Set ( VirtualKey);
	}
}
void FullScreenMinimap::SetVid (BOOL VidMem_a)
{
	VidMem= VidMem_a;
}

void __stdcall FullScreenMinimap::UpdateFrame(LPVOID DestSurf, LPDDSURFACEDESC DestDesc)
{
	if (firsttime)
	{
		DWORD threadid;
		HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MegamapFrameThd, 0, 0, &threadid);
	}

	if (lastPitch == -1)
		return;

	if (Do_b
		&& Blit_b
		&& !Flipping
		)
	{
		bool respectPitch = false;


		if (lastPitch != (*TAmainStruct_PtrPtr)->ScreenWidth)
		{
			respectPitch = true;
		}




		Flipping = TRUE;

		if (gameingstate::EXITGAME == (*TAmainStruct_PtrPtr)->GameStateMask)
		{
			megamapON = false;
			Flipping = FALSE;
			return;
		}

		//if (TAInGame == DataShare->TAProgress)
		//{
		//	//IDDrawSurface::OutptTxt ( "FullScreenMinimap blit");

		//	if (DrawMegamapBlit)
		//	{
		//		//IDDrawSurface::OutptTxt("Megamap Blit !");

		//		


			//GameDrawer->BlitTAGameArea(DestSurf);
		//	}

		//	//if (DrawMegamapCursor)
		//	//{
		//	//	// 				DDSURFACEDESC ddsd;
		//	//	// 				DDRAW_INIT_STRUCT(ddsd);

		//	//	if (UseSurfaceCursor)
		//	//	{
		//	//		BlitSurfaceCursor(DestSurf);
		//	//	}
		//	//}
		//}		


		//RECT in;


		static POINT Aspect;
		//LPBYTE PixelBits;

		if (MyMinimap_p
			&& UnitsMap)
		{
			//do
			//{
				//__try
				//{
			UpdateTAProcess();
			//IDDrawSurface::OutptTxt("Megamap Start Update Frame!");
			if (TAInGame != DataShare->TAProgress)
			{
				Flipping = FALSE;
				return;
			}

			//if (Blit_b)
			//{
			//DWORD TickCount = GetTickCount();




/*
			if (TickCount >= nextMEGAtickcount)
			{

				//if (FinalRenderForKeyframe)
				//{
				//	free(FinalRenderForKeyframe);
				//	FinalRenderForKeyframe = 0;

				//	FinalRenderForKeyframe = (LPBYTE)malloc(mmpbWidth * mmpbHeight + 1);
				//}


				


				//}

				//if (DrawMegamapCursor
				//	&& !UseSurfaceCursor)
				//{
				//	//						RECT lpDestRect;

				//							//IDDrawSurface::OutptTxt("Megamap Draw Cursor Frame!");
				//							//DDSURFACEDESC lpDDSurfaceDesc;

				//	//DDRAW_INIT_STRUCT(lpDDSurfaceDesc);


				//		//GameDrawer->Lock(NULL, &lpDDSurfaceDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);

				//		
				//		LPRECT const gameRect = GameDrawer->TAWGameAreaRect(NULL);

				//		int CursorX = Controler->PubCursorX - gameRect->left;
				//		int CursorY = Controler->PubCursorY - gameRect->top;

				//		//GameingState* GameingState_P = (*TAmainStruct_PtrPtr)->GameingState_Ptr;

				//		//lpDDClipperShare->SetClipList(ScreenRegionShare, 0);

				//		//if (GameingState_P
				//		//	&& (gameingstate::MULTI == (*TAmainStruct_PtrPtr)->GameStateMask))
				//		//{
				//		//	IncomeStructureShare->BlitIncome((LPDIRECTDRAWSURFACE)lpDDSurfaceDesc.lpSurface);
				//		//}



				//		// this cursor stuff really is not needed i dont think - just a matter of a hook
				//		if ((CursorY != -1)
				//			&& (CursorX != -1)
				//			&& (CursorX < gameRect->right)
				//			&& (CursorY < gameRect->bottom))
				//		{
				//			if (Controler == NULL || (!Controler->IsDrawRect(TRUE)))
				//			{
				//				TAStuff->DrawCursor(lpDDSurfaceDesc.lpSurface, lpDDSurfaceDesc.dwWidth, lpDDSurfaceDesc.dwHeight, lpDDSurfaceDesc.lPitch,
				//					CursorX, CursorY);
				//			}
				//		}

				//		//RECT gameArea;
				//		//GameDrawer->TAWGameAreaRect(&gameArea);
				//		//size_t height = gameArea.bottom - gameArea.top;

				//		// line by line
				//		
				//	//}

				//	//GameDrawer->Unlock(lpDDSurfaceDesc.lpSurface);
				//}


				// copy picture info from parsed map


				memcpy(NewCopiedmmpb, MiniMapPixelBits, mmpbWidth * mmpbHeight);


				Aspect.x = mmpbWidth;
				Aspect.y = mmpbHeight;
				//if (DrawBackground)
				//{
			// get picture info after copy
					//MyMinimap_p->PictureInfo(NULL, &Aspect);
				//}

				Mapped_p_Global->NowDrawMapped(NewCopiedmmpb, &Aspect);


				//if (DrawMapped)
				//{
					//if (!Mapped_p->NowDrawMapped(PixelBits, &Aspect))
					//{
						//Flipping = FALSE;
						//return;
					//}

				//}

					// set new ptr
				NewCopiedmmpb = MappedBits;


				//Mapped_p->PictureInfo(&PixelBits, &Aspect);


				//if (DrawProjectile)
				//{
					//IDDrawSurface::OutptTxt("Megamap Projectile Frame!");
				ProjectilesMap_p_Global->NowDrawProjectile(NewCopiedmmpb, &Aspect);
				//}

				//if (DrawUnits)
				//{
					//IDDrawSurface::OutptTxt("Megamap Units Frame!");
				UnitsMap_Global->NowDrawUnits(NewCopiedmmpb, &Aspect);
				//}




#define TimesPerSecond 30 // only 30 engine ticks per second
				nextMEGAtickcount = TickCount + (int)(1000.f / TimesPerSecond); // times per second



			}
*/


			//RECT in;
			//in.left = 0;
			//in.right = Aspect.x;
			//in.top = 0;
			//in.bottom = Aspect.y;

			//if (DrawMegamapRect)
			//{


				//if (DrawUnits)
				//{
					//IDDrawSurface::OutptTxt("Megamap Rect Frame!");



					//GameDrawer->MixDSufInBlit(TAStuff->GetTAStuffSurface(), &MegamapRect, UnitsMap->GetSurface(), &in);
				//}
				//else
				// 
				// 
				// 
				//{






					//for (int y = 0; y < Aspect.y; y++)
					//{
					//	memcpy((void*)((DWORD)TAStuff_Surfc + y * Aspect.x), (void*)((DWORD)UnitsMap->GetSurface() + y * Aspect.x), Aspect.x);
					//}

					//GameDrawer->MixBitsInBlit(&MegamapRect, PixelBits, &Aspect, &in);
				//}
			//}





			//IDDrawSurface::OutptTxt("Megamap TAStuff Frame!");
			//RECT in2;
			//in2.left = 0;
			//in2.right = in.right;
			//in2.top = 0;
			//in2.bottom = in.bottom;

			//TAStuff->BlitTAGameStuff(GameDrawer->backSurface_p(), &in2);

			//const RECT rectangle = in2;

			//GameDrawer->TAWGameAreaRect(&rectangle);
			//DDRAW_INIT_STRUCT(lpDDSurfaceDesc);
			////GameDrawer->Lock(NULL, &lpDDSurfaceDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
			//if (surfwidth == -1 && surfheight == -1)
			//{
			//surfwidth = in2.right - in2.left;
			//surfheight = in2.bottom - in2.top;
			//}







			//GameDrawer->Unlock(lpDDSurfaceDesc.lpSurface);




			// needed?

			//GameDrawer->Flip();








			// black background













			if (DestDesc->lpSurface && savedMEGAMAPsurface/* && DestDesc->lpSurface*/)
			{

				// black background

				int top = 32;
				int left = 128;
				int bottom = (*TAmainStruct_PtrPtr)->ScreenHeight - top - 32;

				if (respectPitch)
				{

					for (int y = 0; y < bottom; y++)
					{
						memset((void*)((LPBYTE)DestSurf + ((y + top) * (*TAmainStruct_PtrPtr)->ScreenWidth) + left), 0x00, ((*TAmainStruct_PtrPtr)->GameScreenWidth));
					}
				}
				else
				{
					for (int y = 0; y < bottom; y++)
					{
						memset((void*)((LPBYTE)DestSurf + ((y + top) * (*TAmainStruct_PtrPtr)->ScreenWidth) + left), 0x00, ((*TAmainStruct_PtrPtr)->GameScreenWidth));
					}
				}






				Aspect.x = mmpbWidth;
				Aspect.y = mmpbHeight;


				// copy megamap to top

				if (respectPitch)
				{

					for (int y = 0; y < Aspect.y; y++)
					{
						memcpy((void*)((LPBYTE)DestSurf + ((y + MegaMapInscren.top) * (*TAmainStruct_PtrPtr)->ScreenWidth) + MegaMapInscren.left), (void*)((LPBYTE)savedMEGAMAPsurface + (y * Aspect.x)), Aspect.x);
					}

				}

				else
				{
					for (int y = 0; y < Aspect.y; y++)
					{
						memcpy((void*)((LPBYTE)DestSurf + ((y + MegaMapInscren.top) * (*TAmainStruct_PtrPtr)->ScreenWidth) + MegaMapInscren.left), (void*)((LPBYTE)savedMEGAMAPsurface + (y * Aspect.x)), Aspect.x);
					}
				}

			}


			TAStuff_Global->UpdateTAGameStuffMEGA(DestSurf, DrawMegamapTAStuff, DrawSelectAndOrder, &Blit_b);

			if(!mutexlock)
				hasframeupdated = true;



			



			//LAYER = 0;



		//DWORD TickCount;
		//TickCount = GetTickCount();

		//if (TickCount >= nextMEGAtickcountDRAWBOX)
		//{
		// 
		// 
		// 
		// 
		// 
		// 
			
			
			// this function sucks
			//TAStuff->UpdateTAGameStuffTA(DestDesc, DrawMegamapTAStuff, DrawSelectAndOrder, &Blit_b);




			// 
			// 
			// 
			// 
			// 
			// 
			// 
			//
			//#define TimesPerSecondDRAWBOX 144
			//			nextMEGAtickcountDRAWBOX = TickCount + (int)(1000.f / TimesPerSecondDRAWBOX); // times per second
			//




			// --------------------------------- this is here normally - testing
			
			







			//for (int y = 0; y < Aspect.y; y++)
			//{
			//	memcpy((void*)((LPBYTE)DestDesc->lpSurface + ((y + MegaMapInscren.top) * (*TAmainStruct_PtrPtr)->ScreenWidth) + MegaMapInscren.left), (void*)((LPBYTE)savedMEGAMAPsurface + (y * Aspect.x)), Aspect.x);
			//}






			//LPRECT const gameRect = GameDrawer_Global->TAWGameAreaRect(NULL);

			//int CursorX = MegamapControls_Global->PubCursorX;// -gameRect->left;
			//int CursorY = MegamapControls_Global->PubCursorY;// -gameRect->top;


			//if ((CursorY != -1)
			//	&& (CursorX != -1)
			//	&& (CursorX < gameRect->right)
			//	&& (CursorY < gameRect->bottom))
			//{
			//	if (MegamapControls_Global == NULL || (!MegamapControls_Global->IsDrawRect(TRUE)))
			//	{
			//		if (respectPitch)
			//		{
			//			TAStuff_Global->DrawCursor(DestSurf, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, lastPitch, CursorX, CursorY);

			//		}
			//		else
			//		{
			//			TAStuff_Global->DrawCursor(DestSurf, (*TAmainStruct_PtrPtr)->ScreenWidth, (*TAmainStruct_PtrPtr)->ScreenHeight, (*TAmainStruct_PtrPtr)->ScreenWidth, CursorX, CursorY);
			//		}
			//	}
			//}

		}

	}

	//IDDrawSurface::OutptTxt("Megamap End Update Frame!");
//}
//__except (true)
//{
	//;
//}
//} while (false);



	Flipping = FALSE;
	//}

	//Blit_b = false;

	return;// (LPBYTE)lpDDSurfaceDesc.lpSurface;
}
/*
void FullScreenMinimap::RenderMouseCursor()
{
	if (Do_b
		&& Blit_b
		&& !Flipping
		)
	{
		if (MyMinimap_p
			&& UnitsMap)
		{

			if (DrawMegamapCursor
				&& !UseSurfaceCursor)
			{
				//						RECT lpDestRect;

										//IDDrawSurface::OutptTxt("Megamap Draw Cursor Frame!");
										//DDSURFACEDESC lpDDSurfaceDesc;

				DDRAW_INIT_STRUCT(lpDDSurfaceDesc);

				GameDrawer->Lock(NULL, &lpDDSurfaceDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);

				LPRECT const gameRect = GameDrawer->TAWGameAreaRect(NULL);

				int CursorX = Controler->PubCursorX - gameRect->left;
				int CursorY = Controler->PubCursorY - gameRect->top;



				if ((CursorY != -1)
					&& (CursorX != -1)
					&& (CursorX < gameRect->right)
					&& (CursorY < gameRect->bottom))
				{
					if (Controler == NULL || (!Controler->IsDrawRect(TRUE)))
					{
						TAStuff->DrawCursor(lpDDSurfaceDesc.lpSurface, lpDDSurfaceDesc.dwWidth, lpDDSurfaceDesc.dwHeight, lpDDSurfaceDesc.lPitch,
							CursorX, CursorY);
					}
				}

				GameDrawer->Unlock(lpDDSurfaceDesc.lpSurface);
			}
		}
	}
}

void FullScreenMinimap::GameDrawerFlip()
{
	if (Do_b
		&& Blit_b
		&& !Flipping
		)
	{
		if (MyMinimap_p
			&& UnitsMap)
		{
			GameDrawer->Flip();
			Flipping = FALSE;
		}
	}
}
*/
//void FullScreenMinimap::Blit(LPDIRECTDRAWSURFACE DestSurf)
//{
//
//	if (Do_b
//		&&Blit_b)
//	{
//		if (gameingstate::EXITGAME==(*TAmainStruct_PtrPtr)->GameStateMask)
//		{
//			return ;
//		}
//		if (TAInGame==DataShare->TAProgress
//			)
//		{
//			//IDDrawSurface::OutptTxt ( "FullScreenMinimap blit");
//
//			if (DrawMegamapBlit)
//			{
//				//IDDrawSurface::OutptTxt("Megamap Blit !");
//				GameDrawer->BlitTAGameArea ( DestSurf);
//			}
//
//			if (DrawMegamapCursor)
//			{
//// 				DDSURFACEDESC ddsd;
//// 				DDRAW_INIT_STRUCT(ddsd);
//
//				if(UseSurfaceCursor)
//				{
//					BlitSurfaceCursor ( DestSurf);
//				}
//			}
//		}
//	}
//}

// void FullScreenMinimap::LockBlit_MEGA (LPVOID lpSurfaceMem, int dwWidth,int dwHeight, int lPitch)
// {
// 	
// }

// void FullScreenMinimap::LockBlit_TA (LPVOID lpSurfaceMem, int dwWidth,int dwHeight, int lPitch)
// {
// 	if (Do_b
// 		&&Blit_b
// 		&&DrawMegamapTAStuff)
// 	{
// 		if (gameingstate::EXITGAME==(*TAmainStruct_PtrPtr)->GameStateMask)
// 		{
// 			return ;
// 		}
// 		if (TAInGame==DataShare->TAProgress)
// 		{
// 			TAStuff->LockBlit_TA ( lpSurfaceMem, dwWidth, dwHeight, lPitch);
// 		}
// 	}
// }




void FullScreenMinimap::LockBlit (LPVOID lpSurfaceMem, int dwWidth,int dwHeight, int lPitch)
{
	if (Do_b
		&&Blit_b)
	{
		if (gameingstate::EXITGAME == (*TAmainStruct_PtrPtr)->GameStateMask)
		{
			return;
		}

		if (TAInGame == DataShare->TAProgress)
		{

			if (DrawMegamapTAStuff)
			{
				TAStuff->LockBlit ( lpSurfaceMem, dwWidth, dwHeight, lPitch);
			}

			//TAStuff->LockBlit_MEGA ( lpSurfaceMem, dwWidth, dwHeight, lPitch);
		}

	

	}

	
}

//void FullScreenMinimap::BlitSurfaceCursor (LPDIRECTDRAWSURFACE DestSurf)
//{
//	if (! UseSurfaceCursor)
//	{
//		return ;
//	}
//	int CursorX= Controler->PubCursorX;
//	int CursorY= Controler->PubCursorY;
//
//	//if (DrawMegamapCursor)
//	{
//		if ((CursorY!=-1)
//			&&(CursorX!=-1))
//		{
//			if (Controler==NULL||(! Controler->IsDrawRect ( TRUE)))
//			{
//					TAStuff->DrawCursor ( DestSurf, CursorX,  CursorY);
//			}
//		}
//	}
//}
void FullScreenMinimap::InitSurface (LPDIRECTDRAW TADD, BOOL VidMem)
{
	if (Do_b
		&&(TALobby!=DataShare->TAProgress)
		&&MyMinimap_p)
	{
		if (UnitsMap)
		{
			POINT MinimapAspect;
			MyMinimap_p->PictureInfo ( NULL, &MinimapAspect);
			UnitsMap->ReSet ( MinimapAspect.x, MinimapAspect.y);

			UnitsMap->InitSurface ( TADD, VidMem);
		}

		if (GameDrawer)
		{
			GameDrawer->InitOwnSurface ( TADD, VidMem);
		}

		if (TAStuff)
		{
			TAStuff->InitSurface ( TADD, VidMem);
		}
	}
}
void FullScreenMinimap::InitSurface (LPDIRECTDRAW TADD)
{
	InitSurface ( TADD, VidMem);
}

void FullScreenMinimap::ReleaseSurface (void) 
{
	if (GameDrawer)
	{
		GameDrawer->ReleaseSurface ( );
		
	}
	if (UnitsMap)
	{
		UnitsMap->ReleaseSurface ( );
	}

	if (TAStuff)
	{
		TAStuff->ReleaseSurface();
	}
}

bool FullScreenMinimap::Message(HWND WinProcWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Do_b)
	{
		if (TAInGame==DataShare->TAProgress)
		{

			if (Controler)
			{
				return Controler->Message ( WinProcWnd,  Msg,  wParam,  lParam);
			}
		}
	}
	return false;
}

BOOL FullScreenMinimap::IsBliting ( )
{
	return Blit_b;
}
void FullScreenMinimap::EnterMegaMap ()
{
	Blit_b= TRUE;

	megamapON = true;
}
void FullScreenMinimap::QuitMegaMap ( )
{
	Blit_b= FALSE;

	megamapON = false;
}

void FullScreenMinimap::BlockGUIState ( )
{
/*
	BYTE KeepActiveBits[]= {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
	BYTE Rtn_buf[]= { 0xC2, 0x0C, 0x00 };
	KeepActive= new SingleHook ( (unsigned int)0x4AB0B0, sizeof(Rtn_buf), INLINE_UNPROTECTEVINMENT, Rtn_buf);
	KeepActive1= new SingleHook ( KeepActiveAddr1, sizeof(KeepActiveBits), INLINE_UNPROTECTEVINMENT, KeepActiveBits);*/
}
void FullScreenMinimap::DischargeGUIState ( )
{
/*
	if (KeepActive)
	{
		delete KeepActive;

	}
	if (KeepActive1)
	{
		delete KeepActive1;
	}*/
}

/*
.text:004699D3 224 8B 8C 24 28 02 00 00                                            mov     ecx, [esp+224h+IsDrawUnit_b]
.text:004699DA 224 85 C9                                                           test    ecx, ecx
	.text:004699DC 224 74 27                                                           jz      short loc_469A05


	.text:00469B02 224 8B 9C 24 28 02 00 00                                            mov     ebx, [esp+224h+IsDrawUnit_b]
.text:00469B09 224 85 DB                                                           test    ebx, ebx
	.text:00469B0B 224 0F 84 BF 00 00 00                                               jz      loc_469BD0



	01 224 85 DB                                                           test    ebx, ebx
	.text:00469C03 224 0F 84 2F 01 00 00                                               jz      loc_469D38


xt:00469D6D 224 85 DB                                                           test    ebx, ebx
   .text:00469D6F 224 74 22                                                           jz      short loc_469D93

xt:00469D85 224 85 DB                                                           test    ebx, ebx
   .text:00469D87 224 74 0A                                                           jz      short loc_469D93


ext:00469DB4 224 85 DB                                                           test    ebx, ebx
	.text:00469DB6 224 75 0A                                                           jnz     short loc_469DC2

	test    ebx, ebx
	.text:00469FC4 224 74 0F                                                           jz      short loc_469FD5


	test    ebx, ebx
	.text:00469FE4 224 0F 84 1D 01 00 00                                               jz      loc_46A107


	test    ebx, ebx
	.text:0046A31E 224 0F 84 99 00 00 00                                               jz      loc_46A3BD

	test    ebx, ebx
	.text:0046A3CE 224 74 10                                                           jz      short loc_46A3E0
*/

int __stdcall BlockTADraw (PInlineX86StackBuffer X86StrackBuffer)
{
	FullScreenMinimap * this_me= (FullScreenMinimap *)(X86StrackBuffer->myInlineHookClass_Pish->ParamOfHook);


	if (this_me->IsBliting ( ))
	{
		//()(X86StrackBuffer->Esp+ 4)
		//this_me->BlockGUIState ( );

		*(LPDWORD)(X86StrackBuffer->Esp+4 )= 0;
		//*(LPDWORD)(X86StrackBuffer->Esp+8 )= 0;
		X86StrackBuffer->Esp-= 0x214;
		return X86STRACKBUFFERCHANGE;
	}

	return 0;
}


int __stdcall ForceTADrawBlit (PInlineX86StackBuffer X86StrackBuffer)
{
	FullScreenMinimap * this_me= (FullScreenMinimap *)(X86StrackBuffer->myInlineHookClass_Pish->ParamOfHook);


	if (this_me->IsBliting ( ))
	{
		

		X86StrackBuffer->Eax= *(LPDWORD)(X86StrackBuffer->Esp+ 0x22c);
		//X86StrackBuffer->rtnAddr_Pvoid= ;
		return X86STRACKBUFFERCHANGE;
	}

	return 0;
}

int __stdcall DischargeTADraw (PInlineX86StackBuffer X86StrackBuffer)
{
	FullScreenMinimap * this_me= (FullScreenMinimap *)(X86StrackBuffer->myInlineHookClass_Pish->ParamOfHook);


	if (this_me->IsBliting ( ))
	{
		this_me->DischargeGUIState ( );
		//return X86STRACKBUFFERCHANGE;
	}

	return 0;
}

#endif