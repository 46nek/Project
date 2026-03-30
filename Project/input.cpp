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

    m_mouseX = 0;
    m_mouseY = 0;
    m_absMouseX = 0;
    m_absMouseY = 0;
}

void Input::EndFrame() {
    for (int i = 0; i < KEY_COUNT; i++) {
        m_previousKeys[i] = m_keys[i];
    }

    m_mouseX = 0;
    m_mouseY = 0;
}

void Input::Update(HWND hwnd) {
    if (m_isCursorLocked && hwnd) {
        if (GetForegroundWindow() == hwnd) {
            RECT rect;
            if (GetClientRect(hwnd, &rect)) {
                POINT center;
                center.x = (rect.right - rect.left) / 2;
                center.y = (rect.bottom - rect.top) / 2;

                ClientToScreen(hwnd, &center);
                SetCursorPos(center.x, center.y);
            }
        }
    }
}

void Input::KeyDown(unsigned int input) {
    if (input < KEY_COUNT) {
        m_keys[input] = true;
    }
}

void Input::KeyUp(unsigned int input) {
    if (input < KEY_COUNT) {
        m_keys[input] = false;
    }
}

void Input::MouseMove(int x, int y) {
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
    return (key < KEY_COUNT) ? m_keys[key] : false;
}

bool Input::IsKeyPressed(unsigned int key) {
    if (key >= KEY_COUNT) return false;
    return m_keys[key] && !m_previousKeys[key];
}

void Input::GetMouseDelta(int& x, int& y) {
    x = m_mouseX;
    y = m_mouseY;
}

void Input::SetCursorVisible(bool visible) {
    if (m_isCursorVisible != visible) {
        ::ShowCursor(visible ? TRUE : FALSE);
        m_isCursorVisible = visible;
    }
}

void Input::SetCursorLock(bool lock) {
    m_isCursorLocked = lock;

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