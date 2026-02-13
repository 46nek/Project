#pragma once
#include "Texture.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>

struct SimpleVertex {
	DirectX::XMFLOAT3 pos;      // Pos -> pos
	DirectX::XMFLOAT4 color;    // Color -> color
	DirectX::XMFLOAT2 tex;      // Tex -> tex
	DirectX::XMFLOAT3 normal;   // Normal -> normal
	DirectX::XMFLOAT3 tangent;  // Tangent -> tangent
	DirectX::XMFLOAT3 binormal; // Binormal -> binormal
};

class Model {
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

	void SetTexture(std::shared_ptr<Texture> texture);
	void SetNormalMap(std::shared_ptr<Texture> normalMap);

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);
	DirectX::XMMATRIX GetWorldMatrix() const;
	void SetEmissiveColor(const DirectX::XMFLOAT4& color);
	void SetUseTexture(bool useTexture);
	void SetUseNormalMap(bool useNormalMap);

	DirectX::XMFLOAT4 GetEmissiveColor() const;
	bool GetUseTexture() const;
	bool GetUseNormalMap() const;
	bool HasNormalMap() const;

	// --- 隕夜倹蜿ｰ繧ｫ繝ｪ繝ｳ繧ｰ逕ｨ縺ｮ髢｢謨ｰ ---
	DirectX::XMFLOAT3 GetBoundingSphereCenter() const;
	float GetBoundingSphereRadius() const;

private:
	void RenderBuffers(ID3D11DeviceContext*, const Mesh& mesh);
	void CalculateBoundingSphere(const std::vector<SimpleVertex>& vertices);

	std::vector<Mesh> m_meshes;

	std::shared_ptr<Texture> m_texture;
	std::shared_ptr<Texture> m_normalMap;

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMFLOAT4 m_emissiveColor;
	bool m_useTexture;
	bool m_useNormalMap;

	// --- 隕夜倹蜿ｰ繧ｫ繝ｪ繝ｳ繧ｰ逕ｨ縺ｮ螟画焚 ---
	DirectX::XMFLOAT3 m_boundingSphereCenter;
	float m_boundingSphereRadius;
};
