#pragma once
#include <vector>
#include <string>
#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Texture.h"

struct SimpleVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT2 Tex;
    DirectX::XMFLOAT3 Normal;
};

class Model
{
public:
    // Mesh�\���̂�public�Ɉړ�
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
    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);
    void SetScale(float x, float y, float z);
    DirectX::XMMATRIX GetWorldMatrix() const; 

private:
    void RenderBuffers(ID3D11DeviceContext*, const Mesh& mesh);

    std::vector<Mesh> m_meshes;
    std::unique_ptr<Texture> m_texture;
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};