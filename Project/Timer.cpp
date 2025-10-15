#include "Timer.h"

namespace
{
	// 1秒あたりのミリ秒数を表す定数
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
	// CPUのクロック周波数（ティック/秒）を取得
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0)
	{
		return false;
	}

	// 1ミリ秒あたりのティック数を計算
	m_ticksPerMillisecond = static_cast<float>(m_frequency) / MillisecondsPerSecond;

	// 開始時間を記録
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

void Timer::Tick()
{
	INT64 currentTime;

	// 現在の時間を取得
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	// 前回のフレームからの経過ティック数を計算
	float timeDifference = static_cast<float>(currentTime - m_startTime);

	// デルタタイムをミリ秒単位で計算
	m_deltaTime = timeDifference / m_ticksPerMillisecond;

	// 次のフレームのために開始時間を更新
	m_startTime = currentTime;
}

float Timer::GetDeltaTime() const
{
	// デルタタイムを秒単位で返す
	return m_deltaTime / MillisecondsPerSecond;
}