#include "Renderer.h"
#include "Game.h"
#include "Stage.h"
#include <algorithm>
#include <cmath>

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice)
{
	m_frustum = std::make_unique<Frustum>();
}
Renderer::~Renderer() {}

void Renderer::RenderSceneToTexture(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	const Camera* camera, 
	LightManager* lightManager,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth)
{
	if (!m_graphicsDevice || !camera || !lightManager) return;

	RenderTarget* renderTarget = m_graphicsDevice->GetRenderTarget();
	renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());
	renderTarget->ClearRenderTarget(m_graphicsDevice->GetDeviceContext(), 0.1f, 0.2f, 0.4f, 1.0f);

	RenderDepthPass(stageModels, dynamicModels,lightManager);

	renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());

	RenderMainPass(stageModels, dynamicModels, camera, lightManager, mazeData,pathWidth);
}

void Renderer::RenderFinalPass(const Camera* camera, float vignetteIntensity)
{
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
	// ポストプロセス用の定数バッファを更新
	PostProcessBufferType postProcessBuffer;
	postProcessBuffer.VignetteIntensity = vignetteIntensity;
	m_graphicsDevice->UpdatePostProcessBuffer(postProcessBuffer);

	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);

	DirectX::XMMATRIX currentView = camera->GetViewMatrix();
	DirectX::XMMATRIX previousView = camera->GetPreviousViewMatrix();

	float blurAmount = 1.0f;
	if (memcmp(&currentView, &previousView, sizeof(DirectX::XMMATRIX)) == 0)
	{
		blurAmount = 0.0f;
	}

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
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
	LightManager* lightManager)
{
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

	m_graphicsDevice->GetShadowMapper()->SetRenderTarget(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	std::vector<Model*> allModels = stageModels;
	allModels.insert(allModels.end(), dynamicModels.begin(), dynamicModels.end());
	
	for (Model* model : allModels) {
		if (model) {
			m_graphicsDevice->UpdateMatrixBuffer(
				model->GetWorldMatrix(),
				lightManager->GetLightViewMatrix(),
				lightManager->GetLightProjectionMatrix(),
				lightManager->GetLightViewMatrix(),
				lightManager->GetLightProjectionMatrix()
			);
			model->Render(deviceContext);
		}
	}
}

void Renderer::RenderMainPass(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicmodels,
	const Camera* camera,
	LightManager* lightManager,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth)
{
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
	ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

	DirectX::XMMATRIX viewMatrix = camera->GetViewMatrix();
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
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

	ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	ID3D11ShaderResourceView* shadowSrv = shadowMapper->GetShadowMapSRV();
	deviceContext->PSSetShaderResources(1, 1, &shadowSrv);
	ID3D11SamplerState* shadowSampler = shadowMapper->GetShadowSampleState();
	deviceContext->PSSetSamplers(1, 1, &shadowSampler);

	auto renderModel = [&](Model* model) {
		if (!model) return;

		MaterialBufferType materialBuffer;
		materialBuffer.EmissiveColor = model->GetEmissiveColor();
		materialBuffer.UseTexture = model->GetUseTexture();
		materialBuffer.UseNormalMap = model->HasNormalMap() && model->GetUseNormalMap();
		m_graphicsDevice->UpdateMaterialBuffer(materialBuffer);

		m_graphicsDevice->UpdateMatrixBuffer(
			model->GetWorldMatrix(),
			viewMatrix, // 更新されたviewMatrixを使用
			projectionMatrix, // 更新されたprojectionMatrixを使用
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix()
		);
		model->Render(deviceContext);
		};

	//1.
	for (Model* model : stageModels)
	{
		if (!m_frustum->CheckSphere(model->GetBoundingSphereCenter(), model->GetBoundingSphereRadius()))
		{
			continue;
		}
		renderModel(model);
	}

	//2.
	DirectX::XMFLOAT3 cameraPos = camera->GetPosition();

	for (Model* model : dynamicmodels)
	{
		DirectX::XMFLOAT3 modelPos = model->GetBoundingSphereCenter();

		if (!m_frustum->CheckSphere(modelPos, model->GetBoundingSphereRadius()))
		{
			continue;
		}

		float distSq = (cameraPos.x - modelPos.x) * (cameraPos.x - modelPos.x) +
			(cameraPos.z - modelPos.z) * (cameraPos.z - modelPos.z);

		// オクルージョンカリング
		if (IsOccluded(cameraPos, modelPos, mazeData, pathWidth))
		{
			continue; // 遮蔽されているため描画をスキップ
		}
		renderModel(model);
	}
	deviceContext->OMSetBlendState(m_graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff);
}

bool Renderer::IsOccluded(
	const DirectX::XMFLOAT3& from,
	const DirectX::XMFLOAT3& to,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth)
{
	// 0. 基本的なセットアップ
	DirectX::XMVECTOR vFrom = DirectX::XMLoadFloat3(&from);
	DirectX::XMVECTOR vTo = DirectX::XMLoadFloat3(&to);
	DirectX::XMVECTOR vRayDir = DirectX::XMVectorSubtract(vTo, vFrom);

	DirectX::XMFLOAT3 rayDir;
	DirectX::XMStoreFloat3(&rayDir, vRayDir);

	// 迷路のサイズ
	int mazeHeight = static_cast<int>(mazeData.size());
	int mazeWidth = static_cast<int>(mazeData[0].size());
	if (mazeHeight == 0 || mazeWidth == 0) return false;

	// 1. グリッド座標とステップ方向の計算
	// 現在のグリッド座標 (整数)
	int gridX = static_cast<int>(std::floor(from.x / pathWidth));
	int gridZ = static_cast<int>(std::floor(from.z / pathWidth));

	// 目標のグリッド座標 (整数)
	int targetGridX = static_cast<int>(std::floor(to.x / pathWidth));
	int targetGridZ = static_cast<int>(std::floor(to.z / pathWidth));

	// スタート地点のグリッド座標
	int startGridX = gridX;
	int startGridZ = gridZ;

	// レイの進行方向 (ステップ)
	int stepX = (rayDir.x > 0) ? 1 : -1;
	int stepZ = (rayDir.z > 0) ? 1 : -1;

	// 2. レイパラメータ t のセットアップ
	// レイ: P(t) = from + rayDir * t (t=0..1)
	float t = 0.0f;
	float tMaxX, tMaxZ;
	float deltaTx, deltaTz;

	// 最初のX境界までの t (0..1 の範囲で)
	float nextBoundX = (stepX > 0) ? (std::floor(from.x / pathWidth) + 1) * pathWidth : std::floor(from.x / pathWidth) * pathWidth;
	// 最初のZ境界までの t (0..1 の範囲で)
	float nextBoundZ = (stepZ > 0) ? (std::floor(from.z / pathWidth) + 1) * pathWidth : std::floor(from.z / pathWidth) * pathWidth;

	// ゼロ除算を避ける
	tMaxX = (rayDir.x == 0.0f) ? FLT_MAX : (nextBoundX - from.x) / rayDir.x;
	tMaxZ = (rayDir.z == 0.0f) ? FLT_MAX : (nextBoundZ - from.z) / rayDir.z;

	// グリッドセル1つ分進むのに必要な t の増分
	deltaTx = (rayDir.x == 0.0f) ? FLT_MAX : std::abs(pathWidth / rayDir.x);
	deltaTz = (rayDir.z == 0.0f) ? FLT_MAX : std::abs(pathWidth / rayDir.z);

	// 3. グリッドの走査 (DDA)
	while (t <= 1.0f) // t=1 (ゴール) に到達するまで
	{
		// 現在のセル (gridX, gridZ) をチェック
		bool isStart = (gridX == startGridX && gridZ == startGridZ);
		bool isTarget = (gridX == targetGridX && gridZ == targetGridZ);

		// スタート地点とゴール地点自体は壁チェックをスキップ
		if (!isStart && !isTarget)
		{
			bool isWall = false;
			// グリッド範囲内か？
			if (gridZ >= 0 && gridZ < mazeHeight && gridX >= 0 && gridX < mazeWidth)
			{
				if (mazeData[gridZ][gridX] == MazeGenerator::Wall)
				{
					isWall = true;
				}
			}
			else
			{
				// 迷路の範囲外は壁として扱う
				isWall = true;
			}

			if (isWall)
			{
				// 4. 高さチェック
				// この壁セルに侵入した瞬間の t
				float entryT = (tMaxX < tMaxZ) ? (tMaxX - deltaTx) : (tMaxZ - deltaTz);
				if (entryT < 0.0f) entryT = 0.0f; // 最初のセルの場合

				// このセルから出る瞬間の t
				float exitT = (std::min)(tMaxX, tMaxZ);

				// レイがこのセルを通過する間の Y 座標の範囲 [y_entry, y_exit]
				float y_entry = from.y + rayDir.y * entryT;
				float y_exit = from.y + rayDir.y * exitT;

				float minY = (std::min)(y_entry, y_exit);
				float maxY = (std::max)(y_entry, y_exit);

				// 壁の高さ (0.0 から Stage::WALL_HEIGHT)
				const float wallBottom = 0.0f;
				const float wallTop = Stage::WALL_HEIGHT;

				// [minY, maxY] と [wallBottom, wallTop] のオーバーラップチェック
				if (maxY >= wallBottom && minY <= wallTop)
				{
					return true; // 遮蔽されている
				}
			}
		}

		if (isTarget)
		{
			break; // ターゲットセルに到達
		}

		// 5. t を進める
		if (tMaxX < tMaxZ)
		{
			t = tMaxX;
			tMaxX += deltaTx;
			gridX += stepX;
		}
		else
		{
			t = tMaxZ;
			tMaxZ += deltaTz;
			gridZ += stepZ;
		}
	}

	// 6. ゴールまで到達した (壁に当たらなかった)
	return false; // 遮蔽されていない
}