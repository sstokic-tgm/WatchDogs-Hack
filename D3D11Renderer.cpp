#include "D3D11Renderer.h"
#include <d3dx11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dx11.lib")

D3D11Renderer::D3D11Renderer(ID3D11Device* device) : Renderer(), m_pDevice(device), alive(false), Buffer(), FontBuffer()
{
	m_pDevice->GetImmediateContext(&m_pContext);

	m_InputLayout = nullptr;
	m_fontInputLayout = nullptr;
	m_pshader = nullptr;
	m_vshader = nullptr;
	m_fontpshader = nullptr;
	m_fontvshader = nullptr;

	m_blendstate = nullptr;
	m_raststate = nullptr;
}

D3D11Renderer::~D3D11Renderer()
{
	for each (auto& var in Fonts)
	{
		FreeFont(var.first);
	}
}

#pragma region Shaders
const char pixshader[] = {
	"Texture2D tex2D;\n"
	"SamplerState linearSampler\n"
	"{\n"
	"	Filter = MIN_MAG_MIP_LINEAR;\n"
	"	AddressU = D3D11_TEXTURE_ADDRESS_BORDER;\n"
	"	AddressV = D3D11_TEXTURE_ADDRESS_BORDER;\n"
	"	BorderColor = float4(0.f, 0.f, 0.f, 1.f);\n"
	"};\n"
	"struct PS_INPUT\n"
	"{\n"
	"	float4 pos : SV_POSITION;\n"
	"	float4 col : COLOR;\n"
	"};\n"
	"float4 main( PS_INPUT input ) : SV_Target\n"
	"{\n"
	"	return input.col;\n"
	"};\n"
	"struct FPS_INPUT\n"
	"{\n"
	"	float4 pos : SV_POSITION;\n"
	"	float4 col : COLOR;\n"
	"	float2 tex : TEXCOORD;\n"
	"};\n"
	"float4 fontmain( FPS_INPUT input ) : SV_Target\n"
	"{\n"
	"	return tex2D.Sample( linearSampler, input.tex ) * input.col;\n"
	"};\n"
};

const char vertshader[] = {
	"struct VS_INPUT\n"
	"{\n"
	"	float4 pos : POSITION;\n"
	"	float4 col : COLOR;\n"
	"};\n"
	"struct PS_INPUT\n"
	"{\n"
	"	float4 pos : SV_POSITION;\n"
	"	float4 col : COLOR;\n"
	"};\n"
	"struct FVS_INPUT\n"
	"{\n"
	"	float4 pos : POSITION;\n"
	"	float4 col : COLOR;\n"
	"	float2 tex : TEXCOORD;\n"
	"};\n"
	"struct FPS_INPUT\n"
	"{\n"
	"	float4 pos : SV_POSITION;\n"
	"	float4 col : COLOR;\n"
	"	float2 tex : TEXCOORD;\n"
	"};\n"
	"float4 rearrange(float4 value)\n"
	"{\n"
	"	float4 color;\n"
	"	color.a = value.a;\n"
	"	color.r = value.b;\n"
	"	color.g = value.g;\n"
	"	color.b = value.r;\n"
	"	return color;\n"
	"};\n"
	"PS_INPUT main( VS_INPUT input )\n"
	"{\n"
	"	PS_INPUT output;\n"
	"	output.pos = input.pos;\n"
	"	output.col = rearrange(input.col);\n"
	"	return output;\n"
	"};\n"
	"FPS_INPUT fontmain( FVS_INPUT input )\n"
	"{\n"
	"	FPS_INPUT output;\n"
	"	output.pos = input.pos;\n"
	"	output.col = rearrange(input.col);\n"
	"	output.tex = input.tex;\n"
	"	return output;\n"
	"};\n"
};

