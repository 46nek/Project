// UI.cpp (修正版：この内容で完全に置き換えてください)

#include "UI.h"
#include "Camera.h"
#include "Enemy.h"
#include "Orb.h"
#include <string>

UI::UI()
	: m_graphicsDevice(nullptr),
	m_fontFactory(nullptr),
	m_fontWrapper(nullptr),
	m_remainingOrbs(0),
	m_totalOrbs(0)
{
}

UI::~UI()
{
}

bool UI::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	m_graphicsDevice = graphicsDevice;
	ID3D11Device* device = m_graphicsDevice->GetDevice();

	// ミニマップを初期化
	m_minimap = std::make_unique<Minimap>();
	if (!m_minimap->Initialize(graphicsDevice, mazeData, pathWidth))
	{
		return false;
	}

	// FW1FontWrapperのファクトリとラッパーを作成
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) return false;

	// ▼▼▼ フォントを "Impact" に変更 ▼▼▼
	hr = m_fontFactory->CreateFontWrapper(device, L"Impact", &m_fontWrapper);
	if (FAILED(hr))
	{
		// もし "Impact" が見つからなかった場合、"Arial" にフォールバック
		hr = m_fontFactory->CreateFontWrapper(device, L"Arial", &m_fontWrapper);
		if (FAILED(hr)) return false;
	}

	// アイコン表示用のSpriteBatchとSprite
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());
	m_orbIcon = std::make_unique<Sprite>();
	if (!m_orbIcon->Initialize(device, L"Assets/minimap_orb.png"))
	{
		return false;
	}

	return true;
}

void UI::Shutdown()
{
	if (m_minimap)
	{
		m_minimap->Shutdown();
	}
	if (m_orbIcon)
	{
		m_orbIcon->Shutdown();
	}

	if (m_fontWrapper) m_fontWrapper->Release();
	if (m_fontFactory) m_fontFactory->Release();
}

void UI::Update(float deltaTime, int remainingOrbs, int totalOrbs)
{
	m_remainingOrbs = remainingOrbs;
	m_totalOrbs = totalOrbs;
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs)
{
	// ミニマップの描画
	m_minimap->Render(camera, enemies, orbs);

	// オーブカウンターのアイコン描画
	m_spriteBatch->Begin();

	const DirectX::XMFLOAT2 iconPosition = { 280.0f, 40.0f };
	m_orbIcon->Render(m_spriteBatch.get(), iconPosition, 0.5f);

	m_spriteBatch->End();

	// オーブカウンターのテキスト描画
	if (m_fontWrapper)
	{
		std::wstring remainingText = std::to_wstring(m_remainingOrbs);

		float fontSize = 32.0f;
		float textPosX = iconPosition.x + 30.0f;
		float textPosY = iconPosition.y; // アイコンの中心Y座標
		UINT32 textColor = 0xFFFFFFFF; // 白

		// ▼▼▼ 垂直位置の計算方法とフラグを修正 ▼▼▼
		m_fontWrapper->DrawString(
			m_graphicsDevice->GetDeviceContext(),
			remainingText.c_str(),
			fontSize,
			textPosX,
			textPosY, // Y座標をそのまま指定
			textColor,
			FW1_VCENTER | FW1_RESTORESTATE // FW1_VCENTERで垂直中央揃え
		);
	}
}