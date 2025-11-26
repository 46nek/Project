// Project/UI.cpp

#include "UI.h"
#include "Camera.h"
#include "Enemy.h"
#include "Orb.h"
#include "Game.h" // Game::SCREEN_WIDTH/HEIGHT のために追加
#include <string>

UI::UI()
	: m_graphicsDevice(nullptr),
	m_fontFactory(nullptr),
	m_fontWrapper(nullptr),
	m_remainingOrbs(0),
	m_totalOrbs(0),
	m_staminaPercentage(1.0f)
{
}

UI::~UI()
{
}

bool UI::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	m_graphicsDevice = graphicsDevice;
	ID3D11Device* device = m_graphicsDevice->GetDevice();

	m_minimap = std::make_unique<Minimap>();
	if (!m_minimap->Initialize(graphicsDevice, mazeData, pathWidth))
	{
		return false;
	}

	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) return false;

	hr = m_fontFactory->CreateFontWrapper(device, L"Impact", &m_fontWrapper);
	if (FAILED(hr))
	{
		hr = m_fontFactory->CreateFontWrapper(device, L"Arial", &m_fontWrapper);
		if (FAILED(hr)) return false;
	}

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());
	m_orbIcon = std::make_unique<Sprite>();
	if (!m_orbIcon->Initialize(device, L"Assets/minimap_orb.png"))
	{
		return false;
	}

	// スタミナゲージ用のスプライトを初期化
	// Assets/minimap_frame.png と Assets/minimap_path.png をゲージの画像として再利用します。
	m_staminaBarFrame = std::make_unique<Sprite>();
	if (!m_staminaBarFrame->Initialize(device, L"Assets/minimap_frame.png")) return false;

	m_staminaBarFill = std::make_unique<Sprite>();
	if (!m_staminaBarFill->Initialize(device, L"Assets/minimap_path.png")) return false;

	return true;
}

void UI::Shutdown()
{
	if (m_minimap) m_minimap->Shutdown();
	if (m_orbIcon) m_orbIcon->Shutdown();
	if (m_staminaBarFrame) m_staminaBarFrame->Shutdown();
	if (m_staminaBarFill) m_staminaBarFill->Shutdown();

	if (m_fontWrapper) m_fontWrapper->Release();
	if (m_fontFactory) m_fontFactory->Release();
}

void UI::Update(float deltaTime, int remainingOrbs, int totalOrbs, float staminaPercentage, bool showEnemiesOnMinimap)
{
	m_remainingOrbs = remainingOrbs;
	m_totalOrbs = totalOrbs;
	m_staminaPercentage = staminaPercentage;
	m_showEnemiesOnMinimap = showEnemiesOnMinimap;
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs)
{
	m_minimap->Render(camera, enemies, orbs, specialOrbs, m_showEnemiesOnMinimap);

	// オーブカウンターとスタミナゲージの描画開始
	m_spriteBatch->Begin();

	// --- オーブカウンターのアイコン描画 ---
	const DirectX::XMFLOAT2 iconPosition = { 280.0f, 40.0f };
	// ▼▼▼ エラー箇所を修正 ▼▼▼
	m_orbIcon->Render(m_spriteBatch.get(), iconPosition, 0.5f);
	// ▲▲▲ 修正ここまで ▲▲▲

	// --- スタミナゲージの描画 ---
	const float barWidth = 200.0f;
	const float barHeight = 20.0f;
	const DirectX::XMFLOAT2 barPosition = {
		(Game::SCREEN_WIDTH - barWidth) / 2.0f,
		Game::SCREEN_HEIGHT - 50.0f
	};

	// 背景（フレーム）
	RECT frameRect = {
		(LONG)barPosition.x,
		(LONG)barPosition.y,
		(LONG)(barPosition.x + barWidth),
		(LONG)(barPosition.y + barHeight)
	};
	// ▼▼▼ エラー箇所を修正 ▼▼▼
	m_staminaBarFrame->RenderFill(m_spriteBatch.get(), frameRect);
	// ▲▲▲ 修正ここまで ▲▲▲

	// 塗りつぶし部分
	// スタミナの割合に応じて幅を変える
	RECT fillRect = {
		(LONG)barPosition.x,
		(LONG)barPosition.y,
		(LONG)(barPosition.x + barWidth * m_staminaPercentage),
		(LONG)(barPosition.y + barHeight)
	};

	// スタミナ残量に応じて色を変える
	DirectX::XMFLOAT4 fillColor = { 0.0f, 1.0f, 0.0f, 1.0f }; // 緑
	if (m_staminaPercentage < 0.3f)
	{
		fillColor = { 0.8f, 0.2f, 0.2f, 1.0f }; // 赤
	}

	// RenderFill に色指定のオーバーロードがないため、色を付けて描画します
	m_staminaBarFill->RenderFill(m_spriteBatch.get(), fillRect, fillColor);

	// 描画終了
	m_spriteBatch->End();

	// オーブカウンターのテキスト描画
	if (m_fontWrapper)
	{
		std::wstring remainingText = std::to_wstring(m_remainingOrbs);

		float fontSize = 32.0f;
		float textPosX = iconPosition.x + 30.0f;
		float textPosY = iconPosition.y;
		UINT32 textColor = 0xFFFFFFFF; // 白

		m_fontWrapper->DrawString(
			m_graphicsDevice->GetDeviceContext(),
			remainingText.c_str(),
			fontSize,
			textPosX,
			textPosY,
			textColor,
			FW1_VCENTER | FW1_RESTORESTATE
		);
	}
}

Minimap* UI::GetMinimap() const
{
	return m_minimap.get();
}

void UI::SetMinimapZoom(float zoomLevel) {
	if (m_minimap) {
		m_minimap->SetZoom(zoomLevel);
	}
}