inline HRESULT CompileShaderFromMemory(const char* szdata, SIZE_T len, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromMemory(szdata, len, NULL, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);

	if (FAILED(hr))
	{
		if (pErrorBlob)
			pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}
#pragma endregion

void D3D11Renderer::DestroyObjects()
{
	alive = false;
	SafeRelease(m_InputLayout);
	SafeRelease(m_pshader);
	SafeRelease(m_vshader);
	SafeRelease(m_fontpshader);
	SafeRelease(m_fontvshader);

	SafeRelease(m_blendstate);
	SafeRelease(m_raststate);
}

void D3D11Renderer::Init()
{
	alive = false;
	
	ID3DBlob* pVSBlob = NULL;

	if (FAILED(CompileShaderFromMemory(vertshader, sizeof(vertshader), "main", "vs_4_0", &pVSBlob)))
	{
		DestroyObjects();
		return;
	}

	if (FAILED(m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_vshader)))
	{
		DestroyObjects();
		pVSBlob->Release();
		return;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
		
	if (FAILED(m_pDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_InputLayout)))
	{
		SafeRelease(pVSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pVSBlob);

	if (FAILED(CompileShaderFromMemory(vertshader, sizeof(vertshader), "fontmain", "vs_4_0", &pVSBlob)))
	{
		DestroyObjects();
		return;
	}

	if (FAILED(m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_fontvshader)))
	{
		DestroyObjects();
		pVSBlob->Release();
		return;
	}

	if (FAILED(m_pDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_fontInputLayout)))
	{
		SafeRelease(pVSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pVSBlob);

	ID3DBlob* pPSBlob = NULL;

	if (FAILED(CompileShaderFromMemory(pixshader, sizeof(pixshader), "main", "ps_4_0", &pPSBlob)))
	{
		DestroyObjects();
		return;
	}

	// Create the pixel shader
	if (FAILED(m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pshader)))
	{
		SafeRelease(pPSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pPSBlob);

	if (FAILED(CompileShaderFromMemory(pixshader, sizeof(pixshader), "fontmain", "ps_4_0", &pPSBlob)))
	{
		DestroyObjects();
		return;
	}

	// Create the pixel shader
	if (FAILED(m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_fontpshader)))
	{
		SafeRelease(pPSBlob);
		DestroyObjects();
		return;
	}
	SafeRelease(pPSBlob);

	D3D11_BLEND_DESC blenddesc;
	blenddesc.AlphaToCoverageEnable = FALSE;
	blenddesc.IndependentBlendEnable = FALSE;
	blenddesc.RenderTarget[0].BlendEnable = TRUE;
	blenddesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blenddesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blenddesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blenddesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	if (FAILED(m_pDevice->CreateBlendState(&blenddesc, &m_blendstate)))
	{
		DestroyObjects();
		return;
	}


	D3D11_RASTERIZER_DESC rastdesc = CD3D11_RASTERIZER_DESC(D3D11_FILL_SOLID, D3D11_CULL_NONE, false, 0, 0.f, 0.f, false, false, false, false);

	if (FAILED(m_pDevice->CreateRasterizerState(&rastdesc, &m_raststate)))
	{
		DestroyObjects();
		return;
	}

	alive = true;

	D3D11_VIEWPORT vp;
	UINT numViewports = 1;
	m_pContext->RSGetViewports(&numViewports, &vp);

	width = vp.Width;
	height = vp.Height;

	alive = true;
}

bool D3D11Renderer::Begin(const int fps)
{
	if (!Renderer::Begin(fps))
		return false;
	if (FAILED(Buffer.Begin(m_pDevice)))
		return false;

	if (FAILED(FontBuffer.Begin(m_pDevice)))
	{
		Buffer.End();
		return false;
	}

	for each (auto& var in Fonts)
	{
		Fonts[var.first]->vertices.clear();
	}

	if (!alive)
		Init();

	return true;
}

void D3D11Renderer::End()
{
	Buffer.End();

	for each (auto& var in Fonts)
	{
		FontBuffer.Add(var.second->vertices);
	}
	
	FontBuffer.End();
}

