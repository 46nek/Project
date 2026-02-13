#pragma once
#include "SwapChain.h"
#include "ShaderManager.h"
#include "ShadowMapper.h"
#include "RenderTarget.h"
#include "OrthoWindow.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

struct MatrixBufferType {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX worldInverseTranspose;
	DirectX::XMMATRIX lightView;
	DirectX::XMMATRIX lightProjection;
};

struct MotionBlurBufferType {
	DirectX::XMMATRIX previousViewProjection;
	DirectX::XMMATRIX currentViewProjectionInverse;
	float blurAmount;
	DirectX::XMFLOAT3 padding;
};

/**
 * @struct MaterialBufferType
 * @brief 繝槭ユ繝ｪ繧｢繝ｫ諠・ｱ繧偵す繧ｧ繝ｼ繝繝ｼ縺ｫ貂｡縺吶◆繧√・讒矩菴・
 */
struct MaterialBufferType {
	DirectX::XMFLOAT4 emissiveColor;
	BOOL useTexture;
	BOOL useNormalMap;
	DirectX::XMFLOAT2 padding;
};

/**
 * @struct PostProcessBufferType
 * @brief 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺翫ｈ縺ｳ繝輔か繧ｰ遲峨・繧ｨ繝輔ぉ繧ｯ繝育畑繝代Λ繝｡繝ｼ繧ｿ
 */
struct PostProcessBufferType {
	float vignetteIntensity;
	float fogStart;          // 繝輔か繧ｰ縺ｮ髢句ｧ玖ｷ晞屬
	float fogEnd;            // 繝輔か繧ｰ縺ｮ邨ゆｺ・ｷ晞屬・医％縺ｮ霍晞屬縺ｧ螳悟・縺ｫ繝輔か繧ｰ濶ｲ縺ｫ縺ｪ繧具ｼ・
	float padding;           // 繧｢繝ｩ繧､繝｡繝ｳ繝育畑繝代ョ繧｣繝ｳ繧ｰ
	DirectX::XMFLOAT4 fogColor; // 繝輔か繧ｰ縺ｮ濶ｲ
};

struct LightBufferType;

class GraphicsDevice {
public:
	GraphicsDevice();
	~GraphicsDevice();

	bool Initialize(HWND hWnd, int screenWidth, int screenHeight);
	void Shutdown();
	void BeginScene(float r, float g, float b, float a);
	void EndScene();
	bool UpdateMatrixBuffer(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMMATRIX& lightView, const DirectX::XMMATRIX& lightProjection);
	bool UpdateLightBuffer(const LightBufferType& lightBuffer);
	bool UpdateMotionBlurBuffer(const DirectX::XMMATRIX& prevViewProj, const DirectX::XMMATRIX& currentViewProjInv, float blurAmount);
	bool UpdateMaterialBuffer(const MaterialBufferType& materialBuffer);
	bool UpdatePostProcessBuffer(const PostProcessBufferType& postProcessBuffer);

	ID3D11Device* GetDevice() const { return m_d3dDevice; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_immediateContext; }
	SwapChain* GetSwapChain() const { return m_swapChain.get(); }
	ShaderManager* GetShaderManager() const { return m_shaderManager.get(); }
	ShadowMapper* GetShadowMapper() const { return m_shadowMapper.get(); }
	RenderTarget* GetRenderTarget() const { return m_renderTarget.get(); }
	OrthoWindow* GetOrthoWindow() const { return m_orthoWindow.get(); }
	ID3D11SamplerState* GetSamplerState() const { return m_samplerState; }
	ID3D11BlendState* GetAlphaBlendState() const;
	ID3D11BlendState* GetDefaultBlendState() const;
	ID3D11RasterizerState* GetDefaultRasterizerState() const;

private:
	ID3D11Device* m_d3dDevice;
	ID3D11DeviceContext* m_immediateContext;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_motionBlurBuffer;
	ID3D11Buffer* m_materialBuffer;
	ID3D11Buffer* m_postProcessBuffer;
	ID3D11SamplerState* m_samplerState;
	ID3D11BlendState* m_alphaBlendState;
	ID3D11BlendState* m_defaultBlendState;
	ID3D11RasterizerState* m_defaultRasterizerState;

	std::unique_ptr<SwapChain> m_swapChain;
	std::unique_ptr<ShaderManager> m_shaderManager;
	std::unique_ptr<ShadowMapper> m_shadowMapper;
	std::unique_ptr<RenderTarget> m_renderTarget;
	std::unique_ptr<OrthoWindow> m_orthoWindow;
};
