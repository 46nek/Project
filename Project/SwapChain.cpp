#include "SwapChain.h"

SwapChain::SwapChain()
	: m_swapChain(nullptr), m_renderTargetView(nullptr), m_depthStencilBuffer(nullptr),
	m_depthStencilState(nullptr), m_depthDisabledStencilState(nullptr), m_depthStencilView(nullptr),
	// メンバ変数を初期化
	m_screenWidth(0), m_screenHeight(0) {
}

SwapChain::~SwapChain() {
}

bool SwapChain::Initialize(ID3D11Device* device, HWND hWnd, int screenWidth, int screenHeight) {
	// 画面サイズをメンバ変数に保存
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = screenWidth;
	sd.BufferDesc.Height = screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	IDXGIDevice* dxgiDevice = nullptr;
	device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	IDXGIAdapter* dxgiAdapter = nullptr;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	IDXGIFactory* dxgiFactory = nullptr;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	HRESULT hr = dxgiFactory->CreateSwapChain(device, &sd, &m_swapChain);

	dxgiFactory->Release();
	dxgiAdapter->Release();
	dxgiDevice->Release();

	if (FAILED(hr)) { return false; }

	// レンダーターゲットビューの作成
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) { return false; }
	hr = device->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr)) { return false; }

	// デプスステンシルバッファとビューの作成
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer); // NULL -> nullptr
	if (FAILED(hr)) { return false; }

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthBufferDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_depthStencilBuffer, &dsvDesc, &m_depthStencilView);
	if (FAILED(hr)) { return false; }

	// デプスステンシルステートの作成 (Zバッファ有効/無効)
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
	if (FAILED(hr)) { return false; }
	dsDesc.DepthEnable = false;
	hr = device->CreateDepthStencilState(&dsDesc, &m_depthDisabledStencilState);
	if (FAILED(hr)) { return false; }

	return true;
}

void SwapChain::Shutdown() {
	if (m_swapChain) { m_swapChain->SetFullscreenState(false, nullptr); } // NULL -> nullptr
	if (m_depthDisabledStencilState) { m_depthDisabledStencilState->Release(); }
	if (m_depthStencilState) { m_depthStencilState->Release(); }
	if (m_depthStencilView) { m_depthStencilView->Release(); }
	if (m_depthStencilBuffer) { m_depthStencilBuffer->Release(); }
	if (m_renderTargetView) { m_renderTargetView->Release(); }
	if (m_swapChain) { m_swapChain->Release(); }
}

void SwapChain::BeginScene(ID3D11DeviceContext* deviceContext, float r, float g, float b, float a) {
	// ビューポートを設定する処理を追加
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)m_screenWidth;
	viewport.Height = (FLOAT)m_screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	deviceContext->RSSetViewports(1, &viewport);

	float color[4] = { r, g, b, a };
	deviceContext->ClearRenderTargetView(m_renderTargetView, color);
	deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

void SwapChain::EndScene() {
	m_swapChain->Present(1, 0);
}

void SwapChain::TurnZBufferOn(ID3D11DeviceContext* deviceContext) {
	deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
}

void SwapChain::TurnZBufferOff(ID3D11DeviceContext* deviceContext) {
	deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}