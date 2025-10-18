
#pragma once

#include <memory>
#include "Scene.h"
#include "GraphicsDevice.h"
#include "Input.h"
#include "Audio.h" 

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine);
	void Shutdown();
	void Update(float deltaTime);
	void Render();

private:
	bool ChangeScene(SceneState nextState);

	std::unique_ptr<Scene> m_currentScene;
	GraphicsDevice* m_graphicsDevice;
	Input* m_input;
	DirectX::AudioEngine* m_audioEngine; 
};