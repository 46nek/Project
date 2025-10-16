#include "UIOrb.h"
#include "AssetLoader.h"
#include "Game.h"
#include "LightManager.h"

UIOrb::UIOrb() : m_graphicsDevice(nullptr)
{
}

UIOrb::~UIOrb()
{
}

bool UIOrb::Initialize(GraphicsDevice* graphicsDevice)
{
    m_graphicsDevice = graphicsDevice;

    // UI�p�̃I�[�u���f����ǂݍ���
    m_orbModel = AssetLoader::LoadModelFromFile(m_graphicsDevice->GetDevice(), "Assets/cube.obj");
    if (!m_orbModel)
    {
        return false;
    }
    m_orbModel->SetScale(0.3f, 0.3f, 0.3f);
    m_orbModel->SetEmissiveColor({ 0.6f, 0.8f, 1.0f, 1.0f });
    m_orbModel->SetUseTexture(false);
    m_orbModel->SetUseNormalMap(false);

    // UI�\���p�̃J������ݒ�
    m_orbCamera = std::make_unique<Camera>();
    m_orbCamera->SetPosition(0.0f, 0.5f, -2.0f);
    m_orbCamera->SetRotation(-15.0f, 0.0f, 0.0f);

    return true;
}

void UIOrb::Shutdown()
{
    if (m_orbModel)
    {
        m_orbModel->Shutdown();
    }
}

void UIOrb::Update(float deltaTime)
{
    // UI Orb����]������
    static float uiOrbRotationY = 0.0f;
    static float uiOrbRotationX = 0.0f;
    uiOrbRotationY += deltaTime * 1.5f;
    uiOrbRotationX += deltaTime * 0.5f;
    m_orbModel->SetRotation(uiOrbRotationX, uiOrbRotationY, 0.0f);
}

void UIOrb::Render()
{
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

    // UI�p�̃r���[�|�[�g���` (��ʉE��ɕ\��)
    D3D11_VIEWPORT uiViewport = {};
    uiViewport.Width = 150.0f;
    uiViewport.Height = 150.0f;
    uiViewport.TopLeftX = Game::SCREEN_WIDTH - uiViewport.Width - 20.0f;
    uiViewport.TopLeftY = 20.0f;
    uiViewport.MinDepth = 0.0f;
    uiViewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &uiViewport);

    // �`���Ԃ����Z�b�g
    m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);
    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    deviceContext->OMSetBlendState(m_graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff);
    deviceContext->RSSetState(m_graphicsDevice->GetDefaultRasterizerState());
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
    deviceContext->PSSetShaderResources(0, 3, nullSRVs);
    ID3D11SamplerState* nullSamplers[] = { nullptr, nullptr };
    deviceContext->PSSetSamplers(0, 2, nullSamplers);

    // 3D�`��p�̃V�F�[�_�[��ݒ�
    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

    // UI Orb�ɂ͏Ɩ���K�p���Ȃ�
    LightBufferType uiLightBuffer = {};
    uiLightBuffer.NumLights = 0;
    uiLightBuffer.CameraPosition = m_orbCamera->GetPosition();
    m_graphicsDevice->UpdateLightBuffer(uiLightBuffer);

    // UI�p�̃}�e���A���i���Ȕ����F�̂݁j��ݒ�
    MaterialBufferType materialBuffer;
    materialBuffer.EmissiveColor = m_orbModel->GetEmissiveColor();
    materialBuffer.UseTexture = false;
    materialBuffer.UseNormalMap = false;
    m_graphicsDevice->UpdateMaterialBuffer(materialBuffer);

    // UI�p�̃J�����ƍs��Ń��f����`��
    m_orbCamera->Update();
    DirectX::XMMATRIX uiViewMatrix = m_orbCamera->GetViewMatrix();
    DirectX::XMMATRIX uiProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, 1.0f, 0.1f, 100.0f);

    m_graphicsDevice->UpdateMatrixBuffer(
        m_orbModel->GetWorldMatrix(),
        uiViewMatrix,
        uiProjectionMatrix,
        DirectX::XMMatrixIdentity(),
        DirectX::XMMatrixIdentity());

    // Orb��`��
    m_orbModel->Render(deviceContext);

    // �r���[�|�[�g�Ɛ[�x�ݒ�����C���`��p�ɕ���
    D3D11_VIEWPORT mainViewport = {};
    mainViewport.Width = (FLOAT)Game::SCREEN_WIDTH;
    mainViewport.Height = (FLOAT)Game::SCREEN_HEIGHT;
    mainViewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &mainViewport);
    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);
}