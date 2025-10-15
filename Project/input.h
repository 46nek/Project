#pragma once

/**
 * @brief �L�[�{�[�h�ƃ}�E�X�̓��͂��Ǘ�����N���X
 */
class Input
{
public:
	Input();
	~Input();

	void Initialize();
	/**
	 * @brief �t���[���̏I���ɃL�[��Ԃ��X�V
	 */
	void EndFrame();
	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	void MouseMove(int x, int y);

	/**
	 * @brief �w��L�[�����݉�����Ă��邩
	 */
	bool IsKeyDown(unsigned int key);
	/**
	 * @brief �w��L�[�����̃t���[���ŉ����ꂽ�u�Ԃ�
	 */
	bool IsKeyPressed(unsigned int key);
	/**
	 * @brief �O�t���[������̃}�E�X�̈ړ��ʂ��擾
	 */
	void GetMouseDelta(int& x, int& y);

private:
	bool m_keys[256];
	bool m_previousKeys[256];
	int m_mouseX, m_mouseY;
};