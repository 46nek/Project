#include "ShadowMapper.h"

ShadowMapper::ShadowMapper()
    : m_shadowMapTexture(nullptr), m_shadowMapDSV(nullptr), m_shadowMapSRV(nullptr),
    m_rasterState(nullptr), m_shadowSampleState(nullptr)
{
}
ShadowMapper::~ShadowMapper() {}

bool ShadowMapper::Initialize(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = SHADOWMAP_WIDTH;
    texDesc.Height = SHADOWMAP_HEIGHT;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_shadowMapTexture);
    if (FAILED(hr)) return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = device->CreateDepthStencilView(m_shadowMapTexture, &dsvDesc, &m_shadowMapDSV);
    if (FAILED(hr)) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(m_shadowMapTexture, &srvDesc, &m_shadowMapSRV);
    if (FAILED(hr)) return false;

    // ▼▼▼ 深度バイアスの値を修正 ▼▼▼
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 5000; // 値を調整
    rasterDesc.SlopeScaledDepthBias = 1.0f;
    rasterDesc.DepthBiasClamp = 0.0f; // クランプは0に設定
    // ▲▲▲ 修正完了 ▲▲▲
    hr = device->CreateRasterizerState(&rasterDesc, &m_rasterState);
    if (FAILED(hr)) return false;

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    hr = device->CreateSamplerState(&sampDesc, &m_shadowSampleState);
    if (FAILED(hr)) return false;

    return true;
}

void ShadowMapper::Shutdown()
{
    if (m_shadowSampleState) m_shadowSampleState->Release();
    if (m_rasterState) m_rasterState->Release();
    if (m_shadowMapSRV) m_shadowMapSRV->Release();
    if (m_shadowMapDSV) m_shadowMapDSV->Release();
    if (m_shadowMapTexture) m_shadowMapTexture->Release();
}

void ShadowMapper::SetRenderTarget(ID3D11DeviceContext* deviceContext)
{
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)SHADOWMAP_WIDTH;
    vp.Height = (FLOAT)SHADOWMAP_HEIGHT;
    vp.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &vp);
    deviceContext->OMSetRenderTargets(0, nullptr, m_shadowMapDSV);
    deviceContext->RSSetState(m_rasterState);
    deviceContext->ClearDepthStencilView(m_shadowMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}