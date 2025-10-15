// ShadowMapper.h

#pragma once
#include <d3d11.h>

/**
 * @class ShadowMapper
 * @brief シャドウマッピング用の深度テクスチャと関連リソースを管理
 */
class ShadowMapper
{
public:
	static constexpr int SHADOWMAP_WIDTH = 256;
	static constexpr int SHADOWMAP_HEIGHT = 256;

	ShadowMapper();
	~ShadowMapper();

	bool Initialize(ID3D11Device* device);
	void Shutdown();

	/**
	 * @brief 深度テクスチャをレンダーターゲットとして設定
	 */
	void SetRenderTarget(ID3D11DeviceContext* deviceContext);

	// ゲッター
	ID3D11ShaderResourceView* GetShadowMapSRV() const { return m_shadowMapSRV; }
	ID3D11SamplerState* GetShadowSampleState() const { return m_shadowSampleState; }

private:
	ID3D11Texture2D* m_shadowMapTexture;
	ID3D11DepthStencilView* m_shadowMapDSV;
	ID3D11ShaderResourceView* m_shadowMapSRV;
	ID3D11RasterizerState* m_rasterState;
	ID3D11SamplerState* m_shadowSampleState;
};