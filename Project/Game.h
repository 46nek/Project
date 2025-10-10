#pragma once

#include "Window.h"
#include "GraphicsDevice.h"
#include "Input.h"
#include "SceneManager.h"
#include "Timer.h"
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
    std::unique_ptr<GraphicsDevice> m_graphicsDevice;
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unique_ptr<Timer> m_timer;
};