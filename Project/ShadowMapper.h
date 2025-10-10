#pragma once
#include <d3d11.h>

class ShadowMapper
{
public:
    static constexpr int SHADOWMAP_WIDTH = 2048;
    static constexpr int SHADOWMAP_HEIGHT = 2048;

    ShadowMapper();
    ~ShadowMapper();

    bool Initialize(ID3D11Device* device);
    void Shutdown();

    void SetRenderTarget(ID3D11DeviceContext* deviceContext);

    ID3D11ShaderResourceView* GetShadowMapSRV() const { return m_shadowMapSRV; }
    ID3D11SamplerState* GetShadowSampleState() const { return m_shadowSampleState; }

private:
    ID3D11Texture2D* m_shadowMapTexture;
    ID3D11DepthStencilView* m_shadowMapDSV;
    ID3D11ShaderResourceView* m_shadowMapSRV;
    ID3D11RasterizerState* m_rasterState;
    ID3D11SamplerState* m_shadowSampleState;
};