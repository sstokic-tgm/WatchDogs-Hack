#include "D3DMenu.h"

D3DMenu::D3DMenu(std::string Name, float maxwidth)
{
	title = Name;
	cur = visible = 0;
	x = y = 15.f;
	totwidth = ofs = maxwidth;
	height = 15.f;
	titleheight = totheight = height + 4.f;
	col_title = MCOLOR_TITLE;
	col_group = MCOLOR_GROUP;
	col_text = MCOLOR_TEXT;
	col_off = MCOLOR_OFF;
	col_on = MCOLOR_ON;
	col_current = MCOLOR_CURRENT;
}
void D3DMenu::AddItem(std::string txt, int& var, std::string opt[], int maxval, int typ)
{
	MenuItem menu(typ, txt, opt, var, maxval);

	Items.push_back(menu);

	totheight = (Items.size() *height) + titleheight;
}

void D3DMenu::AddGroup(std::string txt, int &var, std::string opt[], int maxval)
{
	AddItem(txt, var, opt, maxval, MENUGROUP);
}

void D3DMenu::AddText(std::string txt, std::string& opt)
{
	int i;
	AddItem(txt, i, &opt, 0, MENUTEXT);
}

void D3DMenu::Show(std::shared_ptr<Renderer> renderer, Font* font)
{
	int	val;
	DWORD color;
	DWORD colorOnOf;

	if (!visible)
	{
		return;
	}

	float cy = y;

	DrawGui(renderer, 0x000000);
	renderer->SetDrawColor(col_title);
	renderer->AddText(font, x + totwidth / 2, cy + 1, title, FONT_CENTERED);

	cy += titleheight;

	for (unsigned int i = 0; i < Items.size(); i++)
	{
		auto& item = Items[i];
		if (item.typ == MENUTEXT)
		{
			renderer->SetDrawColor(col_text);
			renderer->AddText(font, x, cy, item.txt);

			if (item.opt)
				renderer->AddText(font, x + ofs, cy, item.opt[0], FONT_ALIGN_RIGHT);

		}
		else
		{
			val = (item.var) ? (item.var) : 0;

			if (i == cur)
				color = col_current;
			else if (item.typ == MENUGROUP)
				color = col_group;
			else
				color = col_text;//color = (val) ? col_on : col_off;

			renderer->SetDrawColor(color);
			renderer->AddText(font, x, cy, item.txt, color);

			if (item.opt)
			{
				renderer->SetDrawColor((val) ? col_on : col_off);
				renderer->AddText(font, x + ofs, cy, item.opt[val], FONT_ALIGN_RIGHT);
			}
				

		}
		cy += height;
	}
}

void D3DMenu::Nav(void)
{
	if (GetAsyncKeyState(VK_NUMPAD5) & 1)
	{
		visible = (!visible);
	}

	if (!visible)
		return;


	if (GetAsyncKeyState(VK_NUMPAD0))
	{
		if (GetAsyncKeyState(VK_NUMPAD8) & 1) y -= 10;
		if (GetAsyncKeyState(VK_NUMPAD2) & 1) y += 10;
		if (GetAsyncKeyState(VK_NUMPAD4) & 1) x -= 10;
		if (GetAsyncKeyState(VK_NUMPAD6) & 1) x += 10;
	}
	else
	{
		if (GetAsyncKeyState(VK_NUMPAD8) & 1)
		{
			do {
				cur--;
				if (cur<0)
					cur = Items.size() - 1;
			} while (Items[cur].typ == MENUTEXT);
		}
		else if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			do {
				cur++;
				if (cur == Items.size()) cur = 0;
			} while (Items[cur].typ == MENUTEXT);
		}
		else
		{
			int dir = 0;
			
			if (GetAsyncKeyState(VK_NUMPAD4) & 1 && Items[cur].var > 0) dir = -1;
			if (GetAsyncKeyState(VK_NUMPAD6) & 1 && Items[cur].var < (Items[cur].maxval - 1)) dir = 1;
			if (dir)
			{
				Items[cur].var += dir;
				if (Items[cur].typ == MENUGROUP)
				{
					Items.clear();
				}
			}
		}
	}
}