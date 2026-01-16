#pragma once
#include "GraphicsDevice.h"
#include "Minimap.h"
#include <vector>
#include <memory>
#include "FW1FontWrapper.h"
#include "Sprite.h"
#include "SpriteBatch.h"
#include "GameSettings.h"

class Camera;
class Enemy;
class Orb;

class UI {
public:
	UI();
	~UI();

	bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Shutdown();

	void Update(float deltaTime, int remainingOrbs, int totalOrbs, float skillDuration, float skillCooldown, bool isSkillActive, bool showEnemiesOnMinimap);

	// マウス入力と設定更新用の関数を追加
	void UpdatePauseMenu(int& selectIndex, float mx, float my, bool isClicked, bool isDown, int screenWidth, int screenHeight);

	void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs, float alpha = 1.0f);

	Minimap* GetMinimap() const;
	void SetMinimapZoom(float zoomLevel);
	void RenderPauseMenu(int selectIndex, int screenWidth, int screenHeight);

private:
	GraphicsDevice* m_graphicsDevice;
	std::unique_ptr<Minimap> m_minimap;
	IFW1Factory* m_fontFactory;
	IFW1FontWrapper* m_fontWrapper;
	std::unique_ptr<Sprite> m_orbIcon;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	int m_remainingOrbs;
	int m_totalOrbs;

	float m_skillDuration;
	float m_skillCooldown;
	bool m_isSkillActive;

	bool m_showEnemiesOnMinimap;

	// 設定同期・ドラッグ状態用に追加
	bool m_isDraggingVolume = false;
	bool m_isDraggingBright = false; 
	int m_hoverIndex = -1;

	void DrawMenuContent(int selectIndex, float leftMargin, float topMargin, int screenWidth, int screenHeight);
};