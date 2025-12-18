#include "ParticleSystem.h"
#include "AssetLoader.h"
#include "AssetPaths.h"
#include "GraphicsDevice.h"
#include "ShaderManager.h" 
#include <random>
#include <cmath>

namespace {
	const int MAX_PARTICLES = 10000;
	const float SPAWN_INTERVAL = 0.0001f;
	const float PARTICLE_LIFE = 1.0f;

	const float PARTICLE_SCALE_MIN = 0.02f;
	const float PARTICLE_SCALE_MAX = 0.04f;

	const float SWARM_RADIUS = 1.2f;
	const float BUZZ_SPEED = 5.0f;
}

ParticleSystem::ParticleSystem()
	: m_spawnTimer(0.0f), m_depthStencilState(nullptr),
	m_vertexBuffer(nullptr), m_indexBuffer(nullptr) {
	m_particles.resize(MAX_PARTICLES);
	for (auto& p : m_particles) {
		p.active = false;
	}
	// 頂点バッファ用の配列を予約確保（1粒子あたり4頂点）
	m_vertexBatch.resize(MAX_PARTICLES * 4);
}

ParticleSystem::~ParticleSystem() {
	Shutdown();
}

bool ParticleSystem::Initialize(ID3D11Device* device) {
	if (!InitializeBuffers(device)) return false;
	if (!InitializeDepthStencilState(device)) return false;

	// テクスチャ読み込み
	m_texture = std::make_shared<Texture>();
	if (!m_texture->Initialize(device, AssetPaths::ENEMY_PARTICLE)) {
		return false;
	}

	return true;
}

bool ParticleSystem::InitializeDepthStencilState(ID3D11Device* device) {
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 半透明なので書き込みOFF
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;

	return SUCCEEDED(device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState));
}

bool ParticleSystem::InitializeBuffers(ID3D11Device* device) {
	// 1. ダイナミック頂点バッファの作成
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // CPUから毎フレーム書き込む設定
	vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * MAX_PARTICLES * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(device->CreateBuffer(&vertexBufferDesc, nullptr, &m_vertexBuffer))) {
		return false;
	}

	// 2. インデックスバッファの作成（不変）
	std::vector<unsigned long> indices;
	indices.reserve(MAX_PARTICLES * 6);
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		// 1つの四角形（2つの三角形）を構成するインデックス
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 1);
		indices.push_back(i * 4 + 2);
		indices.push_back(i * 4 + 1);
		indices.push_back(i * 4 + 3);
		indices.push_back(i * 4 + 2);
	}

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * static_cast<UINT>(indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = indices.data();

	if (FAILED(device->CreateBuffer(&indexBufferDesc, &initData, &m_indexBuffer))) {
		return false;
	}

	return true;
}

void ParticleSystem::Update(float deltaTime, const DirectX::XMFLOAT3& emitterPosition) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distStandard(0.0f, 1.0f);
	std::uniform_real_distribution<float> distPos(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distScale(PARTICLE_SCALE_MIN, PARTICLE_SCALE_MAX);
	std::uniform_real_distribution<float> distRot(0.0f, 6.28f);
	std::uniform_real_distribution<float> distOffset(0.0f, 100.0f);

	m_spawnTimer -= deltaTime;
	while (m_spawnTimer <= 0.0f) {
		m_spawnTimer += SPAWN_INTERVAL;

		for (auto& p : m_particles) {
			if (!p.active) {
				p.active = true;

				// 球体状に配置
				float theta = 2.0f * 3.14159f * distStandard(gen);
				float phi = std::acos(2.0f * distStandard(gen) - 1.0f);
				float radius = SWARM_RADIUS * std::cbrt(distStandard(gen));

				float x = radius * std::sin(phi) * std::cos(theta);
				float y = radius * std::sin(phi) * std::sin(theta);
				float z = radius * std::cos(phi);

				p.position = {
					emitterPosition.x + x,
					emitterPosition.y + y + 1.0f,
					emitterPosition.z + z
				};

				p.velocity = { distPos(gen) * 0.2f, distPos(gen) * 0.2f, distPos(gen) * 0.2f };
				p.maxLife = PARTICLE_LIFE + distPos(gen) * 0.2f;
				p.life = p.maxLife;
				p.scale = distScale(gen);
				p.rotation = distRot(gen);
				p.timeOffset = distOffset(gen);
				break;
			}
		}
	}

	for (auto& p : m_particles) {
		if (!p.active) continue;

		p.life -= deltaTime;
		if (p.life <= 0.0f) {
			p.active = false;
			continue;
		}

		float time = (p.maxLife - p.life) + p.timeOffset;
		float noiseX = std::sin(time * BUZZ_SPEED * 3.0f) * 0.5f;
		float noiseY = std::cos(time * BUZZ_SPEED * 4.0f) * 0.5f;
		float noiseZ = std::sin(time * BUZZ_SPEED * 2.5f) * 0.5f;

		p.position.x += (p.velocity.x + noiseX) * deltaTime;
		p.position.y += (p.velocity.y + noiseY) * deltaTime;
		p.position.z += (p.velocity.z + noiseZ) * deltaTime;

		p.rotation += deltaTime * 5.0f;
	}
}

