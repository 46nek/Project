#pragma once
#include <windows.h>

/**
 * @brief 時間計測を行い、デルタタイムを管理するクラス
 */
class Timer {
public:
	Timer();
	~Timer();

	bool Initialize();
	/**
	 * @brief 毎フレーム呼び出し、デルタタイムを計算
	 */
	void Tick();

	/**
	 * @brief 前フレームからの経過時間を秒単位で取得
	 * @return float デルタタイム（秒）
	 */
	float GetDeltaTime() const;

private:
	INT64 m_frequency;
	float m_ticksPerMillisecond;
	INT64 m_startTime;
	float m_deltaTime;
};