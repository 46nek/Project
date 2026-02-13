#include "Minimap.h"
#include "Game.h"
#include <cmath>

Minimap::Minimap()
	: m_graphicsDevice(nullptr),
	m_mazeData(nullptr),
	m_pathWidth(0.0f),
	m_position({ 20.0f, 20.0f }),
	m_viewSize({ 200.0f, 200.0f }),
	m_cellSize(8.0f),
	m_zoomFactor(0.5f),
	m_pathSpriteScale(1.0f),
	m_playerSpriteScale(0.0f),
	m_orbSpriteScale(0.0f),
	m_orbArrowSpriteScale(0.0f),
	m_enemySpriteScale(0.0f) {
}

Minimap::~Minimap() {
	Shutdown();
}

bool Minimap::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	// 螟画峩縺ｪ縺・
	m_graphicsDevice = graphicsDevice;
	m_mazeData = &mazeData;
	m_pathWidth = pathWidth;

	if (!m_mazeData->empty()) {
		m_visitedCells.assign(m_mazeData->size(), std::vector<bool>((*m_mazeData)[0].size(), false));
	}

	ID3D11Device* device = m_graphicsDevice->GetDevice();
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	if (!m_pathSprite) { m_pathSprite = std::make_unique<Sprite>(); }
	if (!m_pathSprite->Initialize(device, L"Assets/minimap_path.png")) { return false; }

	if (!m_playerSprite) { m_playerSprite = std::make_unique<Sprite>(); }
	if (!m_playerSprite->Initialize(device, L"Assets/minimap_player.png")) { return false; }

	if (!m_enemySprite) { m_enemySprite = std::make_unique<Sprite>(); }
	if (!m_enemySprite->Initialize(device, L"Assets/minimap_enemy.png")) { return false; }

	if (!m_orbSprite) { m_orbSprite = std::make_unique<Sprite>(); }
	if (!m_orbSprite->Initialize(device, L"Assets/minimap_orb.png")) { return false; }

	if (!m_frameSprite) { m_frameSprite = std::make_unique<Sprite>(); }
	if (!m_frameSprite->Initialize(device, L"Assets/minimap_frame.png")) { return false; }

	if (!m_orbArrowSprite) { m_orbArrowSprite = std::make_unique<Sprite>(); }
	if (!m_orbArrowSprite->Initialize(device, L"Assets/orb_arrow.png")) { return false; }

	m_pathSpriteScale = m_cellSize / m_pathSprite->GetWidth();
	m_playerSpriteScale = m_cellSize / m_playerSprite->GetWidth();
	m_enemySpriteScale = m_cellSize / m_enemySprite->GetWidth();
	m_orbSpriteScale = m_cellSize / m_orbSprite->GetWidth();
	m_orbArrowSpriteScale = m_cellSize / m_orbArrowSprite->GetWidth();

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.ScissorEnable = TRUE;
	rasterDesc.DepthClipEnable = TRUE;
	HRESULT hr = device->CreateRasterizerState(&rasterDesc, m_scissorRasterizerState.GetAddressOf());
	if (FAILED(hr)) { return false; }

	return true;
}

void Minimap::Shutdown() {
	// 螟画峩縺ｪ縺・
	if (m_pathSprite) { m_pathSprite->Shutdown(); }
	if (m_playerSprite) { m_playerSprite->Shutdown(); }
	if (m_enemySprite) { m_enemySprite->Shutdown(); }
	if (m_orbSprite) { m_orbSprite->Shutdown(); }
	if (m_frameSprite) { m_frameSprite->Shutdown(); }
	if (m_orbArrowSprite) { m_orbArrowSprite->Shutdown(); }
	m_scissorRasterizerState.Reset();
	m_visitedCells.clear();
}

