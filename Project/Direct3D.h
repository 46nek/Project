#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h> 
#include <memory> 
#include "SpriteBatch.h" 
#include "Light.h" 

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
    XMFLOAT2 Tex;
    XMFLOAT3 Normal;
};

struct MatrixBufferType
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

class Direct3D
{
public:
    // コンストラクタとデストラクタ
    Direct3D();
    ~Direct3D();

    // 初期化と解放
    bool Initialize(HWND hWnd, int screenWidth, int screenHeight);
    void Shutdown();

    // 描画処理
    void BeginScene(float r, float g, float b, float a);
    void Begin2D();
    void End2D();
    void EndScene();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();
    ID3D11InputLayout* GetInputLayout();
    ID3D11VertexShader* GetVertexShader();
    ID3D11PixelShader* GetPixelShader();
    ID3D11SamplerState* GetSamplerState();

    void SetWorldMatrix(const XMMATRIX& world);
    void SetViewMatrix(const XMMATRIX& view);
    void SetProjectionMatrix(const XMMATRIX& projection);
    bool UpdateMatrixBuffer();
    bool UpdateLightBuffer(const LightBufferType& lightBuffer);
    DirectX::SpriteBatch* GetSpriteBatch();

    void TurnZBufferOn();
    void TurnZBufferOff();
    XMMATRIX GetOrthoMatrix();

private:
    // DirectX11の主要なインターフェース
    IDXGISwapChain* m_pSwapChain;
    ID3D11Device* m_pd3dDevice;
    ID3D11DeviceContext* m_pImmediateContext;
    ID3D11RenderTargetView* m_pRenderTargetView;
    ID3D11SamplerState* m_pSamplerState;
    ID3D11Texture2D* m_pDepthStencilBuffer;
    ID3D11DepthStencilState* m_pDepthStencilState;
    ID3D11DepthStencilState* m_pDepthDisabledStencilState;
    ID3D11DepthStencilView* m_pDepthStencilView;

    // シェーダーと頂点データ関連
    ID3D11VertexShader* m_pVertexShader;
    ID3D11PixelShader* m_pPixelShader;
    ID3D11InputLayout* m_pVertexLayout;
    ID3D11Buffer* m_pMatrixBuffer;
    ID3D11Buffer* m_pLightBuffer;

    XMMATRIX m_worldMatrix;
    XMMATRIX m_viewMatrix;
    XMMATRIX m_projectionMatrix;
    XMMATRIX m_orthoMatrix;

    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
};