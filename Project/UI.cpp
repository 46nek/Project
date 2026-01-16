#include "UI.h"
#include "Camera.h"
#include "Enemy.h"
#include "Orb.h"
#include "Game.h"
#include "AssetPaths.h"
#include <string>
#include <cstdio> // swprintf_s用

UI::UI()
	: m_graphicsDevice(nullptr),
	m_fontFactory(nullptr),
	m_fontWrapper(nullptr),
	m_remainingOrbs(0),
	m_totalOrbs(0),
	m_skillDuration(0.0f),
	m_skillCooldown(0.0f),
	m_isSkillActive(false) {
}

UI::~UI() {
}

bool UI::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	m_graphicsDevice = graphicsDevice;
	ID3D11Device* device = m_graphicsDevice->GetDevice();

	m_minimap = std::make_unique<Minimap>();
	if (!m_minimap->Initialize(graphicsDevice, mazeData, pathWidth)) {
		return false;
	}

	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) { return false; }

	hr = m_fontFactory->CreateFontWrapper(device, L"Impact", &m_fontWrapper);
	if (FAILED(hr)) {
		hr = m_fontFactory->CreateFontWrapper(device, L"Arial", &m_fontWrapper);
		if (FAILED(hr)) { return false; }
	}

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());
	m_orbIcon = std::make_unique<Sprite>();
	if (!m_orbIcon->Initialize(device, AssetPaths::TEX_MINIMAP_ORB)) {
		return false;
	}

	return true;
}

void UI::Shutdown() {
	if (m_minimap) { m_minimap->Shutdown(); }
	if (m_orbIcon) { m_orbIcon->Shutdown(); }

	if (m_fontWrapper) { m_fontWrapper->Release(); }
	if (m_fontFactory) { m_fontFactory->Release(); }
}

void UI::Update(float deltaTime, int remainingOrbs, int totalOrbs, float skillDuration, float skillCooldown, bool isSkillActive, bool showEnemiesOnMinimap) {
	m_remainingOrbs = remainingOrbs;
	m_totalOrbs = totalOrbs;
	m_skillDuration = skillDuration;
	m_skillCooldown = skillCooldown;
	m_isSkillActive = isSkillActive;
	m_showEnemiesOnMinimap = showEnemiesOnMinimap;
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs, float alpha) {
	if (alpha <= 0.0f) { return; }

	m_minimap->Render(camera, enemies, orbs, specialOrbs, m_showEnemiesOnMinimap, alpha);

	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());

	const DirectX::XMFLOAT2 iconPosition = { 280.0f, 40.0f };
	m_orbIcon->Render(m_spriteBatch.get(), iconPosition, 0.5f, 0.0f, { 1.0f, 1.0f, 1.0f, alpha });

	m_spriteBatch->End();

	if (m_fontWrapper) {
		UINT32 alphaInt = static_cast<UINT32>(alpha * 255.0f);

		// --- オーブ数テキスト ---
		std::wstring remainingText = std::to_wstring(m_remainingOrbs);
		float fontSize = 32.0f;
		float textPosX = iconPosition.x + 30.0f;
		float textPosY = iconPosition.y;
		UINT32 orbColor = (alphaInt << 24) | 0x00FFFFFF;

		m_fontWrapper->DrawString(
			m_graphicsDevice->GetDeviceContext(),
			remainingText.c_str(),
			fontSize,
			textPosX,
			textPosY,
			orbColor,
			FW1_VCENTER | FW1_RESTORESTATE
		);

		// --- スキル状態テキスト ---
		wchar_t skillBuffer[64];
		UINT32 skillColor;

		// 色とテキストの決定
		if (m_isSkillActive) {
			// 発動中（オレンジ）
			swprintf_s(skillBuffer, L"DASH TIME: %.1fs", m_skillDuration);
			skillColor = (alphaInt << 24) | 0x0000A5FF; // ABGR (Orange)
		}
		else if (m_skillCooldown > 0.0f) {
			// クールダウン中（グレー）
			swprintf_s(skillBuffer, L"COOLDOWN: %.1fs", m_skillCooldown);
			skillColor = (alphaInt << 24) | 0x00AAAAAA; // ABGR (Gray)
		}
		else {
			// 使用可能（緑/シアン）
			swprintf_s(skillBuffer, L"DASH READY");
			skillColor = (alphaInt << 24) | 0x00FFFF00; // ABGR (Cyan)
		}

		float skillTextX = iconPosition.x;
		float skillTextY = iconPosition.y + 40.0f; // アイコンの下に表示

		m_fontWrapper->DrawString(
			m_graphicsDevice->GetDeviceContext(),
			skillBuffer,
			24.0f,
			skillTextX,
			skillTextY,
			skillColor,
			FW1_LEFT | FW1_TOP | FW1_RESTORESTATE
		);
	}
}

