#include "Sprite.h"
#include "WICTextureLoader.h"
#include "SpriteBatch.h"

Sprite::Sprite() : m_textureWidth(0), m_textureHeight(0)
{
}

Sprite::~Sprite()
{
}

bool Sprite::Initialize(ID3D11Device* device, const wchar_t* textureFilename)
{
    Microsoft::WRL::ComPtr<ID3D11Resource> resource; // ComPtr�𖾊m��
    HRESULT hr = DirectX::CreateWICTextureFromFile(device, textureFilename, resource.GetAddressOf(), m_textureView.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D; // ComPtr�𖾊m��
    hr = resource.As(&texture2D);
    if (SUCCEEDED(hr))
    {
        D3D11_TEXTURE2D_DESC desc;
        texture2D->GetDesc(&desc);
        m_textureWidth = desc.Width;
        m_textureHeight = desc.Height;
    }

    return true;
}

void Sprite::Render(DirectX::SpriteBatch* spriteBatch, const DirectX::XMFLOAT2& position, float scale, float rotation, const DirectX::XMFLOAT4& color)
{
    if (spriteBatch && m_textureView)
    {
        DirectX::XMFLOAT2 origin(m_textureWidth / 2.0f, m_textureHeight / 2.0f); 
        spriteBatch->Draw(m_textureView.Get(), position, nullptr, DirectX::XMLoadFloat4(&color), rotation, origin, scale); 
    }
}

void Sprite::Shutdown()
{
    m_textureView.Reset();
}