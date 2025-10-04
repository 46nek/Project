// main.cpp
#include "Game.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    Game* game;
    bool result;

    // Gameオブジェクトを作成
    game = new Game;
    if (!game)
    {
        return 0;
    }

    // 初期化して実行
    result = game->Initialize();
    if (result)
    {
        game->Run();
    }

    // 終了処理
    game->Shutdown();
    delete game;
    game = nullptr;

    return 0;
}