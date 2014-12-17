#include "Renderer.h"
#include <vector>
#include <chrono>

using namespace std::chrono;

#define CALC_FONT_SIZE(a) -MulDiv( a, (INT)(GetDeviceCaps(hDC, LOGPIXELSY)), 72 )

HRESULT FontData::CreateFontObjects(const Font font, FontData* data, std::unique_ptr<DWORD[]>& lpBitmapBits)
{
	DWORD texWidth, texHeight;
	if (font.Size > 60)
		texWidth = 2048;
	else if (font.Size > 30)
		texWidth = 1024;
	else if (font.Size > 15)
		texWidth = 512;
	else
		texWidth = 256;

	DWORD*      pBitmapBits;
	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = texWidth;
	bmi.bmiHeader.biHeight = -static_cast<int>(texWidth);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;

	HDC hDC = CreateCompatibleDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
	SetMapMode(hDC, MM_TEXT);

	HFONT hFont = CreateFontA(CALC_FONT_SIZE(font.Size), 0, 0, 0, font.Flags, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, font.FontName.c_str());

	if (!hFont)
		return E_FAIL;

	SelectObject(hDC, hBitmap);
	SelectObject(hDC, hFont);

	SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
	SetBkColor(hDC, 0x00000000);
	SetTextAlign(hDC, TA_TOP);

	float x = 0, y = 0;
	char str[2] = "x";
	SIZE sz;

	GetTextExtentPoint32A(hDC, " ", 1, &sz);

	float spacing = sz.cx;

	std::vector<XMFLOAT4> sizes;

	for (char c = 32; c < 127; c++)
	{
		str[0] = c;
		GetTextExtentPoint32A(hDC, str, 1, &sz);

		if (x + sz.cx > texWidth)
		{
			x = 0;
			y += sz.cy;
		}

		sizes.push_back(XMFLOAT4(x, y, x + sz.cx, y + sz.cy));

		x += sz.cx + spacing / 2;
	}

	texHeight = y + sz.cy;

	byte c = 0;
	for each (auto& var in sizes)
	{
		str[0] = c + 32;

		ExtTextOutA(hDC, static_cast<int>(ceilf(var.x)), static_cast<int>(ceilf(var.y)), ETO_OPAQUE, NULL, str, 1, NULL);

		data->m_fTexCoords[c++] = { var.x / texWidth, var.y / texHeight, var.z / texWidth, var.w / texHeight };
	}

	data->m_Spacing = spacing;
	data->m_TexHeight = texHeight;
	data->m_TexWidth = texWidth;

	DWORD len = texWidth * texHeight;
	auto buffer = std::make_unique<DWORD[]>(len);

	if (!buffer)
		return E_OUTOFMEMORY;

	memcpy((void*)buffer.get(), pBitmapBits, min(len * sizeof(DWORD), texWidth * texWidth * sizeof(DWORD)));

	lpBitmapBits = std::move(buffer);

	DeleteObject(hBitmap);
	DeleteObject(hFont);
	DeleteDC(hDC);
	return S_OK;
}

high_resolution_clock::time_point lastTime = high_resolution_clock::time_point();

bool Renderer::Begin(const int fps)
{

	auto now = high_resolution_clock::now();
	auto millis = duration_cast<milliseconds>(now - lastTime).count();

	auto time = static_cast<int>(1.f / fps * std::milli::den);
	auto milliPerFrame = duration<long, std::milli>(time).count();
	if (millis >= milliPerFrame)
	{
		lastTime = now;
		return true;
	}
	return false;
}

HRESULT Renderer::GetTextExtent(FontData* font, const std::string& strText, SIZE &sz, float scale)
{
	float fRowWidth = 0.0f;
	float fRowHeight = (font->m_fTexCoords[0].y - font->m_fTexCoords[0].w) * font->m_TexHeight * scale;
	float fWidth = 0.0f;
	float fHeight = fRowHeight;

	for each(auto c in strText)
	{
		if (c == '\n')
		{
			fRowWidth = 0.0f;
			fHeight += fRowHeight;
		}

		c -= 32;

		if (c < 0 || c >= 96)
			continue;


		float tx1 = font->m_fTexCoords[c].x;
		float tx2 = font->m_fTexCoords[c].z;

		fRowWidth += (tx2 - tx1)* font->m_TexWidth;

		if (fRowWidth > fWidth)
			fWidth = fRowWidth;
	}
	sz.cx = static_cast<LONG>(fWidth * scale);
	sz.cy = static_cast<LONG>(fHeight * scale);

	return S_OK;
}