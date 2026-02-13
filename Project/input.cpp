#include "Input.h"
#include <algorithm> 
#include <iterator>

Input::Input() :
	m_mouseX(0), m_mouseY(0), m_absMouseX(0), m_absMouseY(0),
	m_isCursorLocked(false), m_isCursorVisible(true) 
{
}

Input::~Input() {
}

void Input::Initialize() {
	std::fill(std::begin(m_keys), std::end(m_keys), false);
	std::fill(std::begin(m_previousKeys), std::end(m_previousKeys), false);

	// 繝槭え繧ｹ遘ｻ蜍暮㍼縺ｪ縺ｩ繧貞・譛溷喧
	m_mouseX = 0;
	m_mouseY = 0;
	m_absMouseX = 0;
	m_absMouseY = 0;
}

void Input::EndFrame() {
	for (int i = 0; i < KEY_COUNT; i++) {
		m_previousKeys[i] = m_keys[i];
	}

	// 谺｡縺ｮ繝輔Ξ繝ｼ繝縺ｮ縺溘ａ縺ｫ繝槭え繧ｹ遘ｻ蜍暮㍼繧偵Μ繧ｻ繝・ヨ
	m_mouseX = 0;
	m_mouseY = 0;
}

void Input::Update(HWND hwnd) {
	// 繧ｫ繝ｼ繧ｽ繝ｫ繝ｭ繝・け縺梧怏蜉ｹ縺ｪ繧峨√き繝ｼ繧ｽ繝ｫ繧堤判髱｢荳ｭ螟ｮ縺ｫ謌ｻ縺・
	if (m_isCursorLocked && hwnd) {
		// 繧ｦ繧｣繝ｳ繝峨え縺後い繧ｯ繝・ぅ繝悶↑蝣ｴ蜷医・縺ｿ螳溯｡鯉ｼ磯撼繧｢繧ｯ繝・ぅ繝匁凾縺ｫ繝槭え繧ｹ繧貞･ｪ繧上↑縺・ｈ縺・↓・・
		if (GetForegroundWindow() == hwnd) {
			// 繧ｦ繧｣繝ｳ繝峨え縺ｮ荳ｭ螟ｮ蠎ｧ讓吶ｒ險育ｮ・
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT center;
			center.x = (rect.right - rect.left) / 2;
			center.y = (rect.bottom - rect.top) / 2;

			// 繧ｹ繧ｯ繝ｪ繝ｼ繝ｳ蠎ｧ讓吶↓螟画鋤縺励※繧ｻ繝・ヨ
			ClientToScreen(hwnd, &center);
			SetCursorPos(center.x, center.y);
		}
	}
}

void Input::KeyDown(unsigned int input) {
	// 繧ｭ繝ｼ縺梧款縺輔ｌ縺溘％縺ｨ繧定ｨ倬鹸
	m_keys[input] = true;
}

void Input::KeyUp(unsigned int input) {
	// 繧ｭ繝ｼ縺碁屬縺輔ｌ縺溘％縺ｨ繧定ｨ倬鹸
	m_keys[input] = false;
}

void Input::MouseMove(int x, int y) {
	// 蜑阪・繝輔Ξ繝ｼ繝縺九ｉ縺ｮ遘ｻ蜍暮㍼繧貞刈邂・
	m_mouseX += x;
	m_mouseY += y;
}

void Input::SetMousePosition(int x, int y) {
	m_absMouseX = x;
	m_absMouseY = y;
}

void Input::GetMousePosition(int& x, int& y) {
	x = m_absMouseX;
	y = m_absMouseY;
}

bool Input::IsKeyDown(unsigned int key) {
	// 謖・ｮ壹＆繧後◆繧ｭ繝ｼ縺梧款縺輔ｌ縺ｦ縺・ｋ縺九←縺・°縺ｮ迥ｶ諷九ｒ霑斐☆
	return m_keys[key];
}

bool Input::IsKeyPressed(unsigned int key) {
	// 迴ｾ蝨ｨ縺ｯ謚ｼ縺輔ｌ縺ｦ縺・※縲∝燕縺ｮ繝輔Ξ繝ｼ繝縺ｧ縺ｯ謚ｼ縺輔ｌ縺ｦ縺・↑縺九▲縺溷ｴ蜷医↓true繧定ｿ斐☆
	return m_keys[key] && !m_previousKeys[key];
}

void Input::GetMouseDelta(int& x, int& y) {
	x = m_mouseX;
	y = m_mouseY;
}

void Input::SetCursorVisible(bool visible) {
	// 迥ｶ諷九′螟峨ｏ繧句ｴ蜷医・縺ｿAPI繧貞他縺ｶ・・howCursor縺ｯ蜀・Κ繧ｫ繧ｦ繝ｳ繧ｿ繧呈戟縺､縺溘ａ・・
	if (m_isCursorVisible != visible) {
		::ShowCursor(visible ? TRUE : FALSE);
		m_isCursorVisible = visible;
	}
}

void Input::SetCursorLock(bool lock) {
	m_isCursorLocked = lock;

	// 繝ｭ繝・け縺吶ｋ縺ｨ縺阪・繧ｫ繝ｼ繧ｽ繝ｫ繧呈ｶ医☆縺ｮ縺御ｸ闊ｬ逧・
	if (lock) {
		SetCursorVisible(false);
	}
	else {
		SetCursorVisible(true);
	}
}

bool Input::IsCursorLocked() const {
	return m_isCursorLocked;
}
