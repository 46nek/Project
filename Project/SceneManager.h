#pragma once

#include <memory>
#include "Scene.h"
#include "Direct3D.h"
#include "Input.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	bool Initialize(Direct3D* d3d, Input* input);
	void Shutdown();
	void Update(float deltaTime);
	void Render();

private:
	void ChangeScene(SceneState nextState);

private:
	std::unique_ptr<Scene> m_currentScene; // Œ»İ‚ÌƒV[ƒ“‚ğ•Û
	Direct3D* m_D3D;
	Input* m_Input;
};