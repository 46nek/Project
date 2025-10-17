// UI.h (この内容で完全に置き換えてください)

#pragma once

#include "GraphicsDevice.h"
#include "Minimap.h"
#include <vector>
#include <memory>
#include "FW1FontWrapper.h" // <--- DXTKの代わりにこちらをインクルード

// 前方宣言
class Camera;
class Enemy;
class Orb;

/**
 * @class UI
 * @brief ゲーム内のUI要素の管理と描画を行うクラス
 */
class UI
{
public:
	UI();
	~UI();

	bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Shutdown();
	void Update(float deltaTime, int remainingOrbs, int totalOrbs);
	void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs);

private:
	GraphicsDevice* m_graphicsDevice;
	std::unique_ptr<Minimap> m_minimap;

	// ▼▼▼ 以下のように変更 ▼▼▼
	IFW1Factory* m_fontFactory;
	IFW1FontWrapper* m_fontWrapper;

	std::unique_ptr<Sprite> m_orbIcon;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch; // アイコン描画用にSpriteBatchは残す

	int m_remainingOrbs;
	int m_totalOrbs;
	// ▲▲▲ 変更ここまで ▲▲▲
};