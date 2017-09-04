#include "stdafx.h"
#include "ParvaImg.h"

//ULONG_PTR m_gdiplusToken;

ParvaImg::ParvaImg(HDC hdc)
{
	this->m_hdc = hdc;
	InitGdi();
}

ParvaImg::~ParvaImg()
{
	Empty();

	GdiplusShutdown(m_gdiplusToken);
}

bool ParvaImg::DrawImage(TCHAR * ImageFile, RECT rc)
{
	if (!LoadFromPNGFile(ImageFile))
		return false;

	return this->DrawImage(rc);
	
}

bool ParvaImg::DrawImage(HINSTANCE hInst, int ResourceID, RECT rc)
{
	if (!LoadFromPNGRes(hInst, ResourceID))
		return false;

	return this->DrawImage(rc);
}

void ParvaImg::DrawRichText(TCHAR * text, Point pt, TCHAR * fontName, int fontSize)
{
	
}

void ParvaImg::Empty()
{
	if (m_pImage != NULL)
		delete m_pImage;

	m_pImage = NULL;
}

void ParvaImg::SetDC(HDC newDC)
{
	m_hdc = newDC;
}

bool ParvaImg::LoadFromPNGFile(TCHAR * file)
{
	Empty();

	m_pImage = Image::FromFile(file);
	//错误判断  
	if ((m_pImage == NULL) || (m_pImage->GetLastStatus() != Ok))
	{
		try { delete m_pImage; }
		catch (...) { assert(FALSE); } m_pImage = NULL;
		return false;
	}

	return true;
}

bool ParvaImg::LoadFromPNGRes(HINSTANCE hInst, int ResourceID)
{
	bool bRet = false;
	Empty();

	HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(ResourceID), _T("PNG"));
	if (!hResource)
		return false;

	DWORD imageSize = ::SizeofResource(hInst, hResource);
	if (!imageSize)
		return false;

	const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
	if (!pResourceData)
		return false;
	
	auto m_hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (m_hBuffer)
	{
		void* pBuffer = ::GlobalLock(m_hBuffer);
		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;
			if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK)
			{
				m_pImage = Image::FromStream(pStream);
				pStream->Release();
				if (m_pImage)
				{
					if (m_pImage->GetLastStatus() == Gdiplus::Ok)
						bRet = true;
					else Empty();
				}
			}			
			::GlobalUnlock(m_hBuffer);
		}
		::GlobalFree(m_hBuffer);
	}
	return bRet;
}

bool ParvaImg::DrawImage(RECT rc)
{
	assert(m_pImage != NULL);
	//创建屏幕  
	assert(m_hdc != NULL);
	Graphics graphics(m_hdc);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);


	//获取属性  
	INT nImageWidth = rc.right - rc.left;
	INT nImageHeight = rc.bottom - rc.top;

	//构造位置  
	RectF rcDrawRect;
	rcDrawRect.X = (REAL)0;
	rcDrawRect.Y = (REAL)0;
	rcDrawRect.Width = (REAL)nImageWidth;
	rcDrawRect.Height = (REAL)nImageHeight;

	auto pScaledImage = m_pImage->GetThumbnailImage(nImageWidth, nImageHeight, NULL, NULL);
	//绘画图像  
	graphics.DrawImage(pScaledImage, rcDrawRect, 0, 0, (REAL)nImageWidth, (REAL)nImageHeight, UnitPixel);
	delete pScaledImage;

	return true;
}

void ParvaImg::InitGdi()
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

