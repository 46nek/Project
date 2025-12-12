#pragma once
#include <d3d11.h>
#include "Model.h"
#include <memory>

class OrthoWindow {
public:
	OrthoWindow();
	~OrthoWindow();

	bool Initialize(ID3D11Device* device, int windowWidth, int windowHeight);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

private:
	std::unique_ptr<Model> m_quad;
};