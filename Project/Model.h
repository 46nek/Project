// Model.h (‚±‚Ì“à—e‚ÅŠ®‘S‚É’u‚«Š·‚¦‚Ä‚­‚¾‚³‚¢)

#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>
#include "Texture.h"

struct SimpleVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 Tex;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Binormal;
};

class Model
{
public:
	struct Mesh {
		ID3D11Buffer* vertexBuffer = nullptr;
		ID3D11Buffer* indexBuffer = nullptr;
		int indexCount = 0;
	};

	Model();
	~Model();

	bool Initialize(ID3D11Device* device, const std::vector<SimpleVertex>& vertices, const std::vector<unsigned long>& indices);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	void SetTexture(std::unique_ptr<Texture> texture);
	void SetNormalMap(std::unique_ptr<Texture> normalMap);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);
	DirectX::XMMATRIX GetWorldMatrix() const;
	void SetEmissiveColor(const DirectX::XMFLOAT4& color);
	void SetUseTexture(bool useTexture);

	DirectX::XMFLOAT4 GetEmissiveColor() const;
	bool GetUseTexture() const;

private:
	void RenderBuffers(ID3D11DeviceContext*, const Mesh& mesh);

	std::vector<Mesh> m_meshes;
	std::unique_ptr<Texture> m_texture;
	std::unique_ptr<Texture> m_normalMap;
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMFLOAT4 m_emissiveColor;
	bool m_useTexture;
};