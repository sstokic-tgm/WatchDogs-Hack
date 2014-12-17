#include "harCs.h"
#include <math.h>

//---------------------------------------------//
static HookClass hkNoReload;
static HookClass hkInfFocus;
static HookClass hkNoRecoil;
static HookClass hkBuildAStack;
//---------------------------------------------//
QWORD retNoReloadAddr;
QWORD retInfFocusAddr;
QWORD pJmpToNoRecoilAddr;
QWORD retBuildAStackAddr;
//---------------------------------------------//
QWORD pNoReloadAddr;
QWORD pNoRecoilAddr;
QWORD pInfBatAddr;
QWORD pInfFocAddr;
QWORD pBuildAStackAddr;
QWORD pInfItemAddr;
QWORD pVehicleGodmodeAddr;
QWORD pVehicleGodmodeAddr2;

bool WDFuncs::DataCompare(BYTE* pData, BYTE* bMask, char * szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
	if (*szMask == 'x' && *pData != *bMask)
		return FALSE;

	return (*szMask == NULL);
}

QWORD WDFuncs::FindPattern(QWORD dwAddress, DWORD_PTR dwLen, BYTE *bMask, char * szMask)
{
	for (DWORD_PTR i = 0; i<dwLen; i++)
		if (DataCompare((BYTE*)(dwAddress+i),bMask,szMask))  
			return (QWORD)(dwAddress+i);
	return 0;
}

int cHKnoReload = 0;
int cUHKnoReload = 0;
void WDFuncs::EnableNoReload(bool enable)
{
	cHKnoReload++;

	if (enable == true && cHKnoReload == 1)
	{
		//retNoReloadAddr = (QWORD)funcs.hDisruptHandle + 0x55C177;
		retNoReloadAddr = pNoReloadAddr + 0x6;

		hkNoReload.Hook((FARPROC)pNoReloadAddr, (FARPROC)noReload);
		hkNoReload.Rehook();

		cUHKnoReload++;
	}
	else if (enable == false && cUHKnoReload == 1)
	{
		hkNoReload.Unhook();
		cUHKnoReload = 0;
	}

	if (cUHKnoReload != 1)
	{
		cHKnoReload = 0;
		
	}
}

int cHKnoRecoil = 0;
int cUHKnoRecoil = 0;
void WDFuncs::EnableNoRecoil(bool enable)
{
	cHKnoRecoil++;

	if(enable == true && cHKnoRecoil == 1)
	{
		hkNoRecoil.Hook((FARPROC)pNoRecoilAddr, (FARPROC)noRecoil);
		hkNoRecoil.Rehook();

		cUHKnoRecoil++;
	}
	else if (enable == false && cUHKnoRecoil == 1)
	{
		hkNoRecoil.Unhook();
		cUHKnoRecoil = 0;
	}

	if (cUHKnoRecoil != 1)
	{
		cHKnoRecoil = 0;
	}
}

