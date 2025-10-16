// ShaderManager.h

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
	ID3D11VertexShader* GetPostProcessVertexShader() const { return m_postProcessVertexShader; }
	ID3D11PixelShader* GetPixelShader() const { return m_pixelShader; }
	ID3D11PixelShader* GetTexturePixelShader() const { return m_texturePixelShader; }
	ID3D11PixelShader* GetMotionBlurPixelShader() const { return m_motionBlurPixelShader; }
	ID3D11VertexShader* GetDepthVertexShader() const { return m_depthVertexShader; }
	ID3D11InputLayout* GetInputLayout() const { return m_inputLayout; }

	// Å•Å•Å• à»â∫Çí«â¡ Å•Å•Å•
	ID3D11VertexShader* GetSimpleVertexShader() const { return m_simpleVertexShader; }
	ID3D11PixelShader* GetSimplePixelShader() const { return m_simplePixelShader; }
	// Å£Å£Å£ í«â¡Ç±Ç±Ç‹Ç≈ Å£Å£Å£

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11VertexShader* m_postProcessVertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11PixelShader* m_texturePixelShader;
	ID3D11PixelShader* m_motionBlurPixelShader;
	ID3D11VertexShader* m_depthVertexShader;
	ID3D11InputLayout* m_inputLayout;

	ID3D11VertexShader* m_simpleVertexShader;
	ID3D11PixelShader* m_simplePixelShader;
};