#include "OrthoWindow.h"

OrthoWindow::OrthoWindow() {}
OrthoWindow::~OrthoWindow() {}

bool OrthoWindow::Initialize(ID3D11Device* device, int windowWidth, int windowHeight)
{
    std::vector<SimpleVertex> vertices =
    {
        { { -1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    };
    std::vector<unsigned long> indices = { 0, 1, 2, 0, 2, 3 };

    m_quad = std::make_unique<Model>();
    return m_quad->Initialize(device, vertices, indices);
}

void OrthoWindow::Shutdown()
{
    if (m_quad)
    {
        m_quad->Shutdown();
        m_quad.reset();
    }
}

void OrthoWindow::Render(ID3D11DeviceContext* deviceContext)
{
    m_quad->Render(deviceContext);
}