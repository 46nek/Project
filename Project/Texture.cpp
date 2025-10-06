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

    // �e�N�X�`���t�@�C����ǂݍ���
    result = DirectX::CreateWICTextureFromFile(device, filename, nullptr, m_textureView.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void Texture::Shutdown()
{
    // ComPtr�������I�Ƀe�N�X�`����������Ă���܂�
}

ID3D11ShaderResourceView* Texture::GetTexture()
{
    return m_textureView.Get();
}