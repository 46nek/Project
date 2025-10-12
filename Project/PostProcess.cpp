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

    // �����_�[�^�[�Q�b�g�e�N�X�`���̍쐬
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

    // �����_�[�^�[�Q�b�g�r���[�̍쐬
    hr = device->CreateRenderTargetView(m_renderTargetTexture.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) return false;

    // �V�F�[�_�[���\�[�X�r���[�̍쐬
    hr = device->CreateShaderResourceView(m_renderTargetTexture.Get(), nullptr, &m_shaderResourceView);
    if (FAILED(hr)) return false;

    // ���x�}�b�v�p�e�N�X�`���̍쐬
    textureDesc.Format = DXGI_FORMAT_R16G16_FLOAT; // ���x�x�N�g���p�Ƀt�H�[�}�b�g��ύX
    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_velocityTexture);
    if (FAILED(hr)) return false;
    
    //���x�}�b�v�p�����_�[�^�[�Q�b�g�r���[�̍쐬
    hr = device->CreateRenderTargetView(m_velocityTexture.Get(), nullptr, &m_velocityRenderTargetView);
    if (FAILED(hr)) return false;
    
    // ���x�}�b�v�p�V�F�[�_�[���\�[�X�r���[�̍쐬
    hr = device->CreateShaderResourceView(m_velocityTexture.Get(), nullptr, &m_velocityShaderResourceView);
    if (FAILED(hr)) return false;

    // �u���[�p�e�N�X�`���̍쐬
    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_blurTexture);
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(m_blurTexture.Get(), nullptr, &m_blurRenderTargetView);
    if (FAILED(hr)) return false;

    hr = device->CreateShaderResourceView(m_blurTexture.Get(), nullptr, &m_blurShaderResourceView);
    if (FAILED(hr)) return false;

    // �萔�o�b�t�@�̍쐬
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device->CreateBuffer(&bufferDesc, nullptr, &m_screenSizeBuffer);
    if (FAILED(hr)) return false;

    // �T���v���[�X�e�[�g�̍쐬
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
    // MRT (Multiple Render Targets) �̐ݒ�
    // 0�ԖڂɐF�A1�Ԗڂɑ��x�}�b�v���Z�b�g
    ID3D11RenderTargetView * rtvs[2] = { m_renderTargetView.Get(), m_velocityRenderTargetView.Get() };
    deviceContext->OMSetRenderTargets(2, rtvs, m_graphicsDevice->GetSwapChain()->GetDepthStencilView());
}

void PostProcess::RenderBlur(ID3D11DeviceContext* deviceContext)
{
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

    // �萔�o�b�t�@�̍X�V
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    deviceContext->Map(m_screenSizeBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    ScreenSizeBufferType* dataPtr = (ScreenSizeBufferType*)mappedResource.pData;
    dataPtr->screenWidth = Game::SCREEN_WIDTH;
    dataPtr->screenHeight = Game::SCREEN_HEIGHT;
    deviceContext->Unmap(m_screenSizeBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(0, 1, m_screenSizeBuffer.GetAddressOf());

    deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    // �����u���[
    ID3D11RenderTargetView* blurRtv = m_blurRenderTargetView.Get();
    deviceContext->OMSetRenderTargets(1, &blurRtv, nullptr);
    deviceContext->PSSetShader(shaderManager->GetHorizontalBlurPixelShader(), nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf()); 

    m_spriteBatch->Begin(DirectX::SpriteSortMode_Immediate, nullptr, nullptr, nullptr, nullptr, [=]() {
        deviceContext->PSSetShader(shaderManager->GetHorizontalBlurPixelShader(), nullptr, 0);
        });
    m_spriteBatch->Draw(m_shaderResourceView.Get(), RECT{ 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT });
    m_spriteBatch->End();

    // �����u���[
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
    
    // �ŏI�I�ȕ`�����o�b�N�o�b�t�@�ɖ߂�
    ID3D11RenderTargetView * backBufferRtv = m_graphicsDevice->GetSwapChain()->GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &backBufferRtv, nullptr);
    
    // ���[�V�����u���[�V�F�[�_�[���Z�b�g
    deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);
    // 0�ԃX���b�g�ɃV�[���̃e�N�X�`���A1�ԃX���b�g�ɑ��x�}�b�v�̃e�N�X�`�����Z�b�g
    ID3D11ShaderResourceView * srvs[] = { m_shaderResourceView.Get(), m_velocityShaderResourceView.Get() };
    deviceContext->PSSetShaderResources(0, 2, srvs);
    deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    
    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_shaderResourceView.Get(), RECT{ 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT });
    m_spriteBatch->End();
    }
void PostProcess::End(ID3D11DeviceContext* deviceContext)
{
    // �����_�[�^�[�Q�b�g�����ɖ߂�
    ID3D11RenderTargetView* rtv = m_graphicsDevice->GetSwapChain()->GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &rtv, m_graphicsDevice->GetSwapChain()->GetDepthStencilView());
}