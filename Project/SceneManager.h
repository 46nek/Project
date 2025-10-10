#pragma once

#include <memory>
#include "Scene.h"
#include "GraphicsDevice.h"
#include "Input.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input);
	void Shutdown();
	void Update(float deltaTime);
	void Render();

private:
	bool ChangeScene(SceneState nextState);

private:
	std::unique_ptr<Scene> m_currentScene;
	GraphicsDevice* m_graphicsDevice;
	Input* m_Input;
};