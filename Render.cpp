#include "harCs.h"
#include <chrono>


void Reset(std::shared_ptr<Renderer> renderer)
{

}

void D3DMenu::DrawGui(std::shared_ptr<Renderer> renderer, DWORD col)
{
	if (background == 5)					// solid
		col |= 0xff000000;
	else
		col |= background * 0x30000000;	// transparency
	renderer->SetDrawColor(col);
	renderer->AddFilledRect(XMFLOAT4(x - 3.f, y - 2.f, totwidth + 6.f, totheight + 4.f));
	renderer->SetDrawColor(col); //col | 0xff000000
	renderer->AddFilledRect(XMFLOAT4(x - 3.f, y - 2.f, totwidth + 6.f, titleheight + 1.f));
}

void Present(std::shared_ptr<Renderer> renderer)
{
	static Font font("Arial", 10, FW_BOLD);

	if (pMenu->Items.size() == 0)
		RebuildMenu();

	pMenu->Show(renderer, &font);
	pMenu->Nav();
	
}

using namespace std::chrono;
void FPSCheck(std::string& str)
{
	static high_resolution_clock::time_point lastTime;
	static int ticks = 0;

	auto now = high_resolution_clock::now();
	auto secs = duration_cast<seconds>(now - lastTime);
	ticks++;
	if (secs.count() >= 1)
	{
		str = std::to_string(ticks / secs.count()) + " FPS";
		ticks = 0;
		lastTime = now;
	}
}