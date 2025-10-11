#pragma once
#include <d3d11.h>
#include <wrl/client.h> 

/**
 * @class Texture
 * @brief WIC (Windows Imaging Component) ���g�p�����e�N�X�`���̓ǂݍ��݂ƊǗ�
 */
class Texture
{
public:
    Texture();
    ~Texture();

    bool Initialize(ID3D11Device* device, const wchar_t* filename);
    void Shutdown();
    ID3D11ShaderResourceView* GetTexture();

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
};