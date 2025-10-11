#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Texture.h"
#include "SpriteBatch.h"

/**
 * @class Sprite
 * @brief 2D�X�v���C�g�̃e�N�X�`���ǂݍ��݂ƕ`����Ǘ�
 */
class Sprite
{
public:
    Sprite();
    ~Sprite();

    bool Initialize(ID3D11Device* device, const wchar_t* textureFilename);
    /**
     * @brief �w�肵�����W�ɃX�v���C�g��`��
     */
    void Render(DirectX::SpriteBatch* spriteBatch, const DirectX::XMFLOAT2& position, float scale = 1.0f, float rotation = 0.0f, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
    /**
     * @brief �w�肵����`�̈�𖄂߂�悤�ɃX�v���C�g��`��
     */
    void RenderFill(DirectX::SpriteBatch* spriteBatch, const RECT& destinationRectangle);
    void Shutdown();

    int GetWidth() const { return m_textureWidth; }
    int GetHeight() const { return m_textureHeight; }

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
    int m_textureWidth;
    int m_textureHeight;
};