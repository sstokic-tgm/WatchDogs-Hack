#pragma once

#include <string>
#include <Windows.h>
#include <xnamath.h>
#include <memory>


#define FONT_ALIGN_LEFT 0
#define FONT_CENTERED 1
#define FONT_ALIGN_RIGHT 2

struct Font
{
public:
	std::string FontName;
	DWORD Size;
	DWORD Flags;

	Font(std::string fontName, DWORD dwHeight, DWORD dwFlags = 0)
		: FontName(fontName), Size(dwHeight), Flags(dwFlags)
	{}
};

class FontData
{
public:
	XMFLOAT4 m_fTexCoords[0x60];

	UINT32 m_TexWidth;
	UINT32 m_TexHeight;
	float m_Spacing;

	virtual ~FontData() {}

	static HRESULT CreateFontObjects(const Font font, FontData* data, std::unique_ptr<DWORD[]>& lpBitmapBits);
};



class Renderer
{
protected:
	DWORD m_Colour;
public:

	virtual ~Renderer() {}

	virtual void Init() {};

	virtual bool Begin(const int fps);
	virtual void End() {};
	virtual void Present() {};

	virtual void SetDrawColor(DWORD colour) { m_Colour = colour; }

	virtual void AddFilledRect(XMFLOAT4 rect) {};
	virtual void AddFilledLine(XMFLOAT4 rect) {};

	virtual HRESULT LoadFont(Font* font) { return E_FAIL; };
	virtual void FreeFont(Font* font) {};

	virtual HRESULT AddText(Font* font, float x, float y, const std::string& strText, DWORD dwFlag = 0) { return E_FAIL; };
	virtual	HRESULT AddText(Font* font, float x, float y, float scale, const std::string& strText, DWORD dwFlag = 0) { return E_FAIL; };

	HRESULT GetTextExtent(FontData* font, const std::string& strText, SIZE &sz, float scale = 1.f);
};

extern std::string sFPS;
void FPSCheck(std::string& str);
void Reset(std::shared_ptr<Renderer> renderer);
void Present(std::shared_ptr<Renderer> renderer);