void ParticleSystem::Render(GraphicsDevice* graphicsDevice,
	const DirectX::XMMATRIX& viewMatrix,
	const DirectX::XMMATRIX& projectionMatrix,
	const DirectX::XMMATRIX& lightViewMatrix,
	const DirectX::XMMATRIX& lightProjectionMatrix) {

	if (!m_vertexBuffer || !graphicsDevice || !m_depthStencilState || !m_texture) return;

	ID3D11DeviceContext* context = graphicsDevice->GetDeviceContext();

	// 1. ブレンドステート (半透明)
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(graphicsDevice->GetAlphaBlendState(), blendFactor, 0xffffffff);

	// 2. 深度ステート
	ID3D11DepthStencilState* prevDepthState = nullptr;
	UINT prevStencilRef = 0;
	context->OMGetDepthStencilState(&prevDepthState, &prevStencilRef);
	context->OMSetDepthStencilState(m_depthStencilState, 0);

	// 3. シェーダー設定 (光の影響を受けないTexturePixelShaderを使用)
	ShaderManager* shaderManager = graphicsDevice->GetShaderManager();
	if (shaderManager) {
		context->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
		// ここでTexturePixelShaderを使うことで、ライティング計算（鏡面反射・拡散反射）を回避
		context->PSSetShader(shaderManager->GetTexturePixelShader(), nullptr, 0);
		context->IASetInputLayout(shaderManager->GetInputLayout());
	}

	// 4. マトリクス設定 (World行列はIdentityにし、頂点座標はCPUで計算済みワールド座標を送る)
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
	graphicsDevice->UpdateMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix);

	// 5. 頂点データの更新 (CPUでビルボード計算)
	// カメラの右・上方向ベクトルを取得
	DirectX::XMMATRIX cameraWorld = DirectX::XMMatrixInverse(nullptr, viewMatrix);
	DirectX::XMVECTOR camRight = cameraWorld.r[0];
	DirectX::XMVECTOR camUp = cameraWorld.r[1];

	int activeCount = 0;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (SUCCEEDED(context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		SimpleVertex* verticesPtr = (SimpleVertex*)mappedResource.pData;

		for (const auto& p : m_particles) {
			if (!p.active) continue;

			// ビルボードの4隅を計算
			float halfS = p.scale * 0.5f;

			// 回転(簡易的にZ軸回転)
			float cosR = std::cos(p.rotation);
			float sinR = std::sin(p.rotation);

			// 回転後のオフセットベクトル
			// Right * (x*cos - y*sin) + Up * (x*sin + y*cos)
			DirectX::XMVECTOR rightVec = DirectX::XMVectorScale(camRight, halfS);
			DirectX::XMVECTOR upVec = DirectX::XMVectorScale(camUp, halfS);

			DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&p.position);

			// 4頂点生成
			auto SetVertex = [&](int index, float u, float v, float xSign, float ySign) {
				// 回転を考慮した頂点位置計算
				float rotX = xSign * cosR - ySign * sinR;
				float rotY = xSign * sinR + ySign * cosR;

				DirectX::XMVECTOR finalPos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(camRight, rotX * halfS));
				finalPos = DirectX::XMVectorAdd(finalPos, DirectX::XMVectorScale(camUp, rotY * halfS));

				DirectX::XMStoreFloat3(&verticesPtr[activeCount * 4 + index].pos, finalPos);
				verticesPtr[activeCount * 4 + index].tex = { u, v };
				verticesPtr[activeCount * 4 + index].color = { 1.0f, 1.0f, 1.0f, 1.0f };
				verticesPtr[activeCount * 4 + index].normal = { 0.0f, 0.0f, -1.0f };
				};

			SetVertex(0, 0.0f, 0.0f, -1.0f, 1.0f); // 左上
			SetVertex(1, 1.0f, 0.0f, 1.0f, 1.0f); // 右上
			SetVertex(2, 0.0f, 1.0f, -1.0f, -1.0f); // 左下
			SetVertex(3, 1.0f, 1.0f, 1.0f, -1.0f); // 右下

			activeCount++;
		}
		context->Unmap(m_vertexBuffer, 0);
	}

	// 6. 描画 (1回のドローコールで完了)
	if (activeCount > 0) {
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// テクスチャ設定
		ID3D11ShaderResourceView* textureView = m_texture->GetTexture();
		context->PSSetShaderResources(0, 1, &textureView);

		// まとめて描画
		context->DrawIndexed(activeCount * 6, 0, 0);
	}

	// 後片付け
	context->OMSetBlendState(graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff);
	context->OMSetDepthStencilState(prevDepthState, prevStencilRef);
	if (prevDepthState) prevDepthState->Release();
}

void ParticleSystem::Shutdown() {
	if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
	if (m_indexBuffer) { m_indexBuffer->Release(); m_indexBuffer = nullptr; }
	if (m_depthStencilState) { m_depthStencilState->Release(); m_depthStencilState = nullptr; }
}