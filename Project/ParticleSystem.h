#pragma once
#include "Model.h" // SimpleVertex讒矩菴薙ｒ菴ｿ縺・◆繧・
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <memory>

class GraphicsDevice;

/**
 * @brief 繝代・繝・ぅ繧ｯ繝ｫ讒矩菴・
 */
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

/**
 * @brief 繝代・繝・ぅ繧ｯ繝ｫ繧ｷ繧ｹ繝・Β繧ｯ繝ｩ繧ｹ
 */
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

    // 繝舌ャ繝∵緒逕ｻ逕ｨ縺ｮ繝舌ャ繝輔ぃ
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    std::shared_ptr<Texture> m_texture; // 繝・け繧ｹ繝√Ε蜊倅ｽ薙〒謖√▽

    std::vector<Particle> m_particles;
    float m_spawnTimer;

    ID3D11DepthStencilState* m_depthStencilState;

    // 繝舌ャ繝∵緒逕ｻ逕ｨ縺ｮ鬆らせ驟榊・・・PU蛛ｴ縺ｧ縺ｮ荳譎ゆｿ晉ｮ｡蝣ｴ謇・・
    std::vector<SimpleVertex> m_vertexBatch;
};
