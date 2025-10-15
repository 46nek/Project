#include "Timer.h"

namespace
{
	// 1�b������̃~���b����\���萔
	constexpr float MillisecondsPerSecond = 1000.0f;
}

Timer::Timer()
	: m_frequency(0),
	m_ticksPerMillisecond(0.0f),
	m_startTime(0),
	m_deltaTime(0.0f)
{
}

Timer::~Timer()
{
}

bool Timer::Initialize()
{
	// CPU�̃N���b�N���g���i�e�B�b�N/�b�j���擾
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0)
	{
		return false;
	}

	// 1�~���b������̃e�B�b�N�����v�Z
	m_ticksPerMillisecond = static_cast<float>(m_frequency) / MillisecondsPerSecond;

	// �J�n���Ԃ��L�^
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

void Timer::Tick()
{
	INT64 currentTime;

	// ���݂̎��Ԃ��擾
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	// �O��̃t���[������̌o�߃e�B�b�N�����v�Z
	float timeDifference = static_cast<float>(currentTime - m_startTime);

	// �f���^�^�C�����~���b�P�ʂŌv�Z
	m_deltaTime = timeDifference / m_ticksPerMillisecond;

	// ���̃t���[���̂��߂ɊJ�n���Ԃ��X�V
	m_startTime = currentTime;
}

float Timer::GetDeltaTime() const
{
	// �f���^�^�C����b�P�ʂŕԂ�
	return m_deltaTime / MillisecondsPerSecond;
}