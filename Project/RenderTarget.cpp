#include "RenderTarget.h"

RenderTarget::RenderTarget() {
}
RenderTarget::~RenderTarget() {
}

bool RenderTarget::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, bool createDepthSrv) {
	// --- カラーテクスチャの作成 ---
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture);
	if (FAILED(hr)) { return false; }

	hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), nullptr, &m_renderTargetView);
	if (FAILED(hr)) { return false; }

	hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), nullptr, &m_shaderResourceView);
	if (FAILED(hr)) { return false; }

	// --- 深度バッファの作成 ---
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = createDepthSrv ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | (createDepthSrv ? D3D11_BIND_SHADER_RESOURCE : 0);

	hr = device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer);
	if (FAILED(hr)) { return false; }

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = createDepthSrv ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, &m_depthStencilView);
	if (FAILED(hr)) { return false; }

	// モーションブラー用に、深度テクスチャのシェーダーリソースビューも作成する場合
	if (createDepthSrv) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(m_depthStencilBuffer.Get(), &srvDesc, &m_depthShaderResourceView);
		if (FAILED(hr)) { return false; }
	}

	return true;
}

void RenderTarget::Shutdown() {
	m_depthShaderResourceView.Reset();
	m_depthStencilView.Reset();
	m_depthStencilBuffer.Reset();
	m_shaderResourceView.Reset();
	m_renderTargetView.Reset();
	m_renderTargetTexture.Reset();
}

void RenderTarget::SetRenderTarget(ID3D11DeviceContext* deviceContext) {
	deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

void RenderTarget::ClearRenderTarget(ID3D11DeviceContext* deviceContext, float r, float g, float b, float a) {
	float color[4] = { r, g, b, a };
	deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
	deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTarget::GetShaderResourceView() {
	return m_shaderResourceView.Get();
}

ID3D11ShaderResourceView* RenderTarget::GetDepthShaderResourceView() {
	return m_depthShaderResourceView.Get();
}

ID3D11DepthStencilView* RenderTarget::GetDepthStencilView() {
	return m_depthStencilView.Get();
}