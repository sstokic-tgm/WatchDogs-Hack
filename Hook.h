#ifdef _WIN64
typedef unsigned __int64 UNDTYPE;
#else
typedef unsigned long UNDTYPE;
#endif

#ifndef SafeRelease
#define SafeRelease(var) if(var) {var->Release(); var = NULL;}
#endif

class HookClass
{
	BYTE data[14];
	FARPROC func;
	FARPROC hookFunc;
	bool bHooked;
	bool b64bitJump;
	DWORD dwOldProtect;

public:

	inline HookClass() : bHooked(false), func(NULL), hookFunc(NULL), b64bitJump(false), dwOldProtect(0)
	{
	}

	inline ~HookClass()
	{
		if (bHooked)
			Unhook();

		if (func && dwOldProtect)
			VirtualProtect((LPVOID)func, 14, dwOldProtect, &dwOldProtect);
	}

	inline bool Hook(FARPROC funcIn, FARPROC hookFuncIn)
	{
		if (bHooked)
		{
			if (funcIn == func)
			{
				if (hookFunc != hookFuncIn)
				{
					hookFunc = hookFuncIn;
					Rehook();
					return true;
				}
			}

			Unhook();
		}

		func = funcIn;
		hookFunc = hookFuncIn;

		if (!VirtualProtect((LPVOID)func, 14, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return false;

		memcpy(data, (const void*)func, 14);

		return true;
	}

	inline void Rehook(bool bForce = false)
	{
		if ((!bForce && bHooked) || !func)
			return;

		UNDTYPE startAddr = UNDTYPE(func);
		UNDTYPE targetAddr = UNDTYPE(hookFunc);
		UNDTYPE offset = targetAddr - (startAddr + 5);

#ifdef _WIN64
		b64bitJump = (offset > 0x7fff0000);

		if (b64bitJump)
		{
			LPBYTE addrData = (LPBYTE)func;
			*(addrData++) = 0xFF;
			*(addrData++) = 0x25;
			*((LPDWORD)(addrData)) = 0;
			*((UNDTYPE*)(addrData + 4)) = targetAddr;
		}
		else
#endif
		{
			LPBYTE addrData = (LPBYTE)func;
			*addrData = 0xE9;
			*(DWORD*)(addrData + 1) = DWORD(offset);
		}

		bHooked = true;
	}

	inline void Unhook()
	{
		if (!bHooked || !func)
			return;

		UINT count = b64bitJump ? 14 : 5;

		memcpy((void*)func, data, count);

		bHooked = false;
	}
};