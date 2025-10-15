#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>
#include "SpriteBatch.h"

/**
 * @brief �^�C�g����ʂ̃V�[�����Ǘ�����N���X
 */
class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	std::unique_ptr<Sprite> m_background;
	std::unique_ptr<Sprite> m_titleLogo;
	std::unique_ptr<Sprite> m_pressEnter;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// ������ �ȉ���ǉ� ������
	float m_titleLogoScale;
	float m_pressEnterScale;
	// ������ �ǉ������܂� ������
};