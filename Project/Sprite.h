#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Texture.h"
#include "SpriteBatch.h"

/**
 * @class Sprite
 * @brief 2Dスプライトのテクスチャ読み込みと描画を管理
 */
class Sprite
{
public:
    Sprite();
    ~Sprite();

    bool Initialize(ID3D11Device* device, const wchar_t* textureFilename);
    /**
     * @brief 指定した座標にスプライトを描画
     */
    void Render(DirectX::SpriteBatch* spriteBatch, const DirectX::XMFLOAT2& position, float scale = 1.0f, float rotation = 0.0f, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
    /**
     * @brief 指定した矩形領域を埋めるようにスプライトを描画
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