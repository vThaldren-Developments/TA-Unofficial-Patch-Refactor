#pragma once

class TAGameAreaReDrawer
{
public:
	TAGameAreaReDrawer ();
	~TAGameAreaReDrawer ();

	void BlitTAGameArea (LPBYTE DestSurf);

	BOOL MixDSufInBlit (LPBYTE DestSurf, LPRECT DescRect, LPBYTE Src_DDrawSurface, LPRECT SrcScope);
	BOOL MixBitsInBlit (LPRECT DescRect, LPBYTE SrcBits, LPPOINT SrcAspect, LPRECT SrcScope);
	BOOL GrayBlitOfBits (LPRECT DescRect, LPBYTE SrcBits, LPPOINT SrcAspect, LPRECT SrcScope, BOOL NoMapped);

	LPRECT const TAWGameAreaRect (LPRECT Out_Rect);

	LPBYTE InitOwnSurface (LPDIRECTDRAW TADD, BOOL VidMem);

	void ReleaseSurface (void);

	HRESULT Lock (  LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
	HRESULT Unlock(  LPVOID lpSurfaceData);
		
	LPBYTE Flip (void);
	void Cls (void);
	LPBYTE const backSurface_p();
	LPBYTE const frontSurface_p();

	int width;
	int height;

private:
	LPBYTE GameAreaSurfaceFront_ptr;
	LPBYTE GameAreaSurfaceBack_ptr;
	LPBYTE DescBuf;
	RECT DescRect_Buf;


};