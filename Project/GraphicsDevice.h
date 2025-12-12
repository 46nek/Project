#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "SwapChain.h"
#include "ShaderManager.h"
#include "ShadowMapper.h"
#include "RenderTarget.h"
#include "OrthoWindow.h"

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
 * @brief マテリアル情報をシェーダーに渡すための構造体
 */
struct MaterialBufferType {
	DirectX::XMFLOAT4 emissiveColor;
	BOOL useTexture;
	BOOL useNormalMap;
	DirectX::XMFLOAT2 padding;
};

/**
 * @struct PostProcessBufferType
 * @brief ポストプロセスおよびフォグ等のエフェクト用パラメータ
 */
struct PostProcessBufferType {
	float vignetteIntensity;
	float fogStart;          // フォグの開始距離
	float fogEnd;            // フォグの終了距離（この距離で完全にフォグ色になる）
	float padding;           // アライメント用パディング
	DirectX::XMFLOAT4 fogColor; // フォグの色
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