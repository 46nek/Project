#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "LightManager.h"
#include "Camera.h"

class Renderer
{
public:
    Renderer(GraphicsDevice* graphicsDevice);
    ~Renderer();

    void RenderScene(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);

private:
    void RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager);
    void RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);
    void DrawModels(const std::vector<std::unique_ptr<Model>>& models);

    GraphicsDevice* m_graphicsDevice;
};