#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "Model.h"
#include "AStar.h"
#include "Player.h"
#include <d3d11.h>

class Enemy {
public:
	Enemy();
	~Enemy();

	bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData); // à¯êîÇí«â¡
	void Shutdown();
	void Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	Model* GetModel();
	DirectX::XMFLOAT3 GetPosition() const;

private:
	std::unique_ptr<Model> m_model;
	DirectX::XMFLOAT3 m_position;
	float m_speed;
	std::unique_ptr<AStar> m_astar;
	std::vector<DirectX::XMFLOAT2> m_path;

	int m_pathIndex;
	float m_pathCooldown;
};