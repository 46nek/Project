#pragma once
#include <windows.h>

/**
 * @brief ���Ԍv�����s���A�f���^�^�C�����Ǘ�����N���X
 */
class Timer
{
public:
	Timer();
	~Timer();

	bool Initialize();
	/**
	 * @brief ���t���[���Ăяo���A�f���^�^�C�����v�Z
	 */
	void Tick();

	/**
	 * @brief �O�t���[������̌o�ߎ��Ԃ�b�P�ʂŎ擾
	 * @return float �f���^�^�C���i�b�j
	 */
	float GetDeltaTime() const;

private:
	INT64 m_frequency;
	float m_ticksPerMillisecond;
	INT64 m_startTime;
	float m_deltaTime;
};