void Minimap::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs, bool showEnemies, float alpha) {
	if (alpha <= 0.0f) { return; }
	if (!m_graphicsDevice || !m_mazeData || !camera) { return; }
	if (m_visitedCells.empty()) { return; }

	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();

	// --- 蠎ｧ讓呵ｨ育ｮ・---
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

	// --- 險ｪ蝠冗憾諷九・譖ｴ譁ｰ ---
	int playerGridX = static_cast<int>(playerWorldPos.x / m_pathWidth);
	int playerGridZ = static_cast<int>(playerWorldPos.z / m_pathWidth);

	int viewRange = 0.5;
	for (int z = playerGridZ - viewRange; z <= playerGridZ + viewRange; ++z) {
		for (int x = playerGridX - viewRange; x <= playerGridX + viewRange; ++x) {
			if (z >= 0 && z < static_cast<int>(m_visitedCells.size()) &&
				x >= 0 && x < static_cast<int>(m_visitedCells[z].size())) {
				m_visitedCells[z][x] = true;
			}
		}
	}

	DirectX::XMMATRIX transform =
		DirectX::XMMatrixTranslation(-playerMapPixelPos.x, -playerMapPixelPos.y, 0.0f) *
		DirectX::XMMatrixScaling(m_zoomFactor, m_zoomFactor, 1.0f) *
		DirectX::XMMatrixRotationZ(-playerRotation) *
		DirectX::XMMatrixTranslation(minimapCenter.x, minimapCenter.y, 0.0f);

	// --- 謠冗判螳溯｡・---
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

	D3D11_RECT scissorRect = {
		(LONG)m_position.x, (LONG)m_position.y,
		(LONG)(m_position.x + m_viewSize.x), (LONG)(m_position.y + m_viewSize.y)
	};

	// --- 1. 繝輔Ξ繝ｼ繝(閭梧勹) ---
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());
	m_frameSprite->RenderFill(m_spriteBatch.get(), scissorRect, { 1.0f, 1.0f, 1.0f, alpha });
	m_spriteBatch->End();

	// --- 2. 繝槭ャ繝苓ｦ∫ｴ (繧ｯ繝ｪ繝・ヴ繝ｳ繧ｰ譛牙柑) ---
	deviceContext->RSSetScissorRects(1, &scissorRect);

	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState(), nullptr, nullptr, m_scissorRasterizerState.Get(), nullptr, transform);

	for (size_t y = 0; y < m_mazeData->size(); ++y) {
		for (size_t x = 0; x < (*m_mazeData)[y].size(); ++x) {
			if (!m_visitedCells[y][x]) { continue; }

			if ((*m_mazeData)[y][x] == MazeGenerator::CellType::Path) {
				DirectX::XMFLOAT2 cellPos = {
					x * m_cellSize + m_cellSize * 0.5f,
					(mapHeightInCells - y) * m_cellSize - m_cellSize * 0.5f
				};
				m_pathSprite->Render(m_spriteBatch.get(), cellPos, m_pathSpriteScale, 0.0f, { 1.0f, 1.0f, 1.0f, alpha });
			}
		}
	}

	for (const auto& enemy : enemies) {
		if (enemy) {
			DirectX::XMFLOAT3 enemyWorldPos = enemy->GetPosition();
			int enemyGridX = static_cast<int>(enemyWorldPos.x / m_pathWidth);
			int enemyGridZ = static_cast<int>(enemyWorldPos.z / m_pathWidth);

			DirectX::XMFLOAT2 enemyMapPixelPos = {
				(float)enemyGridX * m_cellSize + m_cellSize * 0.5f,
				(mapHeightInCells - (float)enemyGridZ) * m_cellSize - m_cellSize * 0.5f
			};
			m_enemySprite->Render(m_spriteBatch.get(), enemyMapPixelPos, m_enemySpriteScale * 0.5f, 0.0f, { 1.0f, 0.2f, 0.2f, alpha });
		}
	}

	for (const auto& orb : orbs) {
		if (orb && !orb->IsCollected()) {
			DirectX::XMFLOAT3 orbWorldPos = orb->GetPosition();
			int orbGridX = static_cast<int>(orbWorldPos.x / m_pathWidth);
			int orbGridZ = static_cast<int>(orbWorldPos.z / m_pathWidth);

			DirectX::XMFLOAT2 orbMapPixelPos = {
				(float)orbGridX * m_cellSize + m_cellSize * 0.5f,
				(mapHeightInCells - (float)orbGridZ) * m_cellSize - m_cellSize * 0.5f
			};
			m_orbSprite->Render(m_spriteBatch.get(), orbMapPixelPos, m_orbSpriteScale * 0.4f, 0.0f, { 0.6f, 0.8f, 1.0f, alpha });
		}
	}

	m_spriteBatch->End();

	// --- 譛繧りｿ代＞繧ｪ繝ｼ繝悶・謗｢邏｢ ---
	float minDistanceSq = FLT_MAX;
	DirectX::XMFLOAT3 targetPos = {};
	bool foundTarget = false;

	auto CheckClosestOrb = [&](const auto& orbList) {
		for (const auto& orb : orbList) {
			if (orb && !orb->IsCollected()) {
				DirectX::XMFLOAT3 pos = orb->GetPosition();
				float dx = pos.x - playerWorldPos.x;
				float dz = pos.z - playerWorldPos.z;
				float distSq = dx * dx + dz * dz;
				if (distSq < minDistanceSq) {
					minDistanceSq = distSq;
					targetPos = pos;
					foundTarget = true;
				}
			}
		}
		};

	CheckClosestOrb(orbs);

	// --- 遏｢蜊ｰ縺ｮ謠冗判 ---
	if (foundTarget) {
		float dx = targetPos.x - playerWorldPos.x;
		float dz = targetPos.z - playerWorldPos.z;
		float angleToOrb = atan2f(dx, dz);
		float arrowRotation = angleToOrb - playerRotation;

		float distPixels = sqrtf(minDistanceSq) / m_pathWidth * m_cellSize * m_zoomFactor;
		float relativeX = distPixels * sinf(arrowRotation);
		float relativeY = -distPixels * cosf(arrowRotation);
		float limitX = m_viewSize.x * 0.5f;
		float limitY = m_viewSize.y * 0.5f;

		if (fabsf(relativeX) > limitX || fabsf(relativeY) > limitY) {
			float radius = 15.0f;
			DirectX::XMFLOAT2 arrowPos = minimapCenter;
			arrowPos.x += sinf(arrowRotation) * radius;
			arrowPos.y -= cosf(arrowRotation) * radius;

			m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState(), nullptr, nullptr, m_scissorRasterizerState.Get());
			m_orbArrowSprite->Render(m_spriteBatch.get(), arrowPos, m_orbArrowSpriteScale * 1.2f, arrowRotation, { 1.0f, 1.0f, 1.0f, alpha });
			m_spriteBatch->End();
		}
	}

	// 繝励Ξ繧､繝､繝ｼ縺ｮ謠冗判 (蟶ｸ縺ｫ荳ｭ螟ｮ)
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState(), nullptr, nullptr, m_scissorRasterizerState.Get());
	m_playerSprite->Render(m_spriteBatch.get(), minimapCenter, m_playerSpriteScale * m_zoomFactor * 0.3f, 0.0f, { 1.0f, 1.0f, 1.0f, alpha });
	m_spriteBatch->End();

	// --- 蠕悟・逅・---
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	deviceContext->RSGetViewports(&numViewports, &viewport);
	D3D11_RECT fullRect = { 0, 0, (LONG)viewport.Width, (LONG)viewport.Height };
	deviceContext->RSSetScissorRects(1, &fullRect);

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);
}

void Minimap::SetZoom(float zoomFactor) {
	m_zoomFactor = zoomFactor;
}
