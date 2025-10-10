#include "ShaderManager.h"

ShaderManager::ShaderManager()
    : m_vertexShader(nullptr), m_pixelShader(nullptr), m_depthVertexShader(nullptr), m_inputLayout(nullptr)
{
}
ShaderManager::~ShaderManager() {}

bool ShaderManager::Initialize(ID3D11Device* device)
{
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* depthVsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr;

    // 頂点シェーダーのコンパイル
    hr = D3DCompileFromFile(L"../Shaders/VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) { /* エラー処理 */ return false; }
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(hr)) { vsBlob->Release(); return false; }

    // ピクセルシェーダーのコンパイル
    hr = D3DCompileFromFile(L"../Shaders/PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) { /* エラー処理 */ vsBlob->Release(); return false; }
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    // 深度頂点シェーダーのコンパイル
    hr = D3DCompileFromFile(L"../Shaders/DepthVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &depthVsBlob, &errorBlob);
    if (FAILED(hr)) { /* エラー処理 */ return false; }
    hr = device->CreateVertexShader(depthVsBlob->GetBufferPointer(), depthVsBlob->GetBufferSize(), nullptr, &m_depthVertexShader);
    if (FAILED(hr)) { depthVsBlob->Release(); return false; }

    // インプットレイアウトの作成
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    vsBlob->Release();
    psBlob->Release();
    depthVsBlob->Release();
    if (errorBlob) errorBlob->Release();

    return SUCCEEDED(hr);
}

void ShaderManager::Shutdown()
{
    if (m_inputLayout) m_inputLayout->Release();
    if (m_depthVertexShader) m_depthVertexShader->Release();
    if (m_pixelShader) m_pixelShader->Release();
    if (m_vertexShader) m_vertexShader->Release();
}