#include "Texture.h"
#include "WICTextureLoader.h"

Texture::Texture() {
}

Texture::~Texture() {
}

bool Texture::Initialize(ID3D11Device* device, const wchar_t* filename) {
	HRESULT result;

	// 繝・け繧ｹ繝√Ε繝輔ぃ繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ繧
	result = DirectX::CreateWICTextureFromFile(device, filename, nullptr, m_textureView.GetAddressOf());
	if (FAILED(result)) {
		return false;
	}

	return true;
}

void Texture::Shutdown() {
}

ID3D11ShaderResourceView* Texture::GetTexture() {
	return m_textureView.Get();
}
