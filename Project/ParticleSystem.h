#pragma once
#include "Model.h" 
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <memory>

class GraphicsDevice;

struct Particle {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 velocity;
    float life;
    float maxLife;
    float scale;
    float rotation;
    bool active;
    float timeOffset;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    bool Initialize(ID3D11Device* device);
    void Update(float deltaTime, const DirectX::XMFLOAT3& emitterPosition);

    void Render(GraphicsDevice* graphicsDevice,
        const DirectX::XMMATRIX& viewMatrix,
        const DirectX::XMMATRIX& projectionMatrix,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjectionMatrix);

    void Shutdown();

private:
    bool InitializeBuffers(ID3D11Device* device);
    bool InitializeDepthStencilState(ID3D11Device* device);

    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    std::shared_ptr<Texture> m_texture; 

    std::vector<Particle> m_particles;
    float m_spawnTimer;

    ID3D11DepthStencilState* m_depthStencilState;

    std::vector<SimpleVertex> m_vertexBatch;
};