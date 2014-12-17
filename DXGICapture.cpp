#include <D3D11.h>
#include <Shlobj.h>
#include <Psapi.h>
#include "D3D11Renderer.h"
#include "harCs.h"

HookClass giswapResizeBuffers;
HookClass giswapPresent;

typedef HRESULT(STDMETHODCALLTYPE *DXGISwapResizeBuffersHookPROC)(IDXGISwapChain *swap, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT giFormat, UINT flags);
typedef HRESULT(WINAPI *D3D11CREATEPROC)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*, UINT, UINT, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, IUnknown**, D3D_FEATURE_LEVEL*, IUnknown**);
typedef HRESULT(STDMETHODCALLTYPE *DXGISwapPresentHookPROC)(IDXGISwapChain *swap, UINT syncInterval, UINT flags);

std::shared_ptr<Renderer> Ren;


bool HandleDXGIStuff(IDXGISwapChain *swap)
{
	ID3D11Device *device11 = nullptr;

	if (SUCCEEDED(swap->GetDevice(__uuidof(ID3D11Device), (void **)&device11)))
	{
		if (Ren == std::shared_ptr<Renderer>())
		{
			Ren.reset();
			Ren = std::shared_ptr<Renderer>(new D3D11Renderer(device11));
		}
		return true;
	}
	return false;
}

HRESULT STDMETHODCALLTYPE DXGISwapResizeBuffersHook(IDXGISwapChain *swap, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT giFormat, UINT flags)
{
	giswapResizeBuffers.Unhook();

	HRESULT hRes = swap->ResizeBuffers(bufferCount, width, height, giFormat, flags);

	giswapResizeBuffers.Rehook();

	return hRes;
}

HRESULT STDMETHODCALLTYPE DXGISwapPresentHook(IDXGISwapChain *swap, UINT syncInterval, UINT flags)
{
	giswapPresent.Unhook();


	FPSCheck(sFPS);

	if (HandleDXGIStuff(swap))
	{
		if (Ren->Begin(30))
		{
			Present(Ren);
			Ren->End();
		}
		Ren->Present();
	}

	HRESULT hRes = swap->Present(syncInterval, flags);

	giswapPresent.Rehook();

	return hRes;
}

IDXGISwapChain* CreateDummySwap()
{
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = 2;
	swapDesc.BufferDesc.Height = 2;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = GetForegroundWindow();
	swapDesc.SampleDesc.Count = 1;
	swapDesc.Windowed = TRUE;

	IDXGISwapChain *swap = NULL;
	IUnknown *device = NULL;

	HRESULT hErr;
	HMODULE hDll;

	hDll = GetModuleHandle("d3d11.dll");
	if (hDll)
	{
		D3D11CREATEPROC d3d11Create = (D3D11CREATEPROC)GetProcAddress(hDll, "D3D11CreateDeviceAndSwapChain");
		if (d3d11Create)
		{
			D3D_FEATURE_LEVEL desiredLevels[6] =
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_1,
			};
			D3D_FEATURE_LEVEL receivedLevel;

			IUnknown *context;
			hErr = (*d3d11Create)(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, desiredLevels, 6, D3D11_SDK_VERSION, &swapDesc, &swap, &device, &receivedLevel, &context);
			if (SUCCEEDED(hErr))
			{
				context->Release();
				device->Release();
				return swap;
			}
		}
	}

	return NULL;
}

bool InitDXGICapture()
{
	bool bSuccess = false;

	IDXGISwapChain *swap = CreateDummySwap();
	if (swap)
	{
		bSuccess = true;

		UNDTYPE *vtable = *(UNDTYPE**)swap;
		giswapPresent.Hook((FARPROC)*(vtable + (32 / 4)), (FARPROC)DXGISwapPresentHook);
		giswapResizeBuffers.Hook((FARPROC)*(vtable + (52 / 4)), (FARPROC)DXGISwapResizeBuffersHook);

		SafeRelease(swap);

		giswapPresent.Rehook();
		giswapResizeBuffers.Rehook();
	}

	return bSuccess;
}

void FreeDXGICapture()
{
	giswapPresent.Unhook();
	giswapResizeBuffers.Unhook();
	if (Ren != std::shared_ptr<Renderer>())
	{
		Ren.reset();
		Ren = std::shared_ptr<Renderer>();
	}
}