int cHKinfBat = 0;
int cUHKinfBat = 0;
void WDFuncs::EnableInfinityBattery(bool enable)
{
	DWORD dwOldProtect;
	BYTE origBytes[] = {0x0F, 0x42, 0xC2};
	BYTE hackBytes[] = {0x90, 0x90, 0x90};

	cHKinfBat++;
	
	if (enable == true && cHKinfBat == 1)
	{
		VirtualProtect((LPVOID)(pInfBatAddr + 0x7), sizeof(hackBytes), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		RtlMoveMemory((LPVOID)(pInfBatAddr + 0x7), hackBytes, sizeof(hackBytes));
		VirtualProtect((LPVOID)(pInfBatAddr + 0x7), sizeof(hackBytes), dwOldProtect, &dwOldProtect );

		cUHKinfBat++;
	}
	else if (enable == false && cUHKinfBat == 1)
	{
		VirtualProtect((LPVOID)(pInfBatAddr + 0x7), sizeof(origBytes), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		RtlMoveMemory((LPVOID)(pInfBatAddr + 0x7), origBytes, sizeof(origBytes));
		VirtualProtect((LPVOID)(pInfBatAddr + 0x7), sizeof(origBytes), dwOldProtect, &dwOldProtect );

		cUHKinfBat = 0;
	}

	if (cUHKinfBat != 1)
	{
		cHKinfBat = 0;
	}
}

int cHKinfFoc = 0;
int cUHKinfFoc = 0;
void WDFuncs::EnableInfinityFocus(bool enable)
{
	cHKinfFoc++;

	if (enable == true && cHKinfFoc == 1)
	{
		//retInfFocusAddr = (QWORD)funcs.hDisruptHandle + 0x2B873E;
		retInfFocusAddr = pInfFocAddr + 0xA;

		hkInfFocus.Hook((FARPROC)(pInfFocAddr + 0x2), (FARPROC)infFocus);
		hkInfFocus.Rehook();

		cUHKinfFoc++;
	}
	else if (enable == false && cUHKinfFoc == 1)
	{
		hkInfFocus.Unhook();
		cUHKinfFoc = 0;
	}

	if (cUHKinfFoc != 1)
	{
		cHKinfFoc = 0;
	}
}

int cHKbuildAstack = 0;
int cUHKbuildAstack = 0;
void WDFuncs::EnableBuildAStack(bool enable)
{
	cHKbuildAstack++;

	if (enable == true && cHKbuildAstack == 1)
	{
		retBuildAStackAddr = pBuildAStackAddr + 0x7;

		hkBuildAStack.Hook((FARPROC)pBuildAStackAddr, (FARPROC)buildAstack);
		hkBuildAStack.Rehook();

		cUHKbuildAstack++;
	}
	else if (enable == false && cUHKbuildAstack == 1)
	{
		hkBuildAStack.Unhook();
		cUHKbuildAstack = 0;
	}

	if (cUHKbuildAstack != 1)
	{
		cHKbuildAstack = 0;
	}
}

int cHKinfItems = 0;
int cUHKinfItems = 0;
void WDFuncs::EnableInfinityItems(bool enable)
{
	DWORD dwOldProtect;
	BYTE origBytes[] = {0x2B, 0xC6};
	BYTE hackBytes[] = {0x90, 0x90};

	cHKinfItems++;
	
	if (enable == true && cHKinfItems == 1)
	{
		VirtualProtect((LPVOID)(pInfItemAddr + 0x9), sizeof(hackBytes), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		RtlMoveMemory((LPVOID)(pInfItemAddr + 0x9), hackBytes, sizeof(hackBytes));
		VirtualProtect((LPVOID)(pInfItemAddr + 0x9), sizeof(hackBytes), dwOldProtect, &dwOldProtect);

		cUHKinfItems++;
	}
	else if (enable == false && cUHKinfItems == 1)
	{
		VirtualProtect((LPVOID)(pInfItemAddr + 0x9), sizeof(origBytes), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		RtlMoveMemory((LPVOID)(pInfItemAddr + 0x9), origBytes, sizeof(origBytes));
		VirtualProtect((LPVOID)(pInfItemAddr + 0x9), sizeof(origBytes), dwOldProtect, &dwOldProtect);

		cUHKinfItems = 0;
	}

	if (cUHKinfItems != 1)
	{
		cHKinfItems = 0;
	}
}

int cHKvehicleGodmode = 0;
int cUHKvehicleGodmode = 0;
void WDFuncs::VehicleGodmode(bool enable)
{
	DWORD dwOldProtect;
	BYTE origBytes[] = {0xF3, 0x0F, 0x5C, 0x4D, 0x5F};
	BYTE hackBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90};

	cHKvehicleGodmode++;
	
	if (enable == true && cHKvehicleGodmode == 1)
	{
		VirtualProtect((LPVOID)(pVehicleGodmodeAddr + 0x8), sizeof(hackBytes), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		RtlMoveMemory((LPVOID)(pVehicleGodmodeAddr + 0x8), hackBytes, sizeof(hackBytes));
		VirtualProtect((LPVOID)(pVehicleGodmodeAddr + 0x8), sizeof(hackBytes), dwOldProtect, &dwOldProtect );

		cUHKvehicleGodmode++;
	}
	else if (enable == false && cUHKvehicleGodmode == 1)
	{
		VirtualProtect((LPVOID)(pVehicleGodmodeAddr2 - 0x5), sizeof(origBytes), PAGE_EXECUTE_READWRITE, &dwOldProtect);
		RtlMoveMemory((LPVOID)(pVehicleGodmodeAddr2 - 0x5), origBytes, sizeof(origBytes));
		VirtualProtect((LPVOID)(pVehicleGodmodeAddr2 - 0x5), sizeof(origBytes), dwOldProtect, &dwOldProtect );

		cUHKvehicleGodmode = 0;
	}

	if (cUHKvehicleGodmode != 1)
	{
		cHKvehicleGodmode = 0;
	}
}

void WDFuncs::setupAddr()
{
	pNoReloadAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\xFF\x8F\x98\x00\x00\x00\x85\xC0", "xxxxxxxx");
	pNoRecoilAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x74\x00\x48\x8B\x47\x60\x48\x8D\x95\x90\x00\x00\x00", "x?xxxxxxxx???");
	pJmpToNoRecoilAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x48\x8B\x47\x60\x4D\x8B\xE6\x48\x8B\x50\x08\x48\x85\xD2\x74\x00", "xxxxxxxxxxxxxxx?");
	pInfBatAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x48\x89\x7C\x24\x00\x3B\xD0", "xxxx?xx");
	pInfFocAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x68\xC3\xF3\x0F\x10\x81\x88\x01\x00\x00", "xxxxxxxx??");
	pBuildAStackAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x44\x01\x6A\x0C\x8B\x42\x0C\x00\x00\x0F\x42\xF8", "xxxxxxx??xxx");
	pInfItemAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x83\xF8\xFF\x74\xE1\x3B\xC6\x00\x00\x00\x00", "xxxxxxx????");
	pVehicleGodmodeAddr = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\xF3\x0F\x10\x8E\xD8\x00\x00\x00\xF3\x0F\x5C\x4D\x5F\x0F\x2F\xCE", "xxxxxxxxxxxxxxxx");
	pVehicleGodmodeAddr2 = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x0F\x2F\xCE\xF3\x0F\x11\x8E\xD8\x00\x00\x00", "xxxxxxxxxxx");
	
	QWORD cDriverCountersComponentPlayer = FindPattern((QWORD)funcs.hDisruptHandle, funcs.disruptSize, (BYTE*)"\x48\x00\x00\x00\x00\x00\x00\x89\x0D\xC5\x1A\xEE\x00\x89\x0D\xC3\x1A\xEE\x00\x48\x8D\x0D\xE0\x59\x07\x00\xC7\x05\xC6\x1A\xEE\x00\x00\x80\x00\x00\x48\x89\x05\xB7\x1A\xEE\x00\x48\x83\xC4\x28\xE9\x00\x00\x00\x00", "x??????xxxxx?xxxxx?xxxxxx?xxxxx??x??xxxxxx?xxxxx????");

	print("\n[Addresses]");
	print("NoReload Address:			0x%I64X", pNoReloadAddr);
	print("NoRecoil Address:			0x%I64X", pNoRecoilAddr);
	print("Infinity Battery Address:	0x%I64X + 0x7", pInfBatAddr);
	print("Infinity Focus Address:		0x%I64X + 0x2", pInfFocAddr);
	print("Build a Stack Address:		0x%I64X", pBuildAStackAddr);
	print("Infinity Items Address:		0x%I64X + 0x9", pInfItemAddr);
	print("Vehicle Godmode Address:	0x%I64X + 0x8", pVehicleGodmodeAddr);
	print("cDriverCountersComponentPlayer: 0x%I64X", cDriverCountersComponentPlayer); // + 0x40 = godmode boolean
}