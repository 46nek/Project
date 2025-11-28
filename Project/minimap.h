#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <cfloat>
#include "GraphicsDevice.h"
#include "MazeGenerator.h"
#include "Sprite.h"
#include "Camera.h"
#include "SpriteBatch.h"
#include "Enemy.h"
#include "Orb.h"

class Minimap
{
public:
	Minimap();
	~Minimap();

	bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Shutdown();
	// alphaà¯êîÇí«â¡
	void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs, bool showEnemies, float alpha);
	void SetZoom(float zoomFactor);

private:
	GraphicsDevice* m_graphicsDevice;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<Sprite> m_pathSprite;
	std::unique_ptr<Sprite> m_playerSprite;
	std::unique_ptr<Sprite> m_enemySprite;
	std::unique_ptr<Sprite> m_orbSprite;
	std::unique_ptr<Sprite> m_frameSprite;
	std::unique_ptr<Sprite> m_orbArrowSprite;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_scissorRasterizerState;

	const std::vector<std::vector<MazeGenerator::CellType>>* m_mazeData;
	std::vector<std::vector<bool>> m_visitedCells;

	float m_pathWidth;

	DirectX::XMFLOAT2 m_position;
	DirectX::XMFLOAT2 m_viewSize;
	float m_cellSize;
	float m_zoomFactor;
	float m_pathSpriteScale;
	float m_playerSpriteScale;
	float m_enemySpriteScale;
	float m_orbSpriteScale;
	float m_orbArrowSpriteScale;
};