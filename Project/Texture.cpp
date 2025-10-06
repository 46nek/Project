#include "Texture.h"
#include "WICTextureLoader.h"

Texture::Texture()
{
}

Texture::~Texture()
{
}

bool Texture::Initialize(ID3D11Device* device, const wchar_t* filename)
{
    HRESULT result;

    // テクスチャファイルを読み込む
    result = DirectX::CreateWICTextureFromFile(device, filename, nullptr, m_textureView.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void Texture::Shutdown()
{
    // ComPtrが自動的にテクスチャを解放してくれます
}

ID3D11ShaderResourceView* Texture::GetTexture()
{
    return m_textureView.Get();
}