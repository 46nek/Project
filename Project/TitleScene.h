#pragma once
#include "Scene.h"

class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();

	bool Initialize(Direct3D* d3d, Input* input) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;
};