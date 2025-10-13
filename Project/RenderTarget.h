#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class RenderTarget
{
public:
    RenderTarget();
    ~RenderTarget();

    bool Initialize(ID3D11Device* device, int textureWidth, int textureHeight, bool createDepthSrv = false);
    void Shutdown();

    void SetRenderTarget(ID3D11DeviceContext* deviceContext);
    void ClearRenderTarget(ID3D11DeviceContext* deviceContext, float r, float g, float b, float a);
    ID3D11ShaderResourceView* GetShaderResourceView();
    ID3D11ShaderResourceView* GetDepthShaderResourceView();
    ID3D11DepthStencilView* GetDepthStencilView();

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthShaderResourceView;
};