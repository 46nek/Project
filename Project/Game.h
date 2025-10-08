#pragma once

#include "Window.h"
#include "Direct3D.h"
#include "Input.h"
#include "SceneManager.h"
#include "Timer.h" // Timer‚ð’Ç‰Á
#include <memory>

class Game
{
public:
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;

    Game();
    ~Game();

    bool Initialize(HINSTANCE hInstance);
    void Run();
    void Shutdown();

private:
    bool Update();
    void Render();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<Direct3D> m_direct3D;
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unique_ptr<Timer> m_timer; // Timer‚ð’Ç‰Á
};