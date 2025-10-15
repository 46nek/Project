#pragma once
#include <memory>
#include "Window.h"
#include "GraphicsDevice.h"
#include "Input.h"
#include "SceneManager.h"
#include "Timer.h"

/**
 * @brief �Q�[���S�̂̊Ǘ����s�����C���N���X
 */
class Game
{
public:
	// �萔
	static constexpr int SCREEN_WIDTH = 1280;
	static constexpr int SCREEN_HEIGHT = 720;

	Game();
	~Game();

	/**
	 * @brief �Q�[���̏���������
	 * @param hInstance �A�v���P�[�V�����̃C���X�^���X�n���h��
	 * @return �����������������ꍇ�� true
	 */
	bool Initialize(HINSTANCE hInstance);

	/**
	 * @brief �Q�[���̃��C�����[�v�����s
	 */
	void Run();

	/**
	 * @brief �Q�[���̏I������
	 */
	void Shutdown();

	/**
	 * @brief �Q�[���̃|�[�Y��Ԃ�ݒ�
	 * @param isPaused �|�[�Y��Ԃɂ���ꍇ�� true
	 */
	void SetPaused(bool isPaused);

	/**
	 * @brief ���݂̃|�[�Y��Ԃ��擾
	 * @return �|�[�Y���̏ꍇ�� true
	 */
	bool IsPaused() const;

private:
	/**
	 * @brief �t���[�����Ƃ̍X�V����
	 * @return �Q�[���̎��s�𑱂���ꍇ�� true
	 */
	bool Update();

	/**
	 * @brief �t���[�����Ƃ̕`�揈��
	 */
	void Render();

	// �����o�[�ϐ�
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Input> m_input;
	std::unique_ptr<GraphicsDevice> m_graphicsDevice;
	std::unique_ptr<SceneManager> m_sceneManager;
	std::unique_ptr<Timer> m_timer;
	bool m_isPaused;
};