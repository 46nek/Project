#pragma once
#include <windows.h>

class Timer
{
public:
    Timer();
    ~Timer();

    bool Initialize();
    void Tick();

    // デルタタイムを秒単位で返す (例: 0.016秒)
    float GetDeltaTime() const;

private:
    INT64 m_frequency;
    float m_ticksPerMillisecond;
    INT64 m_startTime;
    float m_deltaTime;
};