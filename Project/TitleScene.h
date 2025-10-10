#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>

class TitleScene : public Scene
{
public:
    TitleScene();
    ~TitleScene();

    bool Initialize(GraphicsDevice* graphicsDevice, Input* input) override; // ïœçX
    void Shutdown() override;
    void Update(float deltaTime) override;
    void Render() override;

private:
    std::unique_ptr<Sprite> m_background;
    std::unique_ptr<Sprite> m_titleLogo;
    std::unique_ptr<Sprite> m_pressEnter;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch; // SpriteBatchÇí«â¡
};