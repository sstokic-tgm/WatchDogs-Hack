#include "harCs.h"

std::thread thr;
bool isRunning;
D3DMenu* pMenu;
std::string	sFPS = "xxx Fps";
// group states
int general = 1;
int hacks = 1;
// item  states
int background = 0;
int bEnableNoReload = 0;
int bEnableNoRecoil = 0;
int bEnableInfBat = 0;
int bEnableInfFoc = 0;
int bEnableInfinityItems = 0;
int bEnableBuildAStack = 0;
int bEnableVehicleGodmode = 0;

std::string opt_Grp[] = { "+", "-" };
std::string opt_OffOn[] = { "Off", "On" };
std::string opt_Back[] = { "Off", "20%", "40%", "60%", "80%", "Solid" };

WDFuncs funcs;

char g_path[MAX_PATH];
void setup(HMODULE hModule);


BOOL WINAPI DllMain(HINSTANCE mod, DWORD dwReason, LPVOID res)
{
	DisableThreadLibraryCalls(mod);

	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		
		setup(mod);
		std::async(std::launch::async, [](){thr = std::thread(InitThread); });

		break;

	case DLL_PROCESS_DETACH:

		isRunning = false;
		thr.join();

		break;
	}
	return TRUE;
}

void RebuildMenu()
{
	pMenu->AddGroup("General", general, opt_Grp);
	if (general)
	{
		pMenu->AddItem("Background", background, opt_Back, 6);
		pMenu->AddText("Framerate", sFPS);
	}

	pMenu->AddGroup("Hacks", hacks, opt_Grp);
	if (hacks)
	{
		pMenu->AddItem("No Reload", bEnableNoReload, opt_OffOn);
		pMenu->AddItem("No Recoil", bEnableNoRecoil, opt_OffOn);
		pMenu->AddItem("Infinity Battery", bEnableInfBat, opt_OffOn);
		pMenu->AddItem("Infinity Focus", bEnableInfFoc, opt_OffOn);
		pMenu->AddItem("Infinity Items", bEnableInfinityItems, opt_OffOn);
		pMenu->AddItem("Build a Stack", bEnableBuildAStack, opt_OffOn);
		pMenu->AddItem("Vehicle Godmode", bEnableVehicleGodmode, opt_OffOn);
	}
}

void InitThread()
{
	MODULEINFO modInfoDisrupt = { 0 };
	MODULEINFO modInfoWD = { 0 };
	HMODULE hDisrupt, hWD;

	isRunning = true;

	std::shared_ptr<D3DMenu> menu = std::make_shared<D3DMenu>(D3DMenu("XxharCs MultiHack", 180));
	menu->visible = 1;

	pMenu = menu.get();

	while (!InitHook())
	{
		std::this_thread::yield();
	}

	do
	{
		hDisrupt = GetModuleHandleA("Disrupt_b64.dll");
		hWD = GetModuleHandleA(0);

	} while (hDisrupt == NULL && hWD == NULL);
	
	GetModuleInformation(GetCurrentProcess(), hDisrupt, &modInfoDisrupt, sizeof(MODULEINFO));
	GetModuleInformation(GetCurrentProcess(), hWD, &modInfoWD, sizeof(MODULEINFO));
	
	print("==============================");
	print("[watch_dogs.exe]");
	print("Address: 0x%I64X", modInfoWD.lpBaseOfDll);
	print("Size: 0x%I64X", modInfoWD.SizeOfImage);
	print("EntryPoint: 0x%I64X", modInfoWD.EntryPoint);
	print("\n[Disrupt_b64.dll]");
	print("Address: 0x%I64X", modInfoDisrupt.lpBaseOfDll);
	print("Size: 0x%I64X", modInfoDisrupt.SizeOfImage);
	print("EntryPoint: 0x%I64X", modInfoDisrupt.EntryPoint);

	funcs.hDisruptHandle = (HMODULE)modInfoDisrupt.lpBaseOfDll;
	funcs.disruptSize = modInfoDisrupt.SizeOfImage;

	funcs.setupAddr();

	while (isRunning)
	{
		HackThread();

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	DeleteHook();
}

void HackThread()
{
	if (bEnableNoReload)
		funcs.EnableNoReload(true);
	else
		funcs.EnableNoReload(false);

	if (bEnableNoRecoil)
		funcs.EnableNoRecoil(true);
	else
		funcs.EnableNoRecoil(false);

	if (bEnableInfBat)
		funcs.EnableInfinityBattery(true);
	else
		funcs.EnableInfinityBattery(false);

	if (bEnableInfFoc)
		funcs.EnableInfinityFocus(true);
	else
		funcs.EnableInfinityFocus(false);

	if (bEnableInfinityItems)
		funcs.EnableInfinityItems(true);
	else
		funcs.EnableInfinityItems(false);

	if (bEnableBuildAStack)
		funcs.EnableBuildAStack(true);
	else
		funcs.EnableBuildAStack(false);

	if (bEnableVehicleGodmode)
		funcs.VehicleGodmode(true);
	else
		funcs.VehicleGodmode(false);
}

void __cdecl print(const char * format, ...)
{
	va_list argptr;

	char szBuff[1024];

	va_start(argptr, format);
	wvsprintf(szBuff, format, argptr);
	va_end(argptr);

	strcat_s(szBuff, "\n");

	char fileToWrite[MAX_PATH] = { 0 };

	strcpy_s(fileToWrite, g_path);
	strcat_s(fileToWrite, "XxharCs WatchDogs Hack.log");

	HANDLE hFile = CreateFile(fileToWrite, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	SetFilePointer(hFile, 0, 0, FILE_END);

	DWORD dummy = 0;
	WriteFile(hFile, szBuff, (DWORD)strlen(szBuff), &dummy, NULL);
	CloseHandle(hFile);
}

void setup(HMODULE hModule)
{
	memset(g_path, 0, MAX_PATH);

	GetModuleFileNameA(hModule, g_path, MAX_PATH);

	for (size_t i = strlen(g_path); i > 0; i--) {
		if (g_path[i] == '\\') {
			g_path[i + 1] = 0;
			break;
		}
	}
}