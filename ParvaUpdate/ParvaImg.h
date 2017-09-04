#pragma once
#include "stdafx.h"
#include <string>
#include <tchar.h>  
#include <assert.h>  

#include <GdiPlus.h>  


//µ¼ÈëGDI+µÄ¿â  
#pragma comment(lib, "Gdiplus.lib") 

using namespace Gdiplus;
 
class ParvaImg
{
public:
	
	ParvaImg(HDC hdc);
	~ParvaImg();
	bool DrawImage(TCHAR* ImageFile, RECT rc);
	bool DrawImage(HINSTANCE hInst, int ResourceID, RECT rc);
	void DrawRichText(TCHAR* text, Point pt, TCHAR* fontName, int fontSize);

	void Empty();
	void SetDC(HDC newDC);
protected:
	void InitGdi();
	bool LoadFromPNGFile(TCHAR * file);
	bool LoadFromPNGRes(HINSTANCE hInst, int ResourceID);
	bool DrawImage(RECT rc);
private:	
	ULONG_PTR m_gdiplusToken;
	 
	Image*	m_pImage = NULL; //pngÍ¼Æ¬ 
	HDC		m_hdc = NULL;
};