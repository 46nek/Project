// Project/UI.cpp

#include "UI.h"
#include "Camera.h"
#include "Enemy.h"
#include "Orb.h"
#include "Game.h"
#include <string>

// (コンストラクタ等は変更なし)
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
	// (変更なし)
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

	m_staminaBarFrame = std::make_unique<Sprite>();
	if (!m_staminaBarFrame->Initialize(device, L"Assets/minimap_frame.png")) return false;

	m_staminaBarFill = std::make_unique<Sprite>();
	if (!m_staminaBarFill->Initialize(device, L"Assets/minimap_path.png")) return false;

	return true;
}

void UI::Shutdown()
{
	// (変更なし)
	if (m_minimap) m_minimap->Shutdown();
	if (m_orbIcon) m_orbIcon->Shutdown();
	if (m_staminaBarFrame) m_staminaBarFrame->Shutdown();
	if (m_staminaBarFill) m_staminaBarFill->Shutdown();

	if (m_fontWrapper) m_fontWrapper->Release();
	if (m_fontFactory) m_fontFactory->Release();
}

void UI::Update(float deltaTime, int remainingOrbs, int totalOrbs, float staminaPercentage, bool showEnemiesOnMinimap)
{
	// (変更なし)
	m_remainingOrbs = remainingOrbs;
	m_totalOrbs = totalOrbs;
	m_staminaPercentage = staminaPercentage;
	m_showEnemiesOnMinimap = showEnemiesOnMinimap;
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs, float alpha)
{
	// 透明度が0以下なら描画しない
	if (alpha <= 0.0f) return;

	// ミニマップにAlphaを渡す
	m_minimap->Render(camera, enemies, orbs, specialOrbs, m_showEnemiesOnMinimap, alpha);

	// オーブカウンターとスタミナゲージの描画開始
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());

	// --- オーブカウンターのアイコン描画 ---
	const DirectX::XMFLOAT2 iconPosition = { 280.0f, 40.0f };
	m_orbIcon->Render(m_spriteBatch.get(), iconPosition, 0.5f, 0.0f, { 1.0f, 1.0f, 1.0f, alpha });

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
	m_staminaBarFrame->RenderFill(m_spriteBatch.get(), frameRect, { 1.0f, 1.0f, 1.0f, alpha });

	// 塗りつぶし部分
	RECT fillRect = {
		(LONG)barPosition.x,
		(LONG)barPosition.y,
		(LONG)(barPosition.x + barWidth * m_staminaPercentage),
		(LONG)(barPosition.y + barHeight)
	};

	DirectX::XMFLOAT4 fillColor = { 0.0f, 1.0f, 0.0f, alpha }; // 緑
	if (m_staminaPercentage < 0.3f)
	{
		fillColor = { 0.8f, 0.2f, 0.2f, alpha }; // 赤
	}

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

		UINT32 alphaInt = static_cast<UINT32>(alpha * 255.0f);
		UINT32 textColor = (alphaInt << 24) | 0x00FFFFFF;

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