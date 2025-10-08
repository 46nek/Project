#pragma once
#include <windows.h>

class Timer
{
public:
    Timer();
    ~Timer();

    bool Initialize();
    void Tick();

    // �f���^�^�C����b�P�ʂŕԂ� (��: 0.016�b)
    float GetDeltaTime() const;

private:
    INT64 m_frequency;
    float m_ticksPerMillisecond;
    INT64 m_startTime;
    float m_deltaTime;
};