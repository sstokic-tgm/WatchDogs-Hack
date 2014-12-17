#ifndef __HARCS_H__
#define __HARCS_H__

#include <Windows.h>
#include <iostream>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <future>
#include <Psapi.h>
#include "D3DMenu.h"
#include "Hook.h"

typedef DWORD64 QWORD;

//GAME BASE
#define DISRUPT_BASE 0x7FED2F50000
//#define DISRUPT_BASE 0x7FF8309D0000
#define DISRUPT_BASE_RELOC(x) ((QWORD)funcs.hDisruptHandle + (x) - DISRUPT_BASE)

#define BATTERY_POWER_TO_BARS(p) ((int)((float)(p) * (1.0f / (397.7546387f / 6.0f))))

class WDFuncs
{
public:

	HMODULE hDisruptHandle;
	DWORD_PTR disruptSize;

	void EnableNoReload(bool enable);
	void EnableNoRecoil(bool enable);
	void EnableInfinityBattery(bool enable);
	void EnableInfinityFocus(bool enable);
	void EnableBuildAStack(bool enable);
	void EnableInfinityItems(bool enable);
	void VehicleGodmode(bool enable);
	void ctOSScan(bool enable);

	void setupAddr();

private:

	bool DataCompare(BYTE* pData, BYTE* bMask, char * szMask);
	QWORD FindPattern(QWORD dwAddress,QWORD dwLen,BYTE *bMask,char * szMask);
};

extern WDFuncs funcs;

extern "C" void noReload();
extern "C" void noRecoil();
extern "C" void infFocus();
extern "C" void buildAstack();

extern "C" QWORD retNoReloadAddr;
extern "C" QWORD retInfFocusAddr;
extern "C" QWORD pJmpToNoRecoilAddr;
extern "C" QWORD retBuildAStackAddr;

bool InitDXGICapture();
void FreeDXGICapture();

__forceinline bool InitHook()
{
	return InitDXGICapture();
}

__forceinline void DeleteHook()
{
	FreeDXGICapture();
}



extern int background;
extern int bEnableNoReload;
extern int bEnableNoRecoil;
extern int bEnableInfBat;
extern int bEnableInfFoc;
extern int bEnableInfinityItems;
extern int bEnableBuildAStack;
extern int bEnableVehicleGodmode;
extern bool isRunning;
extern D3DMenu* pMenu;

void InitThread();
void RebuildMenu();
void HackThread();
extern void __cdecl print(const char * format, ...);

#endif