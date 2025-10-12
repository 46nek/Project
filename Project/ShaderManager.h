#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

/**
 * @class ShaderManager
 * @brief HLSLシェーダーファイルのコンパイルと管理
 */
class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    bool Initialize(ID3D11Device* device);
    void Shutdown();

    // ゲッター
    ID3D11VertexShader* GetVertexShader() const { return m_vertexShader; }
    ID3D11PixelShader* GetPixelShader() const { return m_pixelShader; }
    ID3D11VertexShader* GetDepthVertexShader() const { return m_depthVertexShader; }
    ID3D11InputLayout* GetInputLayout() const { return m_inputLayout; }
    ID3D11InputLayout* GetPostProcessInputLayout() const { return m_postProcessInputLayout; }

    ID3D11VertexShader* GetPostProcessVertexShader() const { return m_postProcessVS; }
    ID3D11PixelShader* GetBrightPassPixelShader() const { return m_brightPassPS; }
    ID3D11PixelShader* GetGaussianBlurPixelShader() const { return m_gaussianBlurPS; }
    ID3D11PixelShader* GetCompositePixelShader() const { return m_compositePS; }

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11VertexShader* m_depthVertexShader;
    ID3D11InputLayout* m_inputLayout;
    ID3D11InputLayout* m_postProcessInputLayout;

    ID3D11VertexShader* m_postProcessVS;
    ID3D11PixelShader* m_brightPassPS;
    ID3D11PixelShader* m_gaussianBlurPS;
    ID3D11PixelShader* m_compositePS;
};