#include "Window.h"
#include "Game.h"
#include <windowsx.h>

extern Game* g_game;

// WndProc縺ｧ菴ｿ縺・◆繧√・髱咏噪繝昴う繝ｳ繧ｿ
static Window* g_windowHandle = nullptr;

Window::Window(LPCWSTR applicationName, int screenWidth, int screenHeight)
	: m_applicationName(applicationName),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_hInstance(nullptr),
	m_hWnd(nullptr),
	m_input(nullptr) {
}

Window::~Window() {
}

bool Window::Initialize(HINSTANCE hInstance, Input* input) {
	g_windowHandle = this;
	m_hInstance = hInstance;
	m_input = input;

	WNDCLASSEX wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc)) {
		return false;
	}

	m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, m_screenWidth, m_screenHeight, nullptr, nullptr, m_hInstance, nullptr);

	if (!m_hWnd) {
		return false;
	}

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01; // Generic Desktop
	rid.usUsage = 0x02;     // Mouse
	rid.dwFlags = 0;
	rid.hwndTarget = m_hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	return true;
}

void Window::Shutdown() {
	ShowCursor(true);
	if (m_hWnd) {
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
	if (m_hInstance) {
		UnregisterClass(m_applicationName, m_hInstance);
		m_hInstance = nullptr;
	}
	g_windowHandle = nullptr;
}

HWND Window::GetHwnd() const {
	return m_hWnd;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	switch (umessage) {
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		if (g_windowHandle) {
			return g_windowHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
		else {
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
}

LRESULT CALLBACK Window::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	switch (umsg) {
	case WM_ACTIVATEAPP:
		if (wparam == FALSE) {
			if (g_game && !g_game->IsPaused() && m_input && m_input->IsCursorLocked()) {
				g_game->SetPaused(true);
			}
		}
		break;

	case WM_LBUTTONDOWN: // 繝槭え繧ｹ蜈･蜉帙ｒInput繧ｯ繝ｩ繧ｹ縺ｫ騾√ｋ
		m_input->KeyDown(VK_LBUTTON);
		if (g_game && g_game->IsPaused()) {
			POINT p = { LOWORD(lparam), HIWORD(lparam) };
			RECT clientRect;
			GetClientRect(hwnd, &clientRect);
			if (PtInRect(&clientRect, p)) {
				g_game->SetPaused(false);
			}
		}
		return 0;

	case WM_LBUTTONUP: // 繝懊ち繝ｳ繧帝屬縺励◆縺薙→繧る夂衍
		m_input->KeyUp(VK_LBUTTON);
		return 0;

	case WM_RBUTTONDOWN: // 蜿ｳ繧ｯ繝ｪ繝・け
		m_input->KeyDown(VK_RBUTTON);
		return 0;

	case WM_RBUTTONUP:
		m_input->KeyUp(VK_RBUTTON);
		return 0;

	case WM_INPUT:
	{
		UINT dwSize = sizeof(RAWINPUT);
		static BYTE lpb[sizeof(RAWINPUT)];
		GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
		RAWINPUT* raw = (RAWINPUT*)lpb;
		if (raw->header.dwType == RIM_TYPEMOUSE) {
			m_input->MouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
		}
		return 0;
	}

	case WM_KEYDOWN:
		m_input->KeyDown((unsigned int)wparam);
		return 0;

	case WM_KEYUP:
		m_input->KeyUp((unsigned int)wparam);
		return 0;

	case WM_SETCURSOR:
		if (g_game && m_input && m_input->IsCursorLocked() && !g_game->IsPaused()) {
			SetCursor(nullptr); // 繧ｫ繝ｼ繧ｽ繝ｫ謠冗判繧偵後↑縺励阪↓縺吶ｋ・域緒逕ｻ縺輔ｌ縺ｪ縺・ｼ・
			return true;     // 蜃ｦ逅・ｮ御ｺ・ｼ・S縺ｮ繝・ヵ繧ｩ繝ｫ繝亥・逅・ｒ縺輔○縺ｪ縺・ｼ・
		}
		// 繧ｿ繧､繝医Ν逕ｻ髱｢繧・・繝ｼ繧ｺ荳ｭ縺ｯbreak縺励※縲＾S縺ｮ繝・ヵ繧ｩ繝ｫ繝亥・逅・DefWindowProc)縺ｫ莉ｻ縺帙ｋ縺薙→縺ｧ遏｢蜊ｰ繧ｫ繝ｼ繧ｽ繝ｫ縺梧緒逕ｻ縺輔ｌ繧・
		break;

	case WM_MOUSEMOVE:
		m_input->SetMousePosition(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}