void D3D11Renderer::Present()
{
	if (!alive)
		return;

	// Save current state
	m_pContext->OMGetBlendState(&m_pUILastBlendState, m_LastBlendFactor, &m_LastBlendMask);
	m_pContext->RSGetState(&m_pUILastRasterizerState);
	m_pContext->OMGetDepthStencilState(&m_LastDepthState, &m_LastStencilRef);
	m_pContext->IAGetInputLayout(&m_LastInputLayout);
	m_pContext->IAGetPrimitiveTopology(&m_LastTopology);
	m_pContext->IAGetVertexBuffers(0, 8, m_LastBuffers, m_LastStrides, m_LastOffsets);
	m_pContext->PSGetShader(&m_LastPSShader, NULL, 0);
	m_pContext->GSGetShader(&m_LastGSShader, NULL, 0);
	m_pContext->VSGetShader(&m_LastVSShader, NULL, 0);

	m_pContext->RSSetState(m_raststate);
	m_pContext->IASetInputLayout(m_InputLayout); 

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pContext->OMSetBlendState(m_blendstate, NULL, 0xFFFFFFFF);
	m_pContext->IASetVertexBuffers(0, 1, &Buffer.GetBuffer(), &stride, &offset);
	m_pContext->PSSetShader(m_pshader, NULL, 0);
	m_pContext->VSSetShader(m_vshader, NULL, 0);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->Draw(Buffer.GetNumVertices(), 0);

	stride = sizeof(FontVertex);
	offset = 0;
	m_pContext->IASetInputLayout(m_fontInputLayout);
	m_pContext->IASetVertexBuffers(0, 1, &FontBuffer.GetBuffer(), &stride, &offset);
	m_pContext->PSSetShader(m_fontpshader, NULL, 0);
	m_pContext->VSSetShader(m_fontvshader, NULL, 0);

	UINT currentIndex = 0;
	for each (auto& var in Fonts)
	{
		auto& data = var.second;
		auto size = data->vertices.size();
		m_pContext->PSSetShaderResources(0, 1, &data->m_Texture);
		m_pContext->Draw(size, currentIndex);
		currentIndex += size;
	}

	m_pContext->OMSetBlendState(m_pUILastBlendState, m_LastBlendFactor, m_LastBlendMask);
	m_pContext->RSSetState(m_pUILastRasterizerState);
	m_pContext->OMSetDepthStencilState(m_LastDepthState, m_LastStencilRef);
	m_pContext->IASetInputLayout(m_LastInputLayout);
	m_pContext->IASetPrimitiveTopology(m_LastTopology);
	m_pContext->IASetVertexBuffers(0, 8, m_LastBuffers, m_LastStrides, m_LastOffsets);
	m_pContext->PSSetShader(m_LastPSShader, NULL, 0);
	m_pContext->GSSetShader(m_LastGSShader, NULL, 0);
	m_pContext->VSSetShader(m_LastVSShader, NULL, 0);
}

void D3D11Renderer::AddFilledRect(XMFLOAT4 rect)
{
	float scalex = 1 / width * 2.f;
	float scaley = 1 / height * 2.f;

	rect.z = (rect.x + rect.z) * scalex - 1.f;
	rect.w = 1.f - (rect.y + rect.w) * scaley;
	rect.x = rect.x * scalex - 1.f;
	rect.y = 1.f - rect.y * scaley;

	Vertex v[6] =
	{
		{ XMFLOAT3(rect.x, rect.w, 0.5f), m_Colour },
		{ XMFLOAT3(rect.x, rect.y, 0.5f), m_Colour },
		{ XMFLOAT3(rect.z, rect.w, 0.5f), m_Colour },
		{ XMFLOAT3(rect.z, rect.y, 0.5f), m_Colour },
		{ XMFLOAT3(rect.z, rect.w, 0.5f), m_Colour },
		{ XMFLOAT3(rect.x, rect.y, 0.5f), m_Colour }
	};
	Buffer.Add(v, ARRAYSIZE(v));
}

void D3D11Renderer::AddFilledLine(XMFLOAT4 rect)
{
	float scalex = 1 / width * 2.f;
	float scaley = 1 / height * 2.f;

	auto rec1 = XMFLOAT3(rect.x * scalex - 1.f, 1.f - rect.y * scaley, 1.f - (rect.y + 1) * scaley);
	auto rec2 = XMFLOAT3(rect.z * scalex - 1.f, 1.f - rect.w * scaley, 1.f - (rect.w + 1) * scaley);
	Vertex v[6] =
	{
		{ XMFLOAT3(rec2.x, rec2.y, 0.5f), m_Colour },
		{ XMFLOAT3(rec1.x, rec1.z, 0.5f), m_Colour },
		{ XMFLOAT3(rec1.x, rec1.y, 0.5f), m_Colour },
				 
		{ XMFLOAT3(rec2.x, rec2.y, 0.5f), m_Colour },
		{ XMFLOAT3(rec2.x, rec2.z, 0.5f), m_Colour },
		{ XMFLOAT3(rec1.x, rec1.y, 0.5f), m_Colour },
	};
	Buffer.Add(v, ARRAYSIZE(v));
}

bool D3D11Renderer::containsFont(Font* font)
{
	return (Fonts.find(font) != Fonts.end());
}

