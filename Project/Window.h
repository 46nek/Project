#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Input.h"

/**
 * @brief ウィンドウの生成と管理を行うクラス
 */
class Window {
public:
	Window(LPCWSTR applicationName, int screenWidth, int screenHeight);
	~Window();

	/**
	 * @brief ウィンドウを初期化
	 * @param hInstance アプリケーションのインスタンスハンドル
	 * @param input 入力管理クラスのインスタンス
	 * @return 初期化が成功した場合は true
	 */
	bool Initialize(HINSTANCE hInstance, Input* input);

	/**
	 * @brief ウィンドウを破棄
	 */
	void Shutdown();

	/**
	 * @brief ウィンドウハンドルを取得
	 * @return HWND ウィンドウハンドル
	 */
	HWND GetHwnd() const;

private:
	/**
	 * @brief ウィンドウプロシージャ
	 */
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	/**
	 * @brief メッセージハンドラ
	 */
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	LPCWSTR m_applicationName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	Input* m_input;
	int m_screenWidth;
	int m_screenHeight;
};