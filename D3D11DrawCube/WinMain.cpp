#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <tchar.h>

using namespace DirectX;

bool bIsExitRequested = false;

ATOM aMainWindow = 0;
HWND hMainWindow = 0;

ID3D11Device *pD3D11Device = NULL;
ID3D11DeviceContext *pD3D11DeviceCtx = NULL;
IDXGISwapChain *pDXGISwapChain = NULL;
ID3D11RenderTargetView *pD3D11RTV = NULL;
ID3D11Texture2D *pD3D11DepthStencilTexture2D = NULL;
ID3D11DepthStencilView *pD3D11DSV = NULL;
ID3D11RasterizerState *pD3D11SolidRS = NULL;
ID3D11RasterizerState *pD3D11WireframeRS = NULL;
ID3D11SamplerState *pD3D11SamplerState = NULL;
ID3D11Buffer *pD3D11VertexBuffer = NULL;
ID3D11Buffer *pD3D11IndexBuffer = NULL;
ID3DBlob *pD3D11VertexShaderBlob = NULL;
ID3DBlob *pD3D11PixelShaderBlob = NULL;
ID3D11VertexShader *pD3D11VertexShader = NULL;
ID3D11PixelShader *pD3D11PixelShader = NULL;
ID3D11InputLayout *pD3D11InputLayout = NULL;
ID3D11Buffer *pD3D11VertexShaderConstantBuffer = NULL;
const FLOAT afRTVColor[]{ 0.0f, 0.0f, 0.0f, 1.0f };

struct Vertex
{
	float x, y, z;
};

Vertex aVertices[] =
{
	{-1.0f, -1.0f, -1.0f},
	{-1.0f, +1.0f, -1.0f},
	{+1.0f, +1.0f, -1.0f},
	{+1.0f, -1.0f, -1.0f},
	{-1.0f, -1.0f, +1.0f},
	{-1.0f, +1.0f, +1.0f},
	{+1.0f, +1.0f, +1.0f},
	{+1.0f, -1.0f, +1.0f}
};

// cube indices
UINT aIndices[] = { 0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7 };

LRESULT WINAPI WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	int iErrorLevel = 0;

	WNDCLASSEX wndCls{};
	wndCls.cbSize = sizeof(WNDCLASSEX);
	wndCls.style = CS_HREDRAW | CS_VREDRAW;
	wndCls.lpfnWndProc = WindowProc;
	wndCls.hIcon = LoadIcon(GetModuleHandle(NULL), IDI_WINLOGO);
	wndCls.hIconSm = LoadIcon(GetModuleHandle(NULL), IDI_WINLOGO);
	wndCls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndCls.hInstance = GetModuleHandle(NULL);
	wndCls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndCls.lpszClassName = TEXT("TheVoidClass");

	aMainWindow = RegisterClassEx(&wndCls);
	if (!aMainWindow)
	{
		iErrorLevel = GetLastError();
	}

	DWORD dwWndStyle = 0;
	RECT rWndRect{};
	if (!iErrorLevel)
	{
		dwWndStyle = WS_OVERLAPPEDWINDOW;

		rWndRect.left = 256;
		rWndRect.top = 256;
		rWndRect.right = rWndRect.left + 1280;
		rWndRect.bottom = rWndRect.top + 720;

		if (!AdjustWindowRectEx(&rWndRect, dwWndStyle, 0, 0))
		{
			iErrorLevel = GetLastError();
		}
	}

	if (!iErrorLevel)
	{
		hMainWindow = CreateWindowEx(
			0,
			(TCHAR *)((ULONG_PTR)((WORD)(aMainWindow))),
			TEXT("D3D11DrawTriangle"),
			dwWndStyle,
			rWndRect.left,
			rWndRect.top,
			rWndRect.right - rWndRect.left,
			rWndRect.bottom - rWndRect.top,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL);

		if (!hMainWindow)
		{
			iErrorLevel = GetLastError();
		}
	}

	if (!iErrorLevel)
	{
		if (!ShowWindow(hMainWindow, SW_SHOW))
		{
			iErrorLevel = GetLastError();
		}
	}

	if (!iErrorLevel)
	{
		if (!UpdateWindow(hMainWindow))
		{
			iErrorLevel = GetLastError();
		}
	}

	if (!iErrorLevel)
	{
		if (!SetForegroundWindow(hMainWindow))
		{
			iErrorLevel = GetLastError();
		}
	}

	UINT uWndWidth = 1280;
	UINT uWndHeight = 720;

	DXGI_MODE_DESC dxgiModeDesc{};
	dxgiModeDesc.Width = uWndWidth;
	dxgiModeDesc.Height = uWndHeight;
	dxgiModeDesc.RefreshRate.Numerator = 60;
	dxgiModeDesc.RefreshRate.Denominator = 1;
	dxgiModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SWAP_CHAIN_DESC dxgiSwChDesc{};
	dxgiSwChDesc.BufferDesc = dxgiModeDesc;
	dxgiSwChDesc.SampleDesc.Count = 1;
	dxgiSwChDesc.SampleDesc.Quality = 0;
	dxgiSwChDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwChDesc.BufferCount = 1;
	dxgiSwChDesc.OutputWindow = hMainWindow;
	dxgiSwChDesc.Windowed = TRUE;
	dxgiSwChDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgiSwChDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#ifdef _DEBUG
	UINT uD3D11Flags = (UINT)D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT uD3D11Flags = 0;
