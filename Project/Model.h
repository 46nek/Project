#pragma once
#include "Direct3D.h"

class Model
{
public:
    Model();
    ~Model();

    bool Initialize(ID3D11Device*);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

    int GetIndexCount();

private:
    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext*);

private:
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
    int m_vertexCount, m_indexCount;
};