Minimap* UI::GetMinimap() const {
	return m_minimap.get();
}

void UI::SetMinimapZoom(float zoomLevel) {
	if (m_minimap) {
		m_minimap->SetZoom(zoomLevel);
	}
}

void UI::RenderPauseMenu(int selectIndex, int screenWidth, int screenHeight) {
	// 1. 背景の暗転
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());
	RECT fullScreen = { 0, 0, screenWidth, screenHeight };
	m_orbIcon->RenderFill(m_spriteBatch.get(), fullScreen, { 0.0f, 0.0f, 0.0f, 0.85f }); //

	// 2. 境界線 (左30%の位置)
	float dividerX = screenWidth * 0.3f;
	RECT dividerLine = { static_cast<LONG>(dividerX), 0, static_cast<LONG>(dividerX + 2), screenHeight };
	m_orbIcon->RenderFill(m_spriteBatch.get(), dividerLine, { 1.0f, 1.0f, 1.0f, 0.3f });

	m_spriteBatch->End();

	if (m_fontWrapper) {
		// --- 左上: MENU タイトル ---
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"MENU", 72.0f, screenWidth * 0.05f, screenHeight * 0.08f, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		// --- 左中央: 選択項目 (0〜4) ---
		float menuTop = screenHeight * 0.28f;
		float menuLeft = screenWidth * 0.05f;
		const wchar_t* items[] = { L"SETTINGS", L"AUDIO", L"CONTROLS", L"HOW TO PLAY", L"SURVIVAL TIPS" };

		for (int i = 0; i < 5; ++i) {
			UINT32 color = (i == selectIndex) ? 0xFF00FFFF : 0xFFBBBBBB;
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), items[i], 32.0f, menuLeft, menuTop + (i * 60.0f), color, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
		}

		// --- 左下: RETURN (index 5) ---
		UINT32 retColor = (selectIndex == 5) ? 0xFF00FFFF : 0xFFFFFFFF;
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"RETURN TO GAME", 32.0f, menuLeft, screenHeight * 0.88f, retColor, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		// --- 右下: EXIT (index 6) ---
		UINT32 exitColor = (selectIndex == 6) ? 0xFF00FFFF : 0xFFFFFFFF;
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"BACK TO TITLE", 32.0f, screenWidth * 0.72f, screenHeight * 0.88f, exitColor, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		// --- 右側: コンテンツ表示 ---
		float dividerX = screenWidth * 0.3f;
		DrawMenuContent(selectIndex, dividerX + (screenWidth * 0.05f), screenHeight * 0.25f);
	}
}

void UI::DrawMenuContent(int selectIndex, float x, float y) {
	const wchar_t* title = L"";
	const wchar_t* body = L"";

	switch (selectIndex) {
	case 0: // SETTINGS
		title = L"VIDEO SETTINGS";
		body = L"• BRIGHTNESS: Adjust the game visibility.\n"
			L"• MOTION BLUR: Toggle blur effect during movement.\n"
			L"• FOV INTENSITY: Adjust the field of view effect.";
		break;
	case 1: // AUDIO
		title = L"AUDIO SETTINGS";
		body = L"• MASTER VOLUME: Adjust overall game sound.\n"
			L"• CURRENT: 80% (Configurable in Setting Scene)";
		break;
	case 2: // CONTROLS
		title = L"CONTROLS";
		body = L"• WASD: Move Player\n"
			L"• MOUSE: Look Around\n"
			L"• SHIFT: Dash (Uses Stamina)\n"
			L"• ESC: Open/Close Menu";
		break;
	case 3: // HOW TO PLAY
		title = L"HOW TO PLAY";
		body = L"1. Collect all Orbs scattered in the maze.\n"
			L"2. Find the glowing Gate to escape.\n"
			L"3. Avoid the 'Watchers' - if they see you, run!";
		break;
	case 4: // TIPS (新設)
		title = L"SURVIVAL TIPS";
		body = L"• Listen carefully to footstep sounds.\n"
			L"• Dashing is fast but very loud.\n"
			L"• Use the minimap to keep track of your path.";
		break;
	case 5: // RETURN (左下選択時)
		title = L"RESUME";
		body = L"Close this menu and continue your escape.";
		break;
	case 6: // EXIT (右下選択時)
		title = L"QUIT TO TITLE";
		body = L"Return to the main menu.\nYour current progress will be lost.";
		break;
	}

	// 描画処理
	m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), title, 48.0f, x, y, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
	m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), body, 24.0f, x, y + 80.0f, 0xFFAAAAAA, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
}