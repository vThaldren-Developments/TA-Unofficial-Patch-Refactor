#pragma once

#define USEMEGAMAP


#define MAXCURSORWIDTH (0x30)
#define MAXCURSORHEIGHT (0x30)

struct tagTNTHeaderStruct;
class TNTtoMiniMap;
class TAGameAreaReDrawer;
class InlineSingleHook;
class SingleHook;

class UnitsMinimap;
class MappedMap;
class ProjectileMap;
class MegaMapControl;
class MegamapTAStuff;

struct tagInlineX86StackBuffer;
typedef struct tagInlineX86StackBuffer * PInlineX86StackBuffer;



class FullScreenMinimap
{
public:



	DDSURFACEDESC lpDDSurfaceDesc;
	int surfwidth;
	int surfheight;



	int megamapFps;
	BOOL Working_B;
	TAGameAreaReDrawer * GameDrawer;
	UnitsMinimap * UnitsMap;
	TNTtoMiniMap * MyMinimap_p;
	MappedMap* Mapped_p;
	ProjectileMap* ProjectilesMap_p;
	MegaMapControl * Controler;
	MegamapTAStuff* TAStuff;

	BOOL Blit_b;
	BOOL Flipping;

	BOOL Do_b;

	int MegamapWidth;
	int MegamapHeight;
	RECT MegaMapInscren;
	RECT MegamapRect;

	RECT TAMAPTAPos;
	//DDSURFACEDESC lpDDSurfaceDesc;

public:
	FullScreenMinimap (BOOL Doit, int FPSlimit= 1000);
	~FullScreenMinimap (void);

	void InitMinimap (tagTNTHeaderStruct * TNTPtr, RECT *  GameScreen= NULL);

	void Blit(LPDIRECTDRAWSURFACE DestSurf);

	void LockBlit (LPVOID lpSurfaceMem, int dwWidth,int dwHeight, int lPitch);


	void LockBlit_TA (LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch);
	void LockBlit_MEGA (LPVOID lpSurfaceMem, int dwWidth, int dwHeight, int lPitch);
	void BlitSurfaceCursor (LPDIRECTDRAWSURFACE DestSurf);

	void InitSurface (LPDIRECTDRAW TADD, BOOL VidMem);
	void InitSurface (LPDIRECTDRAW TADD);
	void ReleaseSurface (void);

	bool Message(HWND WinProcWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	BOOL IsBliting ( );
	void EnterMegaMap ();
	void QuitMegaMap ( );

	void Set (int VirtualKey);
	void SetVid (BOOL VidMem_a);

	void DischargeGUIState ( );
	void BlockGUIState ( );

	void __stdcall UpdateFrame(LPVOID DestSurf, LPDDSURFACEDESC DestDesc);// nah no para

	void RenderMouseCursor();
	void GameDrawerFlip();



	void writejmp(unsigned int injectp, unsigned int targetp)
	{
		DWORD Old;
		if (VirtualProtect((LPVOID)injectp, 5, PAGE_EXECUTE_READWRITE, &Old))
		{
			__asm
			{
				mov edi, injectp
				mov eax, targetp
				sub eax, edi
				sub eax, 5
				mov byte ptr[edi], 0xE9
				mov dword ptr[edi + 1], eax
			}

			VirtualProtect((LPVOID)injectp, 5, PAGE_EXECUTE_READ, &Old);
		}
	}
	




private:
	InlineSingleHook * LoadMap_hook;

	int MegamapVirtualKey;

	int MaxIconWidth;
	int MaxIconHeight;


	BOOL DrawBackground;
	BOOL DrawMapped;
	BOOL DrawProjectile;
	BOOL DrawUnits;
	BOOL DrawMegamapRect;
	BOOL DrawMegamapBlit;
	BOOL DrawSelectAndOrder;
	BOOL DrawMegamapCursor;
	BOOL DrawMegamapTAStuff;

	BOOL WheelZoom;
	BOOL WheelMoveMegaMap;
	BOOL DoubleClickMoveMegamap;

	BOOL UseSurfaceCursor;

	InlineSingleHook * DrawTAScreen_hok;
	InlineSingleHook * DrawTAScreenBlit_hok;
	InlineSingleHook * DrawTAScreenEnd_hok;

	SingleHook * KeepActive;
	SingleHook * KeepActive1;
	BOOL VidMem;

	DWORD worker_tID;
	HANDLE worker_thd;
};



int __stdcall BlockTADraw (PInlineX86StackBuffer X86StrackBuffer);
int __stdcall DischargeTADraw (PInlineX86StackBuffer X86StrackBuffer);


int __stdcall ForceTADrawBlit (PInlineX86StackBuffer X86StrackBuffer);