HRESULT D3D11Renderer::LoadFont(Font* font)
{
	HRESULT hr = S_OK;
	if (containsFont(font))
		return S_OK;

	D3D11FontData* data(new D3D11FontData());
	std::unique_ptr<DWORD[]> bitmap;

	if (FAILED(hr = D3D11FontData::CreateFontObjects(*font, data, bitmap)))
		return hr;


	ID3D11Texture2D* buftex;
	D3D11_TEXTURE2D_DESC texdesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UNORM, data->m_TexWidth, data->m_TexHeight, 1, 1, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	if (FAILED(hr = m_pDevice->CreateTexture2D(&texdesc, nullptr, &buftex)))
	{
		return hr;
	}

	D3D11_MAPPED_SUBRESOURCE texmap;
	if (FAILED(hr = m_pContext->Map(buftex, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
	{
		SafeRelease(buftex);
		return hr;
	}

	BYTE bAlpha;
	DWORD* pDst32;
	BYTE* pDstRow = (BYTE*)texmap.pData;

	for (UINT32 y = 0; y < data->m_TexHeight; y++)
	{
		pDst32 = (DWORD*)pDstRow;
		for (UINT32 x = 0; x < data->m_TexWidth; x++)
		{
			bAlpha = BYTE((bitmap[data->m_TexWidth * y + x] & 0xFF) >> 4);
			if (bAlpha > 0)
				*pDst32++ = ((bAlpha * 0x11) << 24) | 0xFFFFFF;
			else
				*pDst32++ = 0x00000000;
		}
		pDstRow += texmap.RowPitch;
	}

	m_pContext->Unmap(buftex, 0);

	if (FAILED(hr = m_pDevice->CreateShaderResourceView(buftex, nullptr, &data->m_Texture)))
	{
		SafeRelease(buftex);
		return hr;
	}

	SafeRelease(buftex);

	Fonts.insert(std::pair<Font*, D3D11FontData*>(font, data));
	return S_OK;
}

void D3D11Renderer::FreeFont(Font* font)
{
	if (containsFont(font))
		return;

	auto data = Fonts[font];
	SafeRelease(data->m_Texture);

	Fonts.erase(font);

	delete data;
}

HRESULT D3D11Renderer::AddText(Font* font, float x, float y, const std::string& strText, DWORD dwFlag)
{
	return AddText(font, x, y, 1.f, strText, dwFlag);
}

HRESULT D3D11Renderer::AddText(Font* font, float x, float y, float scale, const std::string& strText, DWORD flag)
{
	HRESULT hr = S_OK;
	if (FAILED(hr = LoadFont(font)))
		return hr;

	float scalex = 1 / (float)width * 2.f;
	float scaley = 1 / (float)height * 2.f;

	XMFLOAT4A loc(x * scalex - 1.f, 1.f - y * scaley, 0.f, 0.f);

	auto& data = Fonts[font];

	if (flag != FONT_ALIGN_LEFT)
	{
		if (flag == FONT_ALIGN_RIGHT)
		{
			SIZE sz;
			GetTextExtent(data, strText, sz, scale);
			loc.x -= static_cast<float>(sz.cx) * scalex;
		}
		else if (flag == FONT_CENTERED)
		{
			SIZE sz;
			GetTextExtent(data, strText, sz, scale);
			loc.x -= static_cast<float>(sz.cx) / 2.0f * scalex;
		}
	}

	float fStartX = loc.x;

	for each(auto c in strText)
	{
		if (c < 32 || c >= 128)
		{
			if (c == '\n')
			{
				loc.x = fStartX;
				loc.y += (data->m_fTexCoords[c - 32].y - data->m_fTexCoords[c - 32].w) * data->m_TexHeight * scaley * scale;
			}
			else
				continue;
		}

		c -= 32;

		loc.z = loc.x + ((data->m_fTexCoords[c].z - data->m_fTexCoords[c].x) * data->m_TexWidth * scalex * scale);
		loc.w = loc.y + ((data->m_fTexCoords[c].y - data->m_fTexCoords[c].w) * data->m_TexHeight * scaley * scale);

		if (c != 0)
		{

			FontVertex v[6];
			v[0] = { { XMFLOAT3(loc.x, loc.w, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].x, data->m_fTexCoords[c].w) };
			v[1] = { { XMFLOAT3(loc.x, loc.y, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].x, data->m_fTexCoords[c].y) };
			v[2] = { { XMFLOAT3(loc.z, loc.w, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].z, data->m_fTexCoords[c].w) };
			v[3] = { { XMFLOAT3(loc.z, loc.y, 0.5f), m_Colour }, XMFLOAT2(data->m_fTexCoords[c].z, data->m_fTexCoords[c].y) };
			v[4] = v[2];
			v[5] = v[1];

			for each (auto& vertex in v)
			{
				data->vertices.push_back(vertex);
			}
		}
		loc.x += (loc.z - loc.x);
	}

	return S_OK;
}
