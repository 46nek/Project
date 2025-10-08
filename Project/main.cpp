// main.cpp
#include "Game.h"
#include <windows.h>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR pScmdline, _In_ int iCmdshow)
{
    auto game = std::make_unique<Game>();
    if (game->Initialize(hInstance))
    {
        game->Run();
    }
    game->Shutdown();

    return 0;
}