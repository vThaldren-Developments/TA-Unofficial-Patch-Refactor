
#pragma once

#include <Windows.h>


HMODULE PatchFixHandle;

void* BlitSurfaceToOffscreen_Func;
void* CreateNewSurface_Func;
void* DestroySurface_Func;
void* GetSurface_Func;
void* BlitSurfaceToSurface_Func;
void* ClearScreenWithColor_Func;

void(__stdcall* PFix_BlitSurfaceToOffscreen)(unsigned int Index, LPRECT Dimensions) = (void(__stdcall*)(unsigned int Index, LPRECT Dimensions))BlitSurfaceToOffscreen_Func;
void(__stdcall* PFix_CreateNewSurface)(unsigned int Index, unsigned int Width, unsigned int Height, unsigned int PixelSize) = (void(__stdcall*)(unsigned int Index, unsigned int Width, unsigned int Height, unsigned int PixelSize))CreateNewSurface_Func;
void(__stdcall* PFix_DestroySurface)(unsigned int Index) = (void(__stdcall*)(unsigned int Index))DestroySurface_Func;
unsigned char* (__stdcall* PFix_GetSurface)(unsigned int Index) = (unsigned char* (__stdcall*)(unsigned int Index))GetSurface_Func;
void(__stdcall* PFix_BlitSurfaceToSurface)(unsigned int DestSurfaceIndex, unsigned char* SourceSurface, unsigned int PixelSize, LPRECT Dimensions) = (void(__stdcall*)(unsigned int DestSurfaceIndex, unsigned char* SourceSurface, unsigned int PixelSize, LPRECT Dimensions))BlitSurfaceToSurface_Func;
void(__stdcall* PFix_ClearScreenWithColor)(unsigned int DestSurfaceIndex, LPRECT Dimensions, unsigned int PixelSize, unsigned int Color) = (void(__stdcall*)(unsigned int DestSurfaceIndex, LPRECT Dimensions, unsigned int PixelSize, unsigned int Color))ClearScreenWithColor_Func;


