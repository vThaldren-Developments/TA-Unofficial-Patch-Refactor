#pragma once



class MappedMap
{
public:
	MappedMap (int Width, int Height);
	~MappedMap();

	BOOL NowDrawMapped (LPBYTE PixelBits,  POINT * AspectSrc);
	LPBYTE PictureInfo (LPBYTE * PixelBits_pp, POINT * Aspect);

private:


	BYTE TAGrayTABLE[256];

	HANDLE Event_h;
};
