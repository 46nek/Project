#pragma once
#include "Texture.h"
#include "SpriteBatch.h"
#include <DirectXMath.h>
#include <wrl/client.h>

/**
 * @class Sprite
 * @brief 2D繧ｹ繝励Λ繧､繝医・繝・け繧ｹ繝√Ε隱ｭ縺ｿ霎ｼ縺ｿ縺ｨ謠冗判繧堤ｮ｡逅・
 */
class Sprite {
public:
	Sprite();
	~Sprite();

	bool Initialize(ID3D11Device* device, const wchar_t* textureFilename);
	/**
	 * @brief 謖・ｮ壹＠縺溷ｺｧ讓吶↓繧ｹ繝励Λ繧､繝医ｒ謠冗判
	 */
	void Render(DirectX::SpriteBatch* spriteBatch, const DirectX::XMFLOAT2& position, float scale = 1.0f, float rotation = 0.0f, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	/**
	 * @brief 謖・ｮ壹＠縺溽洸蠖｢鬆伜沺繧貞沂繧√ｋ繧医≧縺ｫ繧ｹ繝励Λ繧､繝医ｒ謠冗判
	 */
	void RenderFill(DirectX::SpriteBatch* spriteBatch, const RECT& destinationRectangle);
	void Shutdown();

	int GetWidth() const { return m_textureWidth; }
	int GetHeight() const { return m_textureHeight; }
	/**
	 * @brief 謖・ｮ壹＠縺溯牡縺ｧ遏ｩ蠖｢鬆伜沺繧貞｡励ｊ縺､縺ｶ縺励※謠冗判縺励∪縺吶・
	 * @param spriteBatch 繧ｹ繝励Λ繧､繝医ヰ繝・メ
	 * @param destinationRect 謠冗判蜈医・遏ｩ蠖｢
	 * @param color 蝪励ｊ縺､縺ｶ縺呵牡 (RGBA)
	 */
	void RenderFill(DirectX::SpriteBatch* spriteBatch, const RECT& destinationRect, const DirectX::XMFLOAT4& color);
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
	int m_textureWidth;
	int m_textureHeight;
};
