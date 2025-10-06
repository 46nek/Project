#pragma once
#include <d3d11.h>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class Texture
{
public:
    Texture();
    ~Texture();

    bool Initialize(ID3D11Device* device, const wchar_t* filename);
    void Shutdown();

    ID3D11ShaderResourceView* GetTexture();

private:
    ComPtr<ID3D11ShaderResourceView> m_textureView;
};