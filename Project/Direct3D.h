// Direct3D.h
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h> 

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct SimpleVertex
{
    float Pos[3];
    float Color[4];
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
    void EndScene();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();

    void SetWorldMatrix(const XMMATRIX& world);
    void SetViewMatrix(const XMMATRIX& view);
    void SetProjectionMatrix(const XMMATRIX& projection);
    bool UpdateMatrixBuffer();

private:
    // DirectX11の主要なインターフェース
    IDXGISwapChain* m_pSwapChain;
    ID3D11Device* m_pd3dDevice;
    ID3D11DeviceContext* m_pImmediateContext;
    ID3D11RenderTargetView* m_pRenderTargetView;

    // シェーダーと頂点データ関連
    ID3D11VertexShader* m_pVertexShader;
    ID3D11PixelShader* m_pPixelShader;
    ID3D11InputLayout* m_pVertexLayout;
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pMatrixBuffer;

    XMMATRIX m_worldMatrix;
    XMMATRIX m_viewMatrix;
    XMMATRIX m_projectionMatrix;
};