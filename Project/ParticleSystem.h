#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <d3d11.h>
#include "Model.h" // SimpleVertex構造体を使うため

class GraphicsDevice;

struct Particle {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	float life;
	float maxLife;
	float scale;
	float rotation;
	bool active;
	float timeOffset;
};

class ParticleSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	bool Initialize(ID3D11Device* device);
	void Update(float deltaTime, const DirectX::XMFLOAT3& emitterPosition);

	void Render(GraphicsDevice* graphicsDevice,
		const DirectX::XMMATRIX& viewMatrix,
		const DirectX::XMMATRIX& projectionMatrix,
		const DirectX::XMMATRIX& lightViewMatrix,
		const DirectX::XMMATRIX& lightProjectionMatrix);

	void Shutdown();

private:
	bool InitializeBuffers(ID3D11Device* device);
	bool InitializeDepthStencilState(ID3D11Device* device);

	// バッチ描画用のバッファ
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	std::shared_ptr<Texture> m_texture; // テクスチャ単体で持つ

	std::vector<Particle> m_particles;
	float m_spawnTimer;

	ID3D11DepthStencilState* m_depthStencilState;

	// バッチ描画用の頂点配列（CPU側での一時保管場所）
	std::vector<SimpleVertex> m_vertexBatch;
};