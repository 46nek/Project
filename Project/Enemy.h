#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "AStar.h"
#include "Player.h"
#include "ParticleSystem.h"
#include <d3d11.h>

class GraphicsDevice; 

class Enemy {
public:
	Enemy();
	~Enemy();

	bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData);
	void Shutdown();
	void Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

	void Render(GraphicsDevice* graphicsDevice,
		const DirectX::XMMATRIX& viewMatrix,
		const DirectX::XMMATRIX& projectionMatrix,
		const DirectX::XMMATRIX& lightViewMatrix,
		const DirectX::XMMATRIX& lightProjectionMatrix);

	DirectX::XMFLOAT3 GetPosition() const;

private:
	std::unique_ptr<ParticleSystem> m_particleSystem;
	DirectX::XMFLOAT3 m_position;
	float m_speed;
	std::unique_ptr<AStar> m_astar;
	std::vector<DirectX::XMFLOAT2> m_path;
	int m_pathIndex;
	float m_pathCooldown;
};