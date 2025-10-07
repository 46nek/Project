#pragma once
#include "Scene.h"
#include "Sprite.h" 
#include <memory> 

class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();

	bool Initialize(Direct3D* d3d, Input* input) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	std::unique_ptr<Sprite> m_background; 
	std::unique_ptr<Sprite> m_titleLogo;  
	std::unique_ptr<Sprite> m_pressEnter;
};