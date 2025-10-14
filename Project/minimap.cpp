#include "Minimap.h"
#include "Game.h" 

Minimap::Minimap()
    : m_graphicsDevice(nullptr),
    m_mazeData(nullptr),
    m_pathWidth(0.0f),
    m_position({ 20.0f, 20.0f }),
    m_viewSize({ 200.0f, 200.0f }),
    m_cellSize(8.0f),
    m_zoomFactor(3.0f),
    m_pathSpriteScale(1.0f),
    m_playerSpriteScale(0.0f)
{
}

Minimap::~Minimap()
{
    Shutdown();
}

bool Minimap::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    m_graphicsDevice = graphicsDevice;
    m_mazeData = &mazeData;
    m_pathWidth = pathWidth;

    ID3D11Device* device = m_graphicsDevice->GetDevice();
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

    // アセットの読み込み
    m_pathSprite = std::make_unique<Sprite>();
    if (!m_pathSprite->Initialize(device, L"Assets/minimap_path.png")) return false;

    m_playerSprite = std::make_unique<Sprite>();
    if (!m_playerSprite->Initialize(device, L"Assets/minimap_player.png")) return false;
    
    m_enemySprite = std::make_unique<Sprite>();
    if (!m_enemySprite->Initialize(device, L"Assets/minimap_enemy.png")) return false; // プレイヤーと同じ画像を流用

    m_frameSprite = std::make_unique<Sprite>();
    if (!m_frameSprite->Initialize(device, L"Assets/minimap_frame.png")) return false;

    // 各スプライトのスケールを個別に計算
    m_pathSpriteScale = m_cellSize / m_pathSprite->GetWidth();
    m_playerSpriteScale = m_cellSize / m_playerSprite->GetWidth();
    m_enemySpriteScale = m_cellSize / m_enemySprite->GetWidth();

    // 描画領域外を切り取るための設定を作成
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.ScissorEnable = TRUE;
    rasterDesc.DepthClipEnable = TRUE;
    HRESULT hr = device->CreateRasterizerState(&rasterDesc, m_scissorRasterizerState.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

void Minimap::Shutdown()
{
    if (m_pathSprite) m_pathSprite->Shutdown();
    if (m_playerSprite) m_playerSprite->Shutdown();
    if (m_enemySprite) m_enemySprite->Shutdown();
    m_scissorRasterizerState.Reset();
}

void Minimap::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies)
{
    if (!m_graphicsDevice || !m_mazeData || !camera) return;

    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();

    // --- 座標計算 ---
    DirectX::XMFLOAT3 playerWorldPos = camera->GetPosition();
    float playerRotation = camera->GetRotation().y * (DirectX::XM_PI / 180.0f);

    DirectX::XMFLOAT2 minimapCenter = {
        m_position.x + m_viewSize.x * 0.5f,
        m_position.y + m_viewSize.y * 0.5f
    };

    float mapHeightInCells = static_cast<float>(m_mazeData->size());

    DirectX::XMFLOAT2 playerMapPixelPos = {
        (playerWorldPos.x / m_pathWidth) * m_cellSize,
        (mapHeightInCells - (playerWorldPos.z / m_pathWidth)) * m_cellSize
    };

    DirectX::XMMATRIX transform =
        DirectX::XMMatrixTranslation(-playerMapPixelPos.x, -playerMapPixelPos.y, 0.0f) *
        DirectX::XMMatrixScaling(m_zoomFactor, m_zoomFactor, 1.0f) *
        DirectX::XMMatrixRotationZ(-playerRotation) *
        DirectX::XMMatrixTranslation(minimapCenter.x, minimapCenter.y, 0.0f);

    // --- 描画実行 ---
    m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

    D3D11_RECT scissorRect = {
        (LONG)m_position.x, (LONG)m_position.y,
        (LONG)(m_position.x + m_viewSize.x), (LONG)(m_position.y + m_viewSize.y)
    };

    // --- 1. フレーム(背景)を一番後ろに描画 ---
    m_spriteBatch->Begin();
    m_frameSprite->RenderFill(m_spriteBatch.get(), scissorRect);
    m_spriteBatch->End();

    // --- 2. 迷路と敵を描画 (クリッピング有効) ---
    deviceContext->RSSetScissorRects(1, &scissorRect);

    m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, nullptr, nullptr, nullptr, m_scissorRasterizerState.Get(), nullptr, transform);
    for (size_t y = 0; y < m_mazeData->size(); ++y)
    {
        for (size_t x = 0; x < (*m_mazeData)[y].size(); ++x)
        {
            if ((*m_mazeData)[y][x] == MazeGenerator::CellType::Path)
            {
                DirectX::XMFLOAT2 cellPos = {
                    x * m_cellSize + m_cellSize * 0.5f,
                    (mapHeightInCells - y) * m_cellSize - m_cellSize * 0.5f
                };
                m_pathSprite->Render(m_spriteBatch.get(), cellPos, m_pathSpriteScale);
            }
        }
    }


    // すべての敵を描画
    for (const auto& enemy : enemies)
    {
        if (enemy)
        {
            DirectX::XMFLOAT3 enemyWorldPos = enemy->GetPosition();

            // 敵の正確なワールド座標から、どのマスにいるかを整数で計算する
            int enemyGridX = static_cast<int>(enemyWorldPos.x / m_pathWidth);
            int enemyGridZ = static_cast<int>(enemyWorldPos.z / m_pathWidth);

            // そのマスの中心座標をピクセル単位で計算する
            DirectX::XMFLOAT2 enemyMapPixelPos = {
            (float)enemyGridX * m_cellSize + m_cellSize * 0.5f,
            (mapHeightInCells - (float)enemyGridZ) * m_cellSize - m_cellSize * 0.5f
            };

            m_enemySprite->Render(m_spriteBatch.get(), enemyMapPixelPos, m_enemySpriteScale * 0.5f, 0.0f, { 1.0f, 0.2f, 0.2f, 1.0f });
        }
    }
    m_spriteBatch->End();


    // プレイヤーの描画 (常に中央)
    m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, nullptr, nullptr, nullptr, m_scissorRasterizerState.Get());
    m_playerSprite->Render(m_spriteBatch.get(), minimapCenter, m_playerSpriteScale * m_zoomFactor * 0.3f, 0.0f);
    m_spriteBatch->End();

    // --- 後処理 ---
    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    deviceContext->RSGetViewports(&numViewports, &viewport);
    D3D11_RECT fullRect = { 0, 0, (LONG)viewport.Width, (LONG)viewport.Height };
    deviceContext->RSSetScissorRects(1, &fullRect);

    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);
}