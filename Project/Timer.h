#pragma once
#include <windows.h>

/**
 * @brief 譎る俣險域ｸｬ繧定｡後＞縲√ョ繝ｫ繧ｿ繧ｿ繧､繝繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ
 */
class Timer {
public:
	Timer();
	~Timer();

	bool Initialize();
	/**
	 * @brief 豈弱ヵ繝ｬ繝ｼ繝蜻ｼ縺ｳ蜃ｺ縺励√ョ繝ｫ繧ｿ繧ｿ繧､繝繧定ｨ育ｮ・
	 */
	void Tick();

	/**
	 * @brief 蜑阪ヵ繝ｬ繝ｼ繝縺九ｉ縺ｮ邨碁℃譎る俣繧堤ｧ貞腰菴阪〒蜿門ｾ・
	 * @return float 繝・Ν繧ｿ繧ｿ繧､繝・育ｧ抵ｼ・
	 */
	float GetDeltaTime() const;

private:
	INT64 m_frequency;
	float m_ticksPerMillisecond;
	INT64 m_startTime;
	float m_deltaTime;
};
