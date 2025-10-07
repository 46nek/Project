#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Model.h"
#include "Timer.h"

class GameScene : public Scene
{
public:
	GameScene();
	~GameScene();

	bool Initialize(Direct3D* d3d, Input* input) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	Camera* m_Camera;
	Model* m_Model;
	Timer* m_Timer;
};