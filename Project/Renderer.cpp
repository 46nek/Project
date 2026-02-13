#include "Renderer.h"
#include "Game.h"
#include "Stage.h"
#include <algorithm>
#include <cmath>

extern Game* g_game;

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice) {
	m_frustum = std::make_unique<Frustum>();
}

Renderer::~Renderer() {
}

void Renderer::RenderSceneToTexture(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	const Camera* camera,
	LightManager* lightManager,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth) {
	if (!m_graphicsDevice || !camera || !lightManager) { return; }

	RenderTarget* renderTarget = m_graphicsDevice->GetRenderTarget();
	renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());
	renderTarget->ClearRenderTarget(m_graphicsDevice->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f); // 閭梧勹繧帝ｻ偵↓螟画峩・医ヵ繧ｩ繧ｰ縺ｫ蜷医ｏ縺帙ｋ・・

	RenderDepthPass(stageModels, dynamicModels, lightManager);

	renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());

	RenderMainPass(stageModels, dynamicModels, camera, lightManager, mazeData, pathWidth);
}

void Renderer::RenderFinalPass(const Camera* camera, float vignetteIntensity) {
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
	auto& settings = g_game->GetSettings(); // 險ｭ螳壹ｒ蜿門ｾ・

	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ繝舌ャ繝輔ぃ縺ｮ譖ｴ譁ｰ
	PostProcessBufferType postProcessBuffer;
	// 譏弱ｋ縺戊ｨｭ螳壹ｒ蜿肴丐 (譏弱ｋ縺・⊇縺ｩ繝ｴ繧｣繝阪ャ繝医ｒ蠑ｱ縺上☆繧・
	postProcessBuffer.vignetteIntensity = vignetteIntensity * (2.0f - settings.brightness);
	postProcessBuffer.fogStart = 0.0f;
	postProcessBuffer.fogEnd = 0.0f;
	postProcessBuffer.fogColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	postProcessBuffer.padding = 0.0f;
	m_graphicsDevice->UpdatePostProcessBuffer(postProcessBuffer);

	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);

	// 繧ｷ繧ｧ繝ｼ繝繝ｼ縺ｯ蟶ｸ縺ｫ MotionBlur 逕ｨ繧剃ｽｿ逕ｨ縺吶ｋ (繝悶Λ繝ｼ驥・0 縺ｪ繧牙腰縺ｪ繧九ヱ繧ｹ繧ｹ繝ｫ繝ｼ縺ｫ縺ｪ繧・
	deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);

	DirectX::XMMATRIX currentView = camera->GetViewMatrix();
	DirectX::XMMATRIX previousView = camera->GetPreviousViewMatrix();

	// 繝｢繝ｼ繧ｷ繝ｧ繝ｳ繝悶Λ繝ｼ險ｭ螳壹′OFF縺ｪ繧峨ヶ繝ｩ繝ｼ驥上ｒ 0 縺ｫ縺吶ｋ
	float blurAmount = settings.motionBlur ? 1.0f : 0.0f;

	// 繧ｫ繝｡繝ｩ縺悟虚縺・※縺・↑縺・ｴ蜷医ｂ繝悶Λ繝ｼ繧・0 縺ｫ縺吶ｋ
	if (memcmp(&currentView, &previousView, sizeof(DirectX::XMMATRIX)) == 0) {
		blurAmount = 0.0f;
	}

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(camera->GetFOV(), (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	DirectX::XMMATRIX prevViewProj = previousView * projectionMatrix;
	DirectX::XMMATRIX currentViewProj = currentView * projectionMatrix;
	DirectX::XMMATRIX currentViewProjInv = DirectX::XMMatrixInverse(nullptr, currentViewProj);

	m_graphicsDevice->UpdateMotionBlurBuffer(prevViewProj, currentViewProjInv, blurAmount);

	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	m_graphicsDevice->UpdateMatrixBuffer(identity, identity, identity, identity, identity);

	ID3D11ShaderResourceView* sceneTexture = m_graphicsDevice->GetRenderTarget()->GetShaderResourceView();
	deviceContext->PSSetShaderResources(0, 1, &sceneTexture);
	ID3D11ShaderResourceView* depthTexture = m_graphicsDevice->GetRenderTarget()->GetDepthShaderResourceView();
	deviceContext->PSSetShaderResources(1, 1, &depthTexture);

	ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	m_graphicsDevice->GetOrthoWindow()->Render(deviceContext);

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}

void Renderer::RenderDepthPass(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	LightManager* lightManager) {
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

	m_graphicsDevice->GetShadowMapper()->SetRenderTarget(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	auto renderDepth = [&](Model* model) {
		if (!model) { return; }

		if (!m_frustum->CheckSphere(model->GetBoundingSphereCenter(), model->GetBoundingSphereRadius())) {
			return;
		}

		m_graphicsDevice->UpdateMatrixBuffer(
			model->GetWorldMatrix(),
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix(),
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix()
		);
		model->Render(deviceContext);
		};

	for (Model* model : stageModels) {
		renderDepth(model);
	}
	for (Model* model : dynamicModels) {
		renderDepth(model);
	}
}

void Renderer::RenderMainPass(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	const Camera* camera,
	LightManager* lightManager,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth) {
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
	ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

	DirectX::XMMATRIX viewMatrix = camera->GetViewMatrix();
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(camera->GetFOV(), (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_frustum->ConstructFrustum(viewMatrix, projectionMatrix);

	D3D11_VIEWPORT vp = {};
	vp.Width = Game::SCREEN_WIDTH;
	vp.Height = Game::SCREEN_HEIGHT;
	vp.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &vp);
	deviceContext->RSSetState(nullptr);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->OMSetBlendState(m_graphicsDevice->GetAlphaBlendState(), blendFactor, 0xffffffff);

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

	m_graphicsDevice->UpdateLightBuffer(lightManager->GetLightBuffer());
	auto& settings = g_game->GetSettings();

	PostProcessBufferType fogParams;
	fogParams.vignetteIntensity = 0.8f * (2.0f - settings.brightness); // 譏弱ｋ縺・⊇縺ｩ蜻ｨ霎ｺ貂帛・繧貞ｼｱ縺・
	fogParams.fogStart = 15.0f;
	fogParams.fogEnd = 60.0f * settings.brightness; // 譏弱ｋ縺・⊇縺ｩ驕縺上∪縺ｧ隕九∴繧九ｈ縺・↓
	fogParams.fogColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	fogParams.padding = 0.0f;
	m_graphicsDevice->UpdatePostProcessBuffer(fogParams);

	ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	ID3D11ShaderResourceView* shadowSrv = shadowMapper->GetShadowMapSRV();
	deviceContext->PSSetShaderResources(1, 1, &shadowSrv);
	ID3D11SamplerState* shadowSampler = shadowMapper->GetShadowSampleState();
	deviceContext->PSSetSamplers(1, 1, &shadowSampler);

	struct CurrentMaterialState {
		DirectX::XMFLOAT4 emissiveColor;
		int useTexture;
		int useNormalMap; 
		bool isValid = false;
	} currentState;

	auto renderModel = [&](Model* model) {
		if (!model) { return; }

		bool needUpdate = !currentState.isValid;
		if (currentState.isValid) {
			auto ec = model->GetEmissiveColor();
			if (ec.x != currentState.emissiveColor.x ||
				ec.y != currentState.emissiveColor.y ||
				ec.z != currentState.emissiveColor.z ||
				model->GetUseTexture() != currentState.useTexture ||
				(model->HasNormalMap() && model->GetUseNormalMap()) != currentState.useNormalMap) {
				needUpdate = true;
			}
		}

		if (needUpdate) {
			MaterialBufferType materialBuffer;
			materialBuffer.emissiveColor = model->GetEmissiveColor();
			materialBuffer.useTexture = model->GetUseTexture();     
			materialBuffer.useNormalMap = model->HasNormalMap() && model->GetUseNormalMap();
			materialBuffer.padding = DirectX::XMFLOAT2(0, 0);
			m_graphicsDevice->UpdateMaterialBuffer(materialBuffer);

			currentState.emissiveColor = materialBuffer.emissiveColor;
			currentState.useTexture = materialBuffer.useTexture;
			currentState.useNormalMap = materialBuffer.useNormalMap;
			currentState.isValid = true;
		}

		m_graphicsDevice->UpdateMatrixBuffer(
			model->GetWorldMatrix(),
			viewMatrix,
			projectionMatrix,
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix()
		);
		model->Render(deviceContext);
		};

	// 1. 繧ｹ繝・・繧ｸ繝｢繝・Ν
	for (Model* model : stageModels) {
		if (!m_frustum->CheckSphere(model->GetBoundingSphereCenter(), model->GetBoundingSphereRadius())) {
			continue;
		}
		renderModel(model);
	}

	// 2. 蜍慕噪繝｢繝・Ν
	for (Model* model : dynamicModels) {
		DirectX::XMFLOAT3 modelPos = model->GetBoundingSphereCenter();
		if (!m_frustum->CheckSphere(modelPos, model->GetBoundingSphereRadius())) {
			continue;
		}
		renderModel(model);
	}

	deviceContext->OMSetBlendState(m_graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff);
}
