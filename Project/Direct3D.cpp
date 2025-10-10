#include "Direct3D.h"
#include "SpriteBatch.h" 

Direct3D::Direct3D()
    : m_pSwapChain(nullptr),
    m_pd3dDevice(nullptr),
    m_pImmediateContext(nullptr),
    m_pRenderTargetView(nullptr),
    m_pVertexShader(nullptr),
    m_pPixelShader(nullptr),
    m_pVertexLayout(nullptr),
    m_pMatrixBuffer(nullptr),
    m_pLightBuffer(nullptr),
    m_pSamplerState(nullptr),
    m_pDepthStencilBuffer(nullptr),
    m_pDepthStencilState(nullptr),
    m_pDepthDisabledStencilState(nullptr),
    m_pDepthStencilView(nullptr),
    m_worldMatrix(DirectX::XMMatrixIdentity()),  
    m_viewMatrix(DirectX::XMMatrixIdentity()),
    m_projectionMatrix(DirectX::XMMatrixIdentity()),
    m_orthoMatrix(DirectX::XMMatrixIdentity())
{
    // コンストラクタの処理は空になります
}

Direct3D::~Direct3D()
{
    // Shutdownでリソースを解放するので、ここでは何もしない
}

bool Direct3D::Initialize(HWND hWnd, int screenWidth, int screenHeight)
{
    HRESULT hr = S_OK;

    // スワップチェーンとデバイスの作成
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
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, nullptr, &m_pImmediateContext);
    if (FAILED(hr)) return false;

    // レンダーターゲットビューの作成
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) return false;
    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr)) return false;

    // デプスバッファのデスクリプタを設定
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = screenWidth;
    depthBufferDesc.Height = screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = m_pd3dDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_pDepthStencilBuffer);
    if (FAILED(hr)) return false;

    // デプスステンシルステートのデスクリプタを設定 (Zバッファ有効)
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = m_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState);
    if (FAILED(hr)) return false;

    // Zバッファ無効のステートを作成
    depthStencilDesc.DepthEnable = false;
    hr = m_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthDisabledStencilState);
    if (FAILED(hr)) return false;

    // デプスステンシルビューの作成
    hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer, NULL, &m_pDepthStencilView);
    if (FAILED(hr)) return false;

    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
    m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 1);


    // ビューポートの設定
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)screenWidth;
    vp.Height = (FLOAT)screenHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports(1, &vp);

    // シェーダーのコンパイルと作成
    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &pVSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        return false;
    }
    hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return false;
    }

    ID3DBlob* pPSBlob = nullptr;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &pPSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        return false;
    }
    hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
    if (FAILED(hr))
    {
        pPSBlob->Release();
        return false;
    }

    // インプットレイアウトの作成
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // <--- 追加
    };
    UINT numElements = ARRAYSIZE(layout);
    hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
    pVSBlob->Release();
    pPSBlob->Release();
    if (FAILED(hr)) return false;

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // サンプラーステートを作成
    hr = m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState);
    if (FAILED(hr))
    {
        return false;
    }

    // ワールド、ビュー、プロジェクション行列を初期化
    m_worldMatrix = XMMatrixIdentity();
    m_viewMatrix = XMMatrixIdentity();

    // プロジェクション行列を作成
    float fieldOfView = 3.141592654f / 4.0f; // 45 degrees
    float screenAspect = (float)screenWidth / (float)screenHeight;
    m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, 0.1f, 1000.0f);

    // コンスタントバッファのデスクリプタを設定
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // コンスタントバッファを作成
    hr = m_pd3dDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    // ライトバッファのデスクリプタを設定
    D3D11_BUFFER_DESC lightBufferDesc;
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    // ライトバッファを作成
    hr = m_pd3dDevice->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    // SpriteBatchの作成
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pImmediateContext);
    // 正射影行列を作成
    m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, 0.1f, 1000.0f);

    return true;
}

void Direct3D::Shutdown()
{
    m_spriteBatch.reset();

    if (m_pSamplerState) { m_pSamplerState->Release(); m_pSamplerState = nullptr; }
    if (m_pMatrixBuffer) { m_pMatrixBuffer->Release(); m_pMatrixBuffer = nullptr; }
    if (m_pDepthDisabledStencilState) { m_pDepthDisabledStencilState->Release(); m_pDepthDisabledStencilState = nullptr; }
    if (m_pDepthStencilState) { m_pDepthStencilState->Release(); m_pDepthStencilState = nullptr; }
    if (m_pDepthStencilView) { m_pDepthStencilView->Release(); m_pDepthStencilView = nullptr; }
    if (m_pDepthStencilBuffer) { m_pDepthStencilBuffer->Release(); m_pDepthStencilBuffer = nullptr; }

    // 作成と逆の順序でリソースを解放
    if (m_pLightBuffer) { m_pLightBuffer->Release(); m_pLightBuffer = nullptr; }
    if (m_pMatrixBuffer) { m_pMatrixBuffer->Release(); m_pMatrixBuffer = nullptr; }
    if (m_pVertexLayout) { m_pVertexLayout->Release(); m_pVertexLayout = nullptr; }
    if (m_pPixelShader) { m_pPixelShader->Release(); m_pPixelShader = nullptr; }
    if (m_pVertexShader) { m_pVertexShader->Release(); m_pVertexShader = nullptr; }
    if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }
    if (m_pImmediateContext) { m_pImmediateContext->Release(); m_pImmediateContext = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
}

void Direct3D::BeginScene(float r, float g, float b, float a)
{
    float color[4] = { r, g, b, a };
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, color);
    m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Direct3D::Begin2D()
{
    m_spriteBatch->Begin();
}

void Direct3D::End2D()
{
    m_spriteBatch->End();
}

DirectX::SpriteBatch* Direct3D::GetSpriteBatch()
{
    return m_spriteBatch.get();
}
void Direct3D::SetWorldMatrix(const XMMATRIX& world)
{
    m_worldMatrix = world;
}

void Direct3D::SetViewMatrix(const XMMATRIX& view)
{
    m_viewMatrix = view;
}

void Direct3D::SetProjectionMatrix(const XMMATRIX& projection)
{
    m_projectionMatrix = projection;
}

bool Direct3D::UpdateMatrixBuffer()
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;

    // 行列を転置してシェーダーで使えるようにする
    XMMATRIX worldMatrix = XMMatrixTranspose(m_worldMatrix);
    XMMATRIX viewMatrix = XMMatrixTranspose(m_viewMatrix);
    XMMATRIX projectionMatrix = XMMatrixTranspose(m_projectionMatrix);

    // コンスタントバッファをロックして書き込めるようにする
    result = m_pImmediateContext->Map(m_pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // データポインタを取得
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // 行列をコンスタントバッファにコピー
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // コンスタントバッファをアンロック
    m_pImmediateContext->Unmap(m_pMatrixBuffer, 0);

    // 頂点シェーダーにコンスタントバッファをセット
    m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffer);

    return true;
}

bool Direct3D::UpdateLightBuffer(const Light* lights, int numLights, const XMFLOAT3& cameraPosition)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    LightBufferType* dataPtr;

    // ライトバッファをロックして書き込めるようにする
    result = m_pImmediateContext->Map(m_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // データポインタを取得
    dataPtr = (LightBufferType*)mappedResource.pData;

    // ライトの情報をコンスタントバッファにコピー
    memcpy(dataPtr->Lights, lights, sizeof(Light) * numLights);
    dataPtr->NumLights = numLights;
    dataPtr->CameraPosition = cameraPosition;

    // コンスタントバッファをアンロック
    m_pImmediateContext->Unmap(m_pLightBuffer, 0);

    // ピクセルシェーダーにライトバッファをセット (スロット1)
    m_pImmediateContext->PSSetConstantBuffers(1, 1, &m_pLightBuffer);

    return true;
}

void Direct3D::EndScene()
{
    m_pSwapChain->Present(1, 0);
}

ID3D11Device* Direct3D::GetDevice()
{
    return m_pd3dDevice;
}

ID3D11DeviceContext* Direct3D::GetDeviceContext()
{
    return m_pImmediateContext;
}

ID3D11InputLayout* Direct3D::GetInputLayout()
{
    return m_pVertexLayout;
}

ID3D11VertexShader* Direct3D::GetVertexShader()
{
    return m_pVertexShader;
}

ID3D11PixelShader* Direct3D::GetPixelShader()
{
    return m_pPixelShader;
}
ID3D11SamplerState* Direct3D::GetSamplerState()
{
    return m_pSamplerState;
}

void Direct3D::TurnZBufferOn()
{
    m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
}

void Direct3D::TurnZBufferOff()
{
    m_pImmediateContext->OMSetDepthStencilState(m_pDepthDisabledStencilState, 1);
}

XMMATRIX Direct3D::GetOrthoMatrix()
{
    return m_orthoMatrix;
}