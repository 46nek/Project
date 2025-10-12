#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "SpriteBatch.h"

class GraphicsDevice;

class PostProcess
{
public:
    PostProcess();
    ~PostProcess();

    bool Initialize(GraphicsDevice* graphicsDevice, int screenWidth, int screenHeight);
    void Shutdown();

    void Begin(ID3D11DeviceContext* deviceContext);
    void RenderBlur(ID3D11DeviceContext* deviceContext);
    void RenderMotionBlur(ID3D11DeviceContext* deviceContext);
    void End(ID3D11DeviceContext* deviceContext);

    ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    
    // 速度マップ用のリソースを追加
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_velocityTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_velocityRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_velocityShaderResourceView;
    
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_blurTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_blurRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_blurShaderResourceView;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_screenSizeBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    GraphicsDevice* m_graphicsDevice;
};