#endif

	HRESULT hrD3D11Result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		uD3D11Flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&dxgiSwChDesc,
		&pDXGISwapChain,
		&pD3D11Device,
		nullptr,
		&pD3D11DeviceCtx);
	if (FAILED(hrD3D11Result))
	{
		iErrorLevel = hrD3D11Result;
	}

	ID3D11Texture2D *d3d11BackgroundTexture = NULL;
	if (!iErrorLevel)
	{
		hrD3D11Result = pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&d3d11BackgroundTexture);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
	}

	if (!iErrorLevel)
	{
		hrD3D11Result = pD3D11Device->CreateRenderTargetView(d3d11BackgroundTexture, NULL, &pD3D11RTV);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->OMSetRenderTargets(1, &pD3D11RTV, NULL);
		}
	}

	if (d3d11BackgroundTexture)
	{
		d3d11BackgroundTexture->Release();
		d3d11BackgroundTexture = NULL;
	}

	if (!iErrorLevel)
	{
		D3D11_TEXTURE2D_DESC d3d11Texture2DDesc{};
		d3d11Texture2DDesc.Width = uWndWidth;
		d3d11Texture2DDesc.Height = uWndHeight;
		d3d11Texture2DDesc.MipLevels = 1;
		d3d11Texture2DDesc.ArraySize = 1;
		d3d11Texture2DDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3d11Texture2DDesc.SampleDesc.Count = 1;
		d3d11Texture2DDesc.SampleDesc.Quality = 0;
		d3d11Texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
		d3d11Texture2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		d3d11Texture2DDesc.CPUAccessFlags = 0;
		d3d11Texture2DDesc.MiscFlags = 0;

		hrD3D11Result = pD3D11Device->CreateTexture2D(&d3d11Texture2DDesc, NULL, &pD3D11DepthStencilTexture2D);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
	}

	if (!iErrorLevel)
	{
		hrD3D11Result = pD3D11Device->CreateDepthStencilView(pD3D11DepthStencilTexture2D, NULL, &pD3D11DSV);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->OMSetRenderTargets(1, &pD3D11RTV, pD3D11DSV);

			D3D11_VIEWPORT d3d11VP{};
			d3d11VP.TopLeftX = 0;
			d3d11VP.TopLeftY = 0;
			d3d11VP.Width = (FLOAT)uWndWidth;
			d3d11VP.Height = (FLOAT)uWndHeight;
			d3d11VP.MinDepth = 0.0f;
			d3d11VP.MaxDepth = 1.0f;

			pD3D11DeviceCtx->RSSetViewports(1, &d3d11VP);

			pD3D11DeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}

	if (!iErrorLevel)
	{
		D3D11_RASTERIZER_DESC d3d11SolidRasterizerDesc{};
		d3d11SolidRasterizerDesc.FillMode = D3D11_FILL_SOLID;
		d3d11SolidRasterizerDesc.CullMode = D3D11_CULL_BACK;
		d3d11SolidRasterizerDesc.MultisampleEnable = FALSE;
		d3d11SolidRasterizerDesc.AntialiasedLineEnable = FALSE;
		d3d11SolidRasterizerDesc.DepthClipEnable = TRUE;

		hrD3D11Result = pD3D11Device->CreateRasterizerState(&d3d11SolidRasterizerDesc, &pD3D11SolidRS);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->RSSetState(pD3D11SolidRS);
		}
	}

	if (!iErrorLevel)
	{
		D3D11_RASTERIZER_DESC d3d11WireframeRasterizerDesc{};
		d3d11WireframeRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		d3d11WireframeRasterizerDesc.CullMode = D3D11_CULL_NONE;
		d3d11WireframeRasterizerDesc.MultisampleEnable = FALSE;
		d3d11WireframeRasterizerDesc.AntialiasedLineEnable = FALSE;
		d3d11WireframeRasterizerDesc.DepthClipEnable = TRUE;

		hrD3D11Result = pD3D11Device->CreateRasterizerState(&d3d11WireframeRasterizerDesc, &pD3D11WireframeRS);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->RSSetState(pD3D11WireframeRS);
		}
	}

	if (!iErrorLevel)
	{
		CD3D11_DEFAULT cd3d11Default{};
		CD3D11_SAMPLER_DESC cd3d11SaplerDesc = CD3D11_SAMPLER_DESC(cd3d11Default);
		D3D11_SAMPLER_DESC d3d11SamplerDesc = cd3d11SaplerDesc;

		hrD3D11Result = pD3D11Device->CreateSamplerState(&d3d11SamplerDesc, &pD3D11SamplerState);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->PSSetSamplers(0, 1, &pD3D11SamplerState);
		}
	}

	if (!iErrorLevel)
	{
		D3D11_BUFFER_DESC d3d11BufferDesc{};
		d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;
		d3d11BufferDesc.ByteWidth = (UINT)sizeof(aVertices);
		d3d11BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		d3d11BufferDesc.MiscFlags = 0;
		d3d11BufferDesc.StructureByteStride = (UINT)sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA d3d11SubresourceData{};
		d3d11SubresourceData.pSysMem = aVertices;

		hrD3D11Result = pD3D11Device->CreateBuffer(&d3d11BufferDesc, &d3d11SubresourceData, &pD3D11VertexBuffer);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			UINT uiStride = sizeof(Vertex);
			UINT uiOffset = 0;
			pD3D11DeviceCtx->IASetVertexBuffers(0, 1, &pD3D11VertexBuffer, &uiStride, &uiOffset);
		}
	}

	if (!iErrorLevel)
	{
		D3D11_BUFFER_DESC d3d11BufferDesc{};
		d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;
		d3d11BufferDesc.ByteWidth = (UINT)sizeof(aIndices);
		d3d11BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		d3d11BufferDesc.MiscFlags = 0;
		d3d11BufferDesc.StructureByteStride = (UINT)sizeof(UINT);

		D3D11_SUBRESOURCE_DATA d3d11SubresourceData{};
		d3d11SubresourceData.pSysMem = aIndices;

		hrD3D11Result = pD3D11Device->CreateBuffer(&d3d11BufferDesc, &d3d11SubresourceData, &pD3D11IndexBuffer);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->IASetIndexBuffer(pD3D11IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		}
	}

	if (!iErrorLevel)
	{
		ID3DBlob *pError = NULL;
		hrD3D11Result = D3DCompileFromFile(
			L"Shader.hlsl",
			NULL,
			0,
			"VS",
			"vs_5_0",
			0,
			0,
			&pD3D11VertexShaderBlob,
			&pError);

		if (FAILED(hrD3D11Result))
		{
			if (pError)
			{
				pError->Release();
			}

			iErrorLevel = hrD3D11Result;
		}
	}

	if (!iErrorLevel)
	{
		ID3DBlob *pError = NULL;
		hrD3D11Result = D3DCompileFromFile(
			L"Shader.hlsl",
			NULL,
			0,
			"PS",
			"ps_5_0",
			0,
			0,
			&pD3D11PixelShaderBlob,
			&pError);

		if (FAILED(hrD3D11Result))
		{
			if (pError)
			{
				pError->Release();
			}

			iErrorLevel = hrD3D11Result;
		}
	}

	if (!iErrorLevel)
	{
		hrD3D11Result = pD3D11Device->CreateVertexShader(
			pD3D11VertexShaderBlob->GetBufferPointer(),
			pD3D11VertexShaderBlob->GetBufferSize(),
			NULL,
			&pD3D11VertexShader);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
	}

	if (!iErrorLevel)
	{
		hrD3D11Result = pD3D11Device->CreatePixelShader(
			pD3D11PixelShaderBlob->GetBufferPointer(),
			pD3D11PixelShaderBlob->GetBufferSize(),
			NULL,
			&pD3D11PixelShader);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
	}

	if (!iErrorLevel)
	{
		pD3D11DeviceCtx->VSSetShader(pD3D11VertexShader, NULL, 0);

		D3D11_INPUT_ELEMENT_DESC d3d11InputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		if (pD3D11InputLayout)
		{
			pD3D11InputLayout->Release();
			pD3D11InputLayout = NULL;
		}

		hrD3D11Result = pD3D11Device->CreateInputLayout(
			d3d11InputElementDesc,
			sizeof(d3d11InputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
			pD3D11VertexShaderBlob->GetBufferPointer(),
			pD3D11VertexShaderBlob->GetBufferSize(),
			&pD3D11InputLayout);

		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->IASetInputLayout(pD3D11InputLayout);
		}		
	}

	if (!iErrorLevel)
	{
		pD3D11DeviceCtx->PSSetShader(pD3D11PixelShader, NULL, 0);
	}

	XMMATRIX xmmWorld = XMMatrixTranslationFromVector(XMVectorSet(5.0f, 0.0f, 0.0f, 0.0f));
	XMMATRIX xmmView = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	XMMATRIX xmmProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), 1280.0f / 720.0f, 1.0f, 1000.0f);
	XMMATRIX xmmWVP = XMMatrixMultiply(xmmWorld, XMMatrixMultiply(xmmView, xmmProjection));

	if (!iErrorLevel)
	{
		D3D11_BUFFER_DESC d3d11BufferDesc{};
		d3d11BufferDesc.Usage = D3D11_USAGE_DEFAULT;
		d3d11BufferDesc.ByteWidth = (UINT)sizeof(XMMATRIX);
		d3d11BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		d3d11BufferDesc.MiscFlags = 0;
		d3d11BufferDesc.StructureByteStride = (UINT)sizeof(XMMATRIX);

		D3D11_SUBRESOURCE_DATA d3d11SubresourceData{};
		d3d11SubresourceData.pSysMem = &xmmWVP;

		hrD3D11Result = pD3D11Device->CreateBuffer(&d3d11BufferDesc, &d3d11SubresourceData, &pD3D11VertexShaderConstantBuffer);
		if (FAILED(hrD3D11Result))
		{
			iErrorLevel = hrD3D11Result;
		}
		else
		{
			pD3D11DeviceCtx->VSSetConstantBuffers(0, 1, &pD3D11VertexShaderConstantBuffer);
		}
	}

	while (!bIsExitRequested && !iErrorLevel)
	{
		MSG Msg{};
		if (!PeekMessageW(&Msg, hMainWindow, 0, 0, PM_REMOVE))
		{

		}

		if (!iErrorLevel)
		{
			if (Msg.message == WM_QUIT)
			{

			}
			else
			{
				if (!TranslateMessage(&Msg))
				{

				}

				if (FAILED(DispatchMessageW(&Msg)))
				{
					iErrorLevel = GetLastError();
				}
			}
		}

		if (!iErrorLevel)
		{
			pD3D11DeviceCtx->ClearRenderTargetView(pD3D11RTV, afRTVColor);
			pD3D11DeviceCtx->ClearDepthStencilView(pD3D11DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			pD3D11DeviceCtx->DrawIndexed(UINT(sizeof(aIndices) / sizeof(UINT)), 0, 0);

			hrD3D11Result = pDXGISwapChain->Present(1, 0);
			if (FAILED(hrD3D11Result))
			{
				iErrorLevel = hrD3D11Result;
			}
		}
	}

	if (pD3D11VertexShaderConstantBuffer)
	{
		pD3D11VertexShaderConstantBuffer->Release();
		pD3D11VertexShaderConstantBuffer = NULL;
	}

	if (pD3D11InputLayout)
	{
		pD3D11InputLayout->Release();
		pD3D11InputLayout = NULL;
	}

	if (pD3D11PixelShader)
	{
		pD3D11PixelShader->Release();
		pD3D11PixelShader = NULL;
	}

	if (pD3D11VertexShader)
	{
		pD3D11VertexShader->Release();
		pD3D11VertexShader = NULL;
	}

	if (pD3D11PixelShaderBlob)
	{
		pD3D11PixelShaderBlob->Release();
		pD3D11PixelShaderBlob = NULL;
	}

	if (pD3D11VertexShaderBlob)
	{
		pD3D11VertexShaderBlob->Release();
		pD3D11VertexShaderBlob = NULL;
	}

	if (pD3D11IndexBuffer)
	{
		pD3D11IndexBuffer->Release();
		pD3D11IndexBuffer = NULL;
	}

	if (pD3D11VertexBuffer)
	{
		pD3D11VertexBuffer->Release();
		pD3D11VertexBuffer = NULL;
	}

	if (pD3D11SamplerState)
	{
		pD3D11SamplerState->Release();
		pD3D11SamplerState = NULL;
	}

	if (pD3D11WireframeRS)
	{
		pD3D11WireframeRS->Release();
		pD3D11WireframeRS = NULL;
	}

	if (pD3D11SolidRS)
	{
		pD3D11SolidRS->Release();
		pD3D11SolidRS = NULL;
	}

	if (pD3D11DSV)
	{
		pD3D11DSV->Release();
		pD3D11DSV = NULL;
	}

	if (pD3D11DepthStencilTexture2D)
	{
		pD3D11DepthStencilTexture2D->Release();
		pD3D11DepthStencilTexture2D = NULL;
	}

	if (pD3D11RTV)
	{
		pD3D11RTV->Release();
		pD3D11RTV = NULL;
	}

	if (pDXGISwapChain)
	{
		pDXGISwapChain->Release();
		pDXGISwapChain = NULL;
	}

	if (pD3D11DeviceCtx)
	{
		pD3D11DeviceCtx->Release();
		pD3D11DeviceCtx = NULL;
	}

	if (pD3D11Device)
	{
		pD3D11Device->Release();
		pD3D11Device = NULL;
	}

	if (hMainWindow)
	{
		if (!ShowWindow(hMainWindow, SW_HIDE))
		{
			iErrorLevel = GetLastError();
		}
	}

	if (hMainWindow)
	{
		if (!DestroyWindow(hMainWindow))
		{
			iErrorLevel = GetLastError();
		}
		else
		{
			hMainWindow = 0;
		}
	}

	if (aMainWindow)
	{
		if (!UnregisterClass((TCHAR *)((ULONG_PTR)((WORD)(aMainWindow))), GetModuleHandle(NULL)))
		{
			iErrorLevel = GetLastError();
		}
		else
		{
			aMainWindow = 0;
		}
	}

	return iErrorLevel;
}

LRESULT WINAPI WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOVE:
		break;
	case WM_SIZE:
		break;
	case WM_CLOSE:
		if (hMainWindow)
		{
			if (!DestroyWindow(hMainWindow))
			{
				//iErrorLevel = GetLastError();
			}
			else
			{
				hMainWindow = 0;
			}
		}
		break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE:
			bIsExitRequested = true;
			break;
		case SC_MAXIMIZE:
			break;
		case SC_MINIMIZE:
			break;
		case SC_RESTORE:
			break;
		default:
			break;
		}
	default:
		break;
	}

	return DefWindowProc(hWnd, Msg, wParam, lParam);
}
