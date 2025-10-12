#pragma once
#include "GraphicsDevice.h"
#include <memory>
#include <wrl/client.h>

class PostProcess
{
public:
    PostProcess();
    ~PostProcess();

    bool Initialize(GraphicsDevice* graphicsDevice, int screenWidth, int screenHeight);
    void Shutdown();

    // シーンの描画を開始するために、レンダーターゲットを設定します
    void SetRenderTarget(ID3D11DeviceContext* deviceContext);
    // ポストプロセスエフェクトを適用し、最終的な結果をバックバッファに描画します
    void Apply(ID3D11DeviceContext* deviceContext);

private:
    // フルスクリーンのポリゴンを描画するヘルパー関数
    void RenderFullscreenQuad(ID3D11DeviceContext* deviceContext);

    GraphicsDevice* m_graphicsDevice;

    // シーン全体をレンダリングするためのレンダーターゲット
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_sceneTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_sceneRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sceneSRV;

    // ブルーム処理用の一時的なレンダーターゲット
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_bloomTexture1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_bloomRTV1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bloomSRV1;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_bloomTexture2;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_bloomRTV2;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bloomSRV2;

    // フルスクリーンクアッド用の頂点・インデックスバッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

    // ガウシアンブラー用の定数バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_blurBuffer;

    // ぼかし処理の定数バッファ構造体
    struct BlurBufferType
    {
        DirectX::XMFLOAT2 textureSize;
        DirectX::XMFLOAT2 blurDirection;
    };
};