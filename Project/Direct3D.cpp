// Direct3D.cpp
#include "Direct3D.h"

Direct3D::Direct3D()
{
    // ポインタをNULLで初期化
    m_pSwapChain = nullptr;
    m_pd3dDevice = nullptr;
    m_pImmediateContext = nullptr;
    m_pRenderTargetView = nullptr;
    m_pVertexShader = nullptr;
    m_pPixelShader = nullptr;
    m_pVertexLayout = nullptr;
    m_pMatrixBuffer = nullptr;
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

    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

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
    };
    UINT numElements = ARRAYSIZE(layout);
    hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
    pVSBlob->Release();
    pPSBlob->Release();
    if (FAILED(hr)) return false;

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
    return true;
}

void Direct3D::Shutdown()
{
    // 作成と逆の順序でリソースを解放
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