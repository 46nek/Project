#pragma once
#include <d3d11.h>
#include <dxgi.h>

/**
 * @class SwapChain
 * @brief スワップチェイン、レンダーターゲットビュー、深度ステンシルビューを管理
 */
class SwapChain {
public:
	SwapChain();
	~SwapChain();

	bool Initialize(ID3D11Device* device, HWND hWnd, int screenWidth, int screenHeight);
	void Shutdown();

	void BeginScene(ID3D11DeviceContext* deviceContext, float r, float g, float b, float a);
	void EndScene();

	void TurnZBufferOn(ID3D11DeviceContext* deviceContext);
	void TurnZBufferOff(ID3D11DeviceContext* deviceContext);

	// ゲッター
	ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView; }
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView; }

private:
	IDXGISwapChain* m_swapChain;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	int m_screenWidth;
	int m_screenHeight;
};