#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    bool Initialize(ID3D11Device* device);
    void Shutdown();

    ID3D11VertexShader* GetVertexShader() const { return m_vertexShader; }
    ID3D11PixelShader* GetPixelShader() const { return m_pixelShader; }
    ID3D11VertexShader* GetDepthVertexShader() const { return m_depthVertexShader; }
    ID3D11InputLayout* GetInputLayout() const { return m_inputLayout; }

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11VertexShader* m_depthVertexShader;
    ID3D11InputLayout* m_inputLayout;
};