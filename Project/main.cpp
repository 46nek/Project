﻿#include "Game.h"
#include <windows.h>
#include <iostream>
#include <fcntl.h>
#include <io.h>

// グローバル変数
Game* g_game = nullptr;

/**
 * @brief Windowsアプリケーションのエントリーポイント
 */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR pScmdline, _In_ int iCmdshow)
{
	auto game = std::make_unique<Game>();

	g_game = game.get();

	if (game->Initialize(hInstance))
	{
		game->Run();
	}
	game->Shutdown();

	return 0;
}