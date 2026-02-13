#include "Sprite.h"
#include "WICTextureLoader.h"
#include "SpriteBatch.h"

Sprite::Sprite() : m_textureWidth(0), m_textureHeight(0) {
}

Sprite::~Sprite() {
}

bool Sprite::Initialize(ID3D11Device* device, const wchar_t* textureFilename) {
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT hr = DirectX::CreateWICTextureFromFile(device, textureFilename, resource.GetAddressOf(), m_textureView.GetAddressOf());
	if (FAILED(hr)) {
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D;
	hr = resource.As(&texture2D);
	if (SUCCEEDED(hr)) {
		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);
		m_textureWidth = desc.Width;
		m_textureHeight = desc.Height;
	}

	return true;
}

// 繝ｭ繧ｴ繝ｻ繝懊ち繝ｳ逕ｨ縺ｮRender髢｢謨ｰ縺ｮ螳溯｣・
void Sprite::Render(DirectX::SpriteBatch* spriteBatch, const DirectX::XMFLOAT2& position, float scale, float rotation, const DirectX::XMFLOAT4& color) {
	if (spriteBatch && m_textureView) {
		DirectX::XMFLOAT2 origin(m_textureWidth / 2.0f, m_textureHeight / 2.0f);
		spriteBatch->Draw(m_textureView.Get(), position, nullptr, DirectX::XMLoadFloat4(&color), rotation, origin, scale);
	}
}

// 閭梧勹逕ｨ縺ｮRender髢｢謨ｰ縺ｮ螳溯｣・
void Sprite::RenderFill(DirectX::SpriteBatch* spriteBatch, const RECT& destinationRectangle) {
	if (spriteBatch && m_textureView) {
		spriteBatch->Draw(m_textureView.Get(), destinationRectangle);
	}
}

void Sprite::Shutdown() {
	m_textureView.Reset();
}

void Sprite::RenderFill(DirectX::SpriteBatch* spriteBatch, const RECT& destinationRect, const DirectX::XMFLOAT4& color) {
	// m_texture 縺ｧ縺ｯ縺ｪ縺上［_textureView 繧剃ｽｿ縺・∪縺・
	if (spriteBatch && m_textureView) {
		// m_textureView.Get() 縺ｧ繧ｷ繧ｧ繝ｼ繝繝ｼ繝ｪ繧ｽ繝ｼ繧ｹ繝薙Η繝ｼ繧貞叙蠕励＠縺ｾ縺・
		spriteBatch->Draw(m_textureView.Get(), destinationRect, DirectX::XMLoadFloat4(&color));
	}
}
