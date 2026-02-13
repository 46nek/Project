#include "Timer.h"

namespace {
	// 1遘偵≠縺溘ｊ縺ｮ繝溘Μ遘呈焚繧定｡ｨ縺吝ｮ壽焚
	constexpr float MILLISECONDS_PER_SECOND = 1000.0f;
}

Timer::Timer()
	: m_frequency(0),
	m_ticksPerMillisecond(0.0f),
	m_startTime(0),
	m_deltaTime(0.0f) {
}

Timer::~Timer() {
}

bool Timer::Initialize() {
	// CPU縺ｮ繧ｯ繝ｭ繝・け蜻ｨ豕｢謨ｰ・医ユ繧｣繝・け/遘抵ｼ峨ｒ蜿門ｾ・
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0) {
		return false;
	}

	// 1繝溘Μ遘偵≠縺溘ｊ縺ｮ繝・ぅ繝・け謨ｰ繧定ｨ育ｮ・
	m_ticksPerMillisecond = static_cast<float>(m_frequency) / MILLISECONDS_PER_SECOND;

	// 髢句ｧ区凾髢薙ｒ險倬鹸
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

void Timer::Tick() {
	INT64 currentTime;

	// 迴ｾ蝨ｨ縺ｮ譎る俣繧貞叙蠕・
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	// 蜑榊屓縺ｮ繝輔Ξ繝ｼ繝縺九ｉ縺ｮ邨碁℃繝・ぅ繝・け謨ｰ繧定ｨ育ｮ・
	float timeDifference = static_cast<float>(currentTime - m_startTime);

	// 繝・Ν繧ｿ繧ｿ繧､繝繧偵Α繝ｪ遘貞腰菴阪〒險育ｮ・
	m_deltaTime = timeDifference / m_ticksPerMillisecond;

	// 谺｡縺ｮ繝輔Ξ繝ｼ繝縺ｮ縺溘ａ縺ｫ髢句ｧ区凾髢薙ｒ譖ｴ譁ｰ
	m_startTime = currentTime;
}

float Timer::GetDeltaTime() const {
	// 繝・Ν繧ｿ繧ｿ繧､繝繧堤ｧ貞腰菴阪〒霑斐☆
	return m_deltaTime / MILLISECONDS_PER_SECOND;
}
