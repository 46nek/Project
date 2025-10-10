#pragma once
#include <d3d11.h>
#include <memory>
#include "SwapChain.h"
#include "ShaderManager.h"
#include "ShadowMapper.h"

struct MatrixBufferType
{
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX worldInverseTranspose;
    DirectX::XMMATRIX lightView;
    DirectX::XMMATRIX lightProjection;
};

struct LightBufferType; // ëOï˚êÈåæ

class GraphicsDevice
{
public:
    GraphicsDevice();
    ~GraphicsDevice();

    bool Initialize(HWND hWnd, int screenWidth, int screenHeight);
    void Shutdown();

    void BeginScene(float r, float g, float b, float a);
    void EndScene();

    bool UpdateMatrixBuffer(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMMATRIX& lightView, const DirectX::XMMATRIX& lightProjection);
    bool UpdateLightBuffer(const LightBufferType& lightBuffer);

    ID3D11Device* GetDevice() const { return m_d3dDevice; }
    ID3D11DeviceContext* GetDeviceContext() const { return m_immediateContext; }
    SwapChain* GetSwapChain() const { return m_swapChain.get(); }
    ShaderManager* GetShaderManager() const { return m_shaderManager.get(); }
    ShadowMapper* GetShadowMapper() const { return m_shadowMapper.get(); }

private:
    ID3D11Device* m_d3dDevice;
    ID3D11DeviceContext* m_immediateContext;
    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_lightBuffer;
    ID3D11SamplerState* m_samplerState;

    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<ShadowMapper> m_shadowMapper;
};