#include "Game.h"
#include <windows.h>
#include <iostream>
#include <fcntl.h>
#include <io.h>

Game* g_game = nullptr;

// コンソールを作成して標準出力をリダイレクトする関数
void CreateConsole()
{
    if (AllocConsole())
    {
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        std::wcout.imbue(std::locale("japanese")); // 日本語表示のための設定
    }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR pScmdline, _In_ int iCmdshow)
{
    CreateConsole();
    auto game = std::make_unique<Game>();

    g_game = game.get();

    if (game->Initialize(hInstance))
    {
        game->Run();
    }
    game->Shutdown();

    return 0;
}