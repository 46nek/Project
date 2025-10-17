// Project/UI.h

#pragma once

#include "GraphicsDevice.h"
#include "Minimap.h"
#include <vector>
#include <memory>
#include "FW1FontWrapper.h"
#include "Sprite.h"       // Sprite.h ���C���N���[�h
#include "SpriteBatch.h"  // SpriteBatch.h ���C���N���[�h

// �O���錾
class Camera;
class Enemy;
class Orb;

/**
 * @class UI
 * @brief �Q�[������UI�v�f�̊Ǘ��ƕ`����s���N���X
 */
class UI
{
public:
	UI();
	~UI();

	bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Shutdown();
	// ������ Update �֐��̈�����ύX ������
	void Update(float deltaTime, int remainingOrbs, int totalOrbs, float staminaPercentage);
	// ������ �ύX�����܂� ������
	void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs);

private:
	GraphicsDevice* m_graphicsDevice;
	std::unique_ptr<Minimap> m_minimap;

	IFW1Factory* m_fontFactory;
	IFW1FontWrapper* m_fontWrapper;

	std::unique_ptr<Sprite> m_orbIcon;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	int m_remainingOrbs;
	int m_totalOrbs;

	// ������ �ȉ���ǉ� ������
	std::unique_ptr<Sprite> m_staminaBarFrame;
	std::unique_ptr<Sprite> m_staminaBarFill;
	float m_staminaPercentage;
	// ������ �ǉ������܂� ������
};