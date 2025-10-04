#include "Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

bool Timer::Initialize()
{
    // CPUのクロック周波数（ティック/秒）を取得
    QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
    if (m_frequency == 0)
    {
        return false;
    }

    // 1ミリ秒あたりのティック数を計算
    m_ticksPerMs = (float)(m_frequency / 1000);

    // 開始時間を記録
    QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

    return true;
}

void Timer::Frame()
{
    INT64 currentTime;
    float timeDifference;

    // 現在の時間を取得
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

    // 前回のフレームからの経過時間を計算
    timeDifference = (float)(currentTime - m_startTime);

    // デルタタイムをミリ秒単位で計算
    m_frameTime = timeDifference / m_ticksPerMs;

    // 次のフレームのために開始時間を更新
    m_startTime = currentTime;
}

float Timer::GetTime()
{
    // デルタタイムを秒単位で返す (例: 0.016秒)
    return m_frameTime / 1000.0f;
}