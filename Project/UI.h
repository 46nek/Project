#pragma once
#include "GraphicsDevice.h"
#include "Minimap.h"
#include <vector>
#include <memory>
#include "FW1FontWrapper.h"
#include "Sprite.h"
#include "SpriteBatch.h"

class Camera;
class Enemy;
class Orb;

class UI
{
public:
	UI();
	~UI();

	bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Shutdown();
	void Update(float deltaTime, int remainingOrbs, int totalOrbs, float staminaPercentage, bool showEnemiesOnMinimap);

	void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs, float alpha = 1.0f);

	Minimap* GetMinimap() const;
	void SetMinimapZoom(float zoomLevel);
private:
	GraphicsDevice* m_graphicsDevice;
	std::unique_ptr<Minimap> m_minimap;
	IFW1Factory* m_fontFactory;
	IFW1FontWrapper* m_fontWrapper;
	std::unique_ptr<Sprite> m_orbIcon;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	int m_remainingOrbs;
	int m_totalOrbs;
	std::unique_ptr<Sprite> m_staminaBarFrame;
	std::unique_ptr<Sprite> m_staminaBarFill;
	float m_staminaPercentage;
	bool m_showEnemiesOnMinimap;
};