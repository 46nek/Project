// Game.h
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Direct3D.h"
#include "Input.h"
#include "SceneManager.h" 

class Game
{
public:
	static constexpr int SCREEN_WIDTH = 1280;
	static constexpr int SCREEN_HEIGHT = 720;

	Game();
	~Game();

	bool Initialize();
	void Run();
	void Shutdown();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	std::unique_ptr<Direct3D> m_D3D;
	std::unique_ptr<Input> m_Input;
	std::unique_ptr<SceneManager> m_SceneManager;

	int m_screenWidth, m_screenHeight;
	bool m_isMessageBoxActive;
};

// Windowsプロシージャのための静的ポインタ
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Game* ApplicationHandle = 0;