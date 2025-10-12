#include "PostProcess.h"
#include "Game.h"

// �|�X�g�v���Z�X�p�̒��_�\����
struct PostProcessVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT2 Tex;
};

PostProcess::PostProcess() : m_graphicsDevice(nullptr) {}
PostProcess::~PostProcess() {}

bool PostProcess::Initialize(GraphicsDevice* graphicsDevice, int screenWidth, int screenHeight)
{
    m_graphicsDevice = graphicsDevice;
    ID3D11Device* device = m_graphicsDevice->GetDevice();
    HRESULT hr;

    // --- �����_�[�^�[�Q�b�g�e�N�X�`���̍쐬 ---
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = screenWidth;
    textureDesc.Height = screenHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // 1. �V�[���p
    hr = device->CreateTexture2D(&textureDesc, nullptr, m_sceneTexture.GetAddressOf());
    if (FAILED(hr)) return false;
    hr = device->CreateRenderTargetView(m_sceneTexture.Get(), nullptr, m_sceneRTV.GetAddressOf());
    if (FAILED(hr)) return false;
    hr = device->CreateShaderResourceView(m_sceneTexture.Get(), nullptr, m_sceneSRV.GetAddressOf());
    if (FAILED(hr)) return false;

    // 2. �u���[���p1
    hr = device->CreateTexture2D(&textureDesc, nullptr, m_bloomTexture1.GetAddressOf());
    if (FAILED(hr)) return false;
    hr = device->CreateRenderTargetView(m_bloomTexture1.Get(), nullptr, m_bloomRTV1.GetAddressOf());
    if (FAILED(hr)) return false;
    hr = device->CreateShaderResourceView(m_bloomTexture1.Get(), nullptr, m_bloomSRV1.GetAddressOf());
    if (FAILED(hr)) return false;

    // 3. �u���[���p2
    hr = device->CreateTexture2D(&textureDesc, nullptr, m_bloomTexture2.GetAddressOf());
    if (FAILED(hr)) return false;
    hr = device->CreateRenderTargetView(m_bloomTexture2.Get(), nullptr, m_bloomRTV2.GetAddressOf());
    if (FAILED(hr)) return false;
    hr = device->CreateShaderResourceView(m_bloomTexture2.Get(), nullptr, m_bloomSRV2.GetAddressOf());
    if (FAILED(hr)) return false;

    // --- �t���X�N���[���N�A�b�h�̍쐬 ---
    PostProcessVertex vertices[] =
    {
        { DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }
    };
    unsigned long indices[] = { 3, 1, 0, 3, 0, 2 };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(PostProcessVertex) * 4;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    hr = device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(unsigned long) * 6;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    hr = device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // --- �ڂ����p�萔�o�b�t�@�̍쐬 ---
    D3D11_BUFFER_DESC blurBufferDesc = {};
    blurBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    blurBufferDesc.ByteWidth = sizeof(BlurBufferType);
    blurBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    blurBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device->CreateBuffer(&blurBufferDesc, NULL, &m_blurBuffer);
    if (FAILED(hr)) return false;

    return true;
}

void PostProcess::Shutdown()
{
    // ComPtr�������I�Ƀ��\�[�X��������܂�
}

void PostProcess::SetRenderTarget(ID3D11DeviceContext* deviceContext)
{
    // �V�[���`��̃����_�[�^�[�Q�b�g��ݒ�
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    deviceContext->ClearRenderTargetView(m_sceneRTV.Get(), clearColor);
    deviceContext->ClearDepthStencilView(m_graphicsDevice->GetSwapChain()->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    ID3D11RenderTargetView* rtv = m_sceneRTV.Get();
    deviceContext->OMSetRenderTargets(1, &rtv, m_graphicsDevice->GetSwapChain()->GetDepthStencilView());
}

void PostProcess::Apply(ID3D11DeviceContext* deviceContext)
{
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr }; // ����p��null�|�C���^

    // Z�o�b�t�@���I�t�ɂ���
    m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

    // �|�X�g�v���Z�X�p�̃C���v�b�g���C�A�E�g��ݒ�
    deviceContext->IASetInputLayout(shaderManager->GetPostProcessInputLayout());

    // 1. ���邢�����𒊏o (Scene -> Bloom1)
    deviceContext->OMSetRenderTargets(1, m_bloomRTV1.GetAddressOf(), nullptr);
    deviceContext->ClearRenderTargetView(m_bloomRTV1.Get(), clearColor);
    deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shaderManager->GetBrightPassPixelShader(), nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, m_sceneSRV.GetAddressOf());
    RenderFullscreenQuad(deviceContext);
    deviceContext->PSSetShaderResources(0, 1, nullSRV); // <-- ���

    // 2. ���������̂ڂ��� (Bloom1 -> Bloom2)
    deviceContext->OMSetRenderTargets(1, m_bloomRTV2.GetAddressOf(), nullptr);
    deviceContext->ClearRenderTargetView(m_bloomRTV2.Get(), clearColor);
    deviceContext->PSSetShader(shaderManager->GetGaussianBlurPixelShader(), nullptr, 0);

    // �ڂ����p�萔�o�b�t�@���X�V (����)
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    deviceContext->Map(m_blurBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    BlurBufferType* dataPtr = (BlurBufferType*)mappedResource.pData;
    dataPtr->textureSize = DirectX::XMFLOAT2(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT);
    dataPtr->blurDirection = DirectX::XMFLOAT2(1.0f, 0.0f);
    deviceContext->Unmap(m_blurBuffer.Get(), 0);
    deviceContext->PSSetConstantBuffers(0, 1, m_blurBuffer.GetAddressOf());

    deviceContext->PSSetShaderResources(0, 1, m_bloomSRV1.GetAddressOf());
    RenderFullscreenQuad(deviceContext);
    deviceContext->PSSetShaderResources(0, 1, nullSRV); // <-- ���

    // 3. ���������̂ڂ��� (Bloom2 -> Bloom1)
    deviceContext->OMSetRenderTargets(1, m_bloomRTV1.GetAddressOf(), nullptr);
    deviceContext->ClearRenderTargetView(m_bloomRTV1.Get(), clearColor);

    // �ڂ����p�萔�o�b�t�@���X�V (����)
    deviceContext->Map(m_blurBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (BlurBufferType*)mappedResource.pData;
    dataPtr->textureSize = DirectX::XMFLOAT2(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT);
    dataPtr->blurDirection = DirectX::XMFLOAT2(0.0f, 1.0f);
    deviceContext->Unmap(m_blurBuffer.Get(), 0);

    deviceContext->PSSetShaderResources(0, 1, m_bloomSRV2.GetAddressOf());
    RenderFullscreenQuad(deviceContext);
    deviceContext->PSSetShaderResources(0, 1, nullSRV); // <-- ���

    // 4. ���� (Scene + Bloom1 -> BackBuffer)
    ID3D11RenderTargetView* backBufferRTV = m_graphicsDevice->GetSwapChain()->GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);
    deviceContext->PSSetShader(shaderManager->GetCompositePixelShader(), nullptr, 0);
    ID3D11ShaderResourceView* compositeResources[] = { m_sceneSRV.Get(), m_bloomSRV1.Get() };
    deviceContext->PSSetShaderResources(0, 2, compositeResources);
    RenderFullscreenQuad(deviceContext);

    // Z�o�b�t�@���I���ɖ߂�
    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

    // �V�F�[�_�[���\�[�X�̃o�C���h������
    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
    deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}

void PostProcess::RenderFullscreenQuad(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride = sizeof(PostProcessVertex);
    unsigned int offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->DrawIndexed(6, 0, 0);
}