#include "PostProcess.h"
#include "Game.h"

struct ScreenSizeBufferType
{
    float screenWidth;
    float screenHeight;
    DirectX::XMFLOAT2 padding;
};

PostProcess::PostProcess() : m_graphicsDevice(nullptr) {}
PostProcess::~PostProcess() {}

bool PostProcess::Initialize(GraphicsDevice* graphicsDevice, int screenWidth, int screenHeight)
{
    m_graphicsDevice = graphicsDevice;
    ID3D11Device* device = m_graphicsDevice->GetDevice();
    HRESULT hr;

    // レンダーターゲットテクスチャの作成
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = screenWidth;
    textureDesc.Height = screenHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture);
    if (FAILED(hr)) return false;

    // レンダーターゲットビューの作成
    hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) return false;

    // シェーダーリソースビューの作成
    hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), nullptr, &m_shaderResourceView);
    if (FAILED(hr)) return false;

    // 速度マップ用テクスチャの作成
    textureDesc.Format = DXGI_FORMAT_R16G16_FLOAT; // 速度ベクトル用にフォーマットを変更
    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_velocityTexture);
    if (FAILED(hr)) return false;
    
    //速度マップ用レンダーターゲットビューの作成
    hr = device->CreateRenderTargetView(m_velocityTexture.Get(), nullptr, &m_velocityRenderTargetView);
    if (FAILED(hr)) return false;
    
    // 速度マップ用シェーダーリソースビューの作成
    hr = device->CreateShaderResourceView(m_velocityTexture.Get(), nullptr, &m_velocityShaderResourceView);
    if (FAILED(hr)) return false;

    // ブラー用テクスチャの作成
    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_blurTexture);
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(m_blurTexture.Get(), nullptr, &m_blurRenderTargetView);
    if (FAILED(hr)) return false;

    hr = device->CreateShaderResourceView(m_blurTexture.Get(), nullptr, &m_blurShaderResourceView);
    if (FAILED(hr)) return false;

    // 定数バッファの作成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device->CreateBuffer(&bufferDesc, nullptr, &m_screenSizeBuffer);
    if (FAILED(hr)) return false;

    // サンプラーステートの作成
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&samplerDesc, &m_samplerState);
    if (FAILED(hr)) return false;

    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

    return true;
}

void PostProcess::Shutdown()
{
}

void PostProcess::Begin(ID3D11DeviceContext* deviceContext)
{
    // MRT (Multiple Render Targets) の設定
    // 0番目に色、1番目に速度マップをセット
    ID3D11RenderTargetView * rtvs[2] = { m_renderTargetView.Get(), m_velocityRenderTargetView.Get() };
    deviceContext->OMSetRenderTargets(2, rtvs, m_graphicsDevice->GetSwapChain()->GetDepthStencilView());
}

void PostProcess::RenderBlur(ID3D11DeviceContext* deviceContext)
{
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

    // 定数バッファの更新
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    deviceContext->Map(m_screenSizeBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    ScreenSizeBufferType* dataPtr = (ScreenSizeBufferType*)mappedResource.pData;
    dataPtr->screenWidth = Game::SCREEN_WIDTH;
    dataPtr->screenHeight = Game::SCREEN_HEIGHT;
    deviceContext->Unmap(m_screenSizeBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(0, 1, m_screenSizeBuffer.GetAddressOf());

    deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    // 水平ブラー
    ID3D11RenderTargetView* blurRtv = m_blurRenderTargetView.Get();
    deviceContext->OMSetRenderTargets(1, &blurRtv, nullptr);
    deviceContext->PSSetShader(shaderManager->GetHorizontalBlurPixelShader(), nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf()); 

    m_spriteBatch->Begin(DirectX::SpriteSortMode_Immediate, nullptr, nullptr, nullptr, nullptr, [=]() {
        deviceContext->PSSetShader(shaderManager->GetHorizontalBlurPixelShader(), nullptr, 0);
        });
    m_spriteBatch->Draw(m_shaderResourceView.Get(), RECT{ 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT });
    m_spriteBatch->End();

    // 垂直ブラー
    ID3D11RenderTargetView* backBufferRtv = m_graphicsDevice->GetSwapChain()->GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &backBufferRtv, nullptr);
    deviceContext->PSSetShader(shaderManager->GetVerticalBlurPixelShader(), nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, m_blurShaderResourceView.GetAddressOf());
    m_spriteBatch->Begin(DirectX::SpriteSortMode_Immediate, nullptr, nullptr, nullptr, nullptr, [=]() {
        deviceContext->PSSetShader(shaderManager->GetVerticalBlurPixelShader(), nullptr, 0);
        });
    m_spriteBatch->Draw(m_blurShaderResourceView.Get(), RECT{ 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT });
    m_spriteBatch->End();
}
void PostProcess::RenderMotionBlur(ID3D11DeviceContext* deviceContext)
{
    ShaderManager * shaderManager = m_graphicsDevice->GetShaderManager();
    
    // 最終的な描画先をバックバッファに戻す
    ID3D11RenderTargetView * backBufferRtv = m_graphicsDevice->GetSwapChain()->GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &backBufferRtv, nullptr);
    
    // モーションブラーシェーダーをセット
    deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);
    // 0番スロットにシーンのテクスチャ、1番スロットに速度マップのテクスチャをセット
    ID3D11ShaderResourceView * srvs[] = { m_shaderResourceView.Get(), m_velocityShaderResourceView.Get() };
    deviceContext->PSSetShaderResources(0, 2, srvs);
    deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    
    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_shaderResourceView.Get(), RECT{ 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT });
    m_spriteBatch->End();
    }
void PostProcess::End(ID3D11DeviceContext* deviceContext)
{
    // レンダーターゲットを元に戻す
    ID3D11RenderTargetView* rtv = m_graphicsDevice->GetSwapChain()->GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &rtv, m_graphicsDevice->GetSwapChain()->GetDepthStencilView());
}