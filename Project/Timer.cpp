#include "Timer.h"

Timer::Timer()
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
    m_ticksPerMs = (float)(m_frequency / 1000);

    // �J�n���Ԃ��L�^
    QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

    return true;
}

void Timer::Frame()
{
    INT64 currentTime;
    float timeDifference;

    // ���݂̎��Ԃ��擾
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

    // �O��̃t���[������̌o�ߎ��Ԃ��v�Z
    timeDifference = (float)(currentTime - m_startTime);

    // �f���^�^�C�����~���b�P�ʂŌv�Z
    m_frameTime = timeDifference / m_ticksPerMs;

    // ���̃t���[���̂��߂ɊJ�n���Ԃ��X�V
    m_startTime = currentTime;
}

float Timer::GetTime()
{
    // �f���^�^�C����b�P�ʂŕԂ� (��: 0.016�b)
    return m_frameTime / 1000.0f;
}