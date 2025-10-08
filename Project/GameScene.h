#pragma once
#include <memory>
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
	std::unique_ptr < Camera> m_Camera;
	std::unique_ptr < Model> m_Model;
	std::unique_ptr<Timer> m_Timer;
};