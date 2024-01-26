/*
(C) Petr Lastovicka

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License.
*/
#include "hdr.h"
#include "xmsol.h"

#define WINGDIPAPI __stdcall
#define GDIPCONST const

class GpImage {};
class GpBitmap : public GpImage {};

enum Status
{
	Ok = 0,
};

enum DebugEventLevel
{
	DebugEventLevelFatal,
	DebugEventLevelWarning
};

typedef Status GpStatus;
typedef DWORD ARGB;

typedef VOID(WINAPI *DebugEventProc)(DebugEventLevel level, CHAR *message);
typedef Status(WINAPI *NotificationHookProc)(OUT ULONG_PTR *token);
typedef VOID(WINAPI *NotificationUnhookProc)(ULONG_PTR token);

struct GdiplusStartupInput
{
	UINT32 GdiplusVersion;
	DebugEventProc DebugEventCallback;
	BOOL SuppressBackgroundThread;
	BOOL SuppressExternalCodecs;

	GdiplusStartupInput(
		DebugEventProc debugEventCallback = NULL,
		BOOL suppressBackgroundThread = FALSE,
		BOOL suppressExternalCodecs = FALSE)
	{
		GdiplusVersion = 1;
		DebugEventCallback = debugEventCallback;
		SuppressBackgroundThread = suppressBackgroundThread;
		SuppressExternalCodecs = suppressExternalCodecs;
	}
};

struct GdiplusStartupOutput
{
	NotificationHookProc NotificationHook;
	NotificationUnhookProc NotificationUnhook;
};

extern "C"
{
	typedef Status(WINAPI *pGdiplusStartup)(OUT ULONG_PTR *token, const GdiplusStartupInput *input, OUT GdiplusStartupOutput *output);
	typedef VOID(WINAPI *pGdiplusShutdown)(ULONG_PTR token);
	typedef GpStatus(WINGDIPAPI *pGdipDisposeImage)(GpImage *image);
	typedef GpStatus(WINGDIPAPI *pGdipCreateBitmapFromFile)(GDIPCONST WCHAR* filename, GpBitmap **bitmap);
	typedef GpStatus(WINGDIPAPI *GdipCreateBitmapFromStream)(IStream* stream, GpBitmap **bitmap);
	typedef GpStatus(WINGDIPAPI *pGdipCreateHBITMAPFromBitmap)(GpBitmap* bitmap, HBITMAP* hbmReturn, ARGB background);
}
//------------------------------------------------------------------

HBITMAP loadImage(TCHAR *file, int resource)
{
	HBITMAP hBitmap=0;
	HMODULE lib = LoadLibrary(_T("gdiplus.dll"));
	if(lib){
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		if(((pGdiplusStartup)GetProcAddress(lib, "GdiplusStartup"))(&gdiplusToken, &gdiplusStartupInput, NULL) == Ok)
		{
			GpBitmap* bitmap;
			convertT2W(file, w);
			GpStatus status = ((pGdipCreateBitmapFromFile)GetProcAddress(lib, "GdipCreateBitmapFromFile"))(w, &bitmap);
			if (status!=Ok && resource)
			{
				//load fallback image from EXE file
				HRSRC r=FindResource(0, MAKEINTRESOURCE(resource), RT_RCDATA);
				HGLOBAL h=LoadResource(0, r);
				void *p=LockResource(h);
				if(p) {
					size_t sz=SizeofResource(0, r);
					HGLOBAL h2=GlobalAlloc(GMEM_MOVEABLE, sz);
					LPVOID p2=GlobalLock(h2);
					if(p2) {
						memcpy(p2, p, sz);
						GlobalUnlock(h2);
						LPSTREAM s;
						if(!(CreateStreamOnHGlobal(h2, FALSE, &s))) {
							status = ((GdipCreateBitmapFromStream)GetProcAddress(lib, "GdipCreateBitmapFromStream"))(s, &bitmap);
							GlobalFree(h2);
						}
					}
				}
			}
			if (status==Ok)
			{
				HBITMAP hbmp;
				if(((pGdipCreateHBITMAPFromBitmap)GetProcAddress(lib, "GdipCreateHBITMAPFromBitmap"))(bitmap, &hbmp, 0) == Ok)
					hBitmap = hbmp;
				((pGdipDisposeImage)GetProcAddress(lib, "GdipDisposeImage"))(bitmap);
			}
			((pGdiplusShutdown)GetProcAddress(lib, "GdiplusShutdown"))(gdiplusToken);
		}
		FreeLibrary(lib);
	}
	if(hBitmap) return hBitmap;
	return (HBITMAP)LoadImage(0, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}


bool isRelativePath(TCHAR* fn)
{
	return fn[0] && fn[1]!=':' && fn[0]!='\\' && (fn[0]!='.' || fn[1]!='.' && fn[1]!='\\');
}

HBITMAP loadBMP(TCHAR *fn, int resource)
{
	TfileName path;
	if(isRelativePath(fn))
	{
		//prepend full path
		getExeDir(path, fn);
		fn = path;
	}
	return loadImage(fn, resource);
}

HBITMAP readBMP(TCHAR *fn, int resource, HDC, int *pwidth, int *pheight)
{
	HBITMAP fbmp= loadBMP(fn, resource);
	if(fbmp){
		BITMAP info;
		GetObject(fbmp, sizeof(BITMAP), &info);
		if(pwidth) *pwidth=info.bmWidth;
		if(pheight) *pheight=info.bmHeight;
	}
	return fbmp;
}


static TCHAR *ImgExt[] ={_T(".bmp"), _T(".jpeg"), _T(".png"), _T(".gif"), _T(".jpg"), _T(".tiff"), _T(".tif")};

bool isImage(TCHAR *file)
{
	TCHAR* ext = cutExt(file);
	if(ext)
	{
		static int formats;
		if(!formats){
			formats=1;
			HMODULE lib = LoadLibrary(_T("gdiplus.dll"));
			if(lib){
				FreeLibrary(lib);
				formats=2;
			}
		}
		if(formats==1) //Windows 98/2000
		{
			if(!_tcsicmp(ext, _T(".bmp"))) return true;
		}
		else{
			for(TCHAR **p = ImgExt; p < endA(ImgExt); p++)
			{
				if(!_tcsicmp(ext, *p)) return true;
			}
		}
	}
	return false;
}
