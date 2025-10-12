#include "ShaderManager.h"

ShaderManager::ShaderManager()
    : m_vertexShader(nullptr), m_pixelShader(nullptr), m_depthVertexShader(nullptr), m_inputLayout(nullptr)
{
}
ShaderManager::~ShaderManager() {}

bool ShaderManager::Initialize(ID3D11Device* device)
{
    HRESULT hr;
    ID3DBlob* errorBlob = nullptr;

    // Helper lambda for compiling shaders
    auto compileShader = [&](LPCWSTR filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blob) -> bool {
        hr = D3DCompileFromFile(filename, nullptr, nullptr, entryPoint, shaderModel, 0, 0, blob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) {
                char* errorMessage = (char*)(errorBlob->GetBufferPointer());
                MessageBoxA(NULL, errorMessage, "Shader Compile Error", MB_OK);
                errorBlob->Release();
            }
            else {
                MessageBoxW(NULL, filename, L"Shader File Not Found", MB_OK);
            }
            return false;
        }
        if (errorBlob) errorBlob->Release();
        return true;
        };

    // --- 既存シェーダーの読み込み ---
    ID3DBlob* vsBlob = nullptr;
    if (!compileShader(L"VertexShader.hlsl", "VS", "vs_5_0", &vsBlob)) return false;
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(hr)) { vsBlob->Release(); return false; }

    ID3DBlob* psBlob = nullptr;
    if (!compileShader(L"PixelShader.hlsl", "PS", "ps_5_0", &psBlob)) return false;
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    ID3DBlob* depthVsBlob = nullptr;
    if (!compileShader(L"DepthVertexShader.hlsl", "main", "vs_5_0", &depthVsBlob)) return false;
    hr = device->CreateVertexShader(depthVsBlob->GetBufferPointer(), depthVsBlob->GetBufferSize(), nullptr, &m_depthVertexShader);
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); depthVsBlob->Release(); return false; }

    // --- ポストプロセス用シェーダーの読み込み ---
    ID3DBlob* postProcessVsBlob = nullptr;
    if (!compileShader(L"PostProcessVS.hlsl", "main", "vs_5_0", &postProcessVsBlob)) return false;
    hr = device->CreateVertexShader(postProcessVsBlob->GetBufferPointer(), postProcessVsBlob->GetBufferSize(), nullptr, &m_postProcessVS);
    if (FAILED(hr)) { postProcessVsBlob->Release(); return false; }

    ID3DBlob* brightPassPsBlob = nullptr;
    if (!compileShader(L"BrightPassPS.hlsl", "main", "ps_5_0", &brightPassPsBlob)) return false;
    hr = device->CreatePixelShader(brightPassPsBlob->GetBufferPointer(), brightPassPsBlob->GetBufferSize(), nullptr, &m_brightPassPS);
    if (FAILED(hr)) { brightPassPsBlob->Release(); return false; }

    ID3DBlob* gaussianBlurPsBlob = nullptr;
    if (!compileShader(L"GaussianBlurPS.hlsl", "main", "ps_5_0", &gaussianBlurPsBlob)) return false;
    hr = device->CreatePixelShader(gaussianBlurPsBlob->GetBufferPointer(), gaussianBlurPsBlob->GetBufferSize(), nullptr, &m_gaussianBlurPS);
    if (FAILED(hr)) { gaussianBlurPsBlob->Release(); return false; }

    ID3DBlob* compositePsBlob = nullptr;
    if (!compileShader(L"CompositePS.hlsl", "main", "ps_5_0", &compositePsBlob)) return false;
    hr = device->CreatePixelShader(compositePsBlob->GetBufferPointer(), compositePsBlob->GetBufferSize(), nullptr, &m_compositePS);
    if (FAILED(hr)) { compositePsBlob->Release(); return false; }

    // --- インプットレイアウトの作成 ---
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC postProcessLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(postProcessLayout, ARRAYSIZE(postProcessLayout), postProcessVsBlob->GetBufferPointer(), postProcessVsBlob->GetBufferSize(), &m_postProcessInputLayout);
    if (FAILED(hr)) return false;

    // --- Blobの解放 ---
    vsBlob->Release();
    psBlob->Release();
    depthVsBlob->Release();
    postProcessVsBlob->Release();
    brightPassPsBlob->Release();
    gaussianBlurPsBlob->Release();
    compositePsBlob->Release();

    return true;
}

void ShaderManager::Shutdown()
{
    if (m_compositePS) { m_compositePS->Release(); m_compositePS = nullptr; }
    if (m_gaussianBlurPS) { m_gaussianBlurPS->Release(); m_gaussianBlurPS = nullptr; }
    if (m_brightPassPS) { m_brightPassPS->Release(); m_brightPassPS = nullptr; }
    if (m_postProcessVS) { m_postProcessVS->Release(); m_postProcessVS = nullptr; }
    if (m_postProcessInputLayout) { m_postProcessInputLayout->Release(); m_postProcessInputLayout = nullptr; }
    if (m_inputLayout) { m_inputLayout->Release(); m_inputLayout = nullptr; }
    if (m_depthVertexShader) { m_depthVertexShader->Release(); m_depthVertexShader = nullptr; }
    if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = nullptr; }
    if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }
}