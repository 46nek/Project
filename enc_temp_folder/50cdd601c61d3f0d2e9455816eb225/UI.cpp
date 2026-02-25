#include "UI.h"
#include "Camera.h"
#include "Enemy.h"
#include "Orb.h"
#include "Game.h"
#include "AssetPaths.h"
#include <string>
#include <cstdio>
#include <algorithm>

extern Game* g_game;

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

void UI::UpdatePauseMenu(int& selectIndex, float mx, float my, bool isClicked, bool isDown, int screenWidth, int screenHeight) {
	float menuTop = screenHeight * 0.28f;
	float menuLeft = screenWidth * 0.05f;
	float itemWidth = screenWidth * 0.2f;
	float itemHeight = 40.0f;

	m_hoverIndex = -1;

	// 1. 左側メインメニューの判定
	for (int i = 0; i < 4; ++i) {
		float y = menuTop + (i * 60.0f);
		if (mx >= menuLeft && mx <= menuLeft + itemWidth && my >= y && my <= y + itemHeight) {
			m_hoverIndex = i;
			if (isClicked) selectIndex = i;
		}
	}

	// 2. 下部ボタンの判定
	if (mx >= menuLeft && mx <= menuLeft + 300.0f && my >= screenHeight * 0.88f && my <= screenHeight * 0.88f + 40.0f) {
		m_hoverIndex = 4;
		if (isClicked) selectIndex = 4;
	}
	if (mx >= screenWidth * 0.72f && mx <= screenWidth * 0.72f + 300.0f && my >= screenHeight * 0.88f && my <= screenHeight * 0.88f + 40.0f) {
		m_hoverIndex = 5;
		if (isClicked) selectIndex = 5;
	}

	// 3. SETTINGS内の操作
	if (selectIndex == 0) {
		GameSettings& s = g_game->GetSettings();
		float dividerX = screenWidth * 0.3f;
		float contentX = dividerX + (screenWidth * 0.05f);
		float contentY = screenHeight * 0.25f;

		float sliderW = 200.0f;
		float volY = contentY + 100.0f;
		float brightY = volY + 80.0f;
		float sensY = brightY + 80.0f;

		auto HandleSlider = [&](float x, float y, float& val, float minV, float maxV, bool& dragging) {
			// 当たり判定の範囲を広めに設定 (+-15, +-35)
			if (isClicked && mx >= x && mx <= x + sliderW && my >= y - 15 && my <= y + 35) dragging = true;
			if (!isDown) dragging = false;
			if (dragging) {
				float norm = std::clamp((mx - x) / sliderW, 0.0f, 1.0f);
				int displayVal = static_cast<int>(std::round(norm * 100.0f)); // 1刻みの整数
				val = minV + (displayVal / 100.0f) * (maxV - minV); // 内部数値に変換
			}
			};

		HandleSlider(contentX, volY, s.volume, 0.0f, 1.0f, m_isDraggingVolume);
		if (m_isDraggingVolume && g_game->GetAudioEngine()) g_game->GetAudioEngine()->SetMasterVolume(s.volume);

		HandleSlider(contentX, brightY, s.brightness, 0.5f, 1.5f, m_isDraggingBright);
		HandleSlider(contentX, sensY, s.mouseSensitivity, 0.5f, 2.0f, m_isDraggingSens);

		if (isClicked) {
			float arrowY = contentY + 340.0f;
			float leftArrowX = contentX + 220.0f;
			float rightArrowX = contentX + 380.0f;

			if (my >= arrowY && my <= arrowY + 40) {
				if (mx >= leftArrowX - 10 && mx <= rightArrowX + 40) s.motionBlur = !s.motionBlur;
			}
			if (my >= arrowY + 80.0f && my <= arrowY + 120.0f) {
				if (mx >= leftArrowX - 10 && mx <= leftArrowX + 40) s.fovIntensity = (s.fovIntensity + 2) % 3;
				if (mx >= rightArrowX - 10 && mx <= rightArrowX + 40) s.fovIntensity = (s.fovIntensity + 1) % 3;
			}
		}
	}
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
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());
	RECT fullScreen = { 0, 0, screenWidth, screenHeight };
	m_orbIcon->RenderFill(m_spriteBatch.get(), fullScreen, { 0.0f, 0.0f, 0.0f, 0.85f });

	float dividerX = screenWidth * 0.3f;
	RECT dividerLine = { static_cast<LONG>(dividerX), 0, static_cast<LONG>(dividerX + 2), screenHeight };
	m_orbIcon->RenderFill(m_spriteBatch.get(), dividerLine, { 1.0f, 1.0f, 1.0f, 0.3f });
	m_spriteBatch->End();

	if (m_fontWrapper) {
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"MENU", 72.0f, screenWidth * 0.05f, screenHeight * 0.08f, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		float menuTop = screenHeight * 0.28f;
		float menuLeft = screenWidth * 0.05f;
		const wchar_t* items[] = { L"SETTINGS", L"CONTROLS", L"HOW TO PLAY", L"SURVIVAL TIPS" };

		for (int i = 0; i < 4; ++i) {
			UINT32 color = (i == m_hoverIndex || i == selectIndex) ? 0xFF00FFFF : 0xFFBBBBBB;
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), items[i], 32.0f, menuLeft, menuTop + (i * 60.0f), color, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
		}

		// インデックスを 4 と 5 に修正
		UINT32 retColor = (m_hoverIndex == 4 || selectIndex == 4) ? 0xFF00FFFF : 0xFFFFFFFF;
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"RETURN TO GAME", 32.0f, menuLeft, screenHeight * 0.88f, retColor, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		UINT32 exitColor = (m_hoverIndex == 5 || selectIndex == 5) ? 0xFF00FFFF : 0xFFFFFFFF;
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"BACK TO TITLE", 32.0f, screenWidth * 0.72f, screenHeight * 0.88f, exitColor, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		DrawMenuContent(selectIndex, dividerX + (screenWidth * 0.05f), screenHeight * 0.25f, screenWidth, screenHeight);
	}
}

void UI::DrawMenuContent(int selectIndex, float x, float y, int screenWidth, int screenHeight) {
	if (selectIndex == 0) {
		GameSettings& s = g_game->GetSettings();
		m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"VIDEO & AUDIO", 48.0f, x, y, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

		float sliderW = 200.0f;
		float startY = y + 100.0f;

		auto DrawSlider = [&](const std::wstring& label, float sy, float value, float minV, float maxV) {
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 24.0f, x, sy - 35.0f, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"----------------------", 32.0f, x, sy, 0xFF444444, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);

			float norm = (value - minV) / (maxV - minV);
			int displayVal = static_cast<int>(std::round(norm * 100.0f));

			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"I", 32.0f, x + (sliderW * norm), sy, 0xFF00FFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			// 数値を表示
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), std::to_wstring(displayVal).c_str(), 24.0f, x + sliderW + 20.0f, sy, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			};

		DrawSlider(L"VOLUME", startY, s.volume, 0.0f, 1.0f);
		DrawSlider(L"BRIGHTNESS", startY + 80.0f, s.brightness, 0.5f, 1.5f);
		DrawSlider(L"MOUSE SENSITIVITY", startY + 160.0f, s.mouseSensitivity, 0.5f, 2.0f);

		float arrowY = startY + 240.0f; // y + 340.0f
		auto DrawArrowItem = [&](const std::wstring& label, const std::wstring& val, float ay) {
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 32.0f, x, ay, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"◀", 32.0f, x + 220.0f, ay, 0xFF00FFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), val.c_str(), 32.0f, x + 275.0f, ay, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), L"▶", 32.0f, x + 380.0f, ay, 0xFF00FFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
			};

		DrawArrowItem(L"MOTION BLUR", s.motionBlur ? L"ON" : L"OFF", arrowY);
		DrawArrowItem(L"FOV", (s.fovIntensity == 0 ? L"NONE" : s.fovIntensity == 1 ? L"WEAK" : L"NORMAL"), arrowY + 80.0f);
		return;
	}

	const wchar_t* title = L"";
	const wchar_t* body = L"";

	switch (selectIndex) {
	case 1:
		title = L"CONTROLS";
		body = L"• WASD: Move Player\n• MOUSE: Look Around\n• SHIFT: Dash\n• ESC: Open/Close Menu";
		break;
	case 2:
		title = L"HOW TO PLAY";
		body = L"1. Collect all Orbs scattered in the maze.\n2. Find the glowing Gate to escape.\n3. Avoid the 'Watchers'!";
		break;
	case 3:
		title = L"TIPS";
		body = L"• Listen carefully to footstep sounds.\n• Dashing is fast but very loud.\n• Use the minimap to keep track of your path.";
		break;
	case 4:
		title = L"RESUME";
		body = L"Close this menu and continue your escape.";
		break;
	case 5:
		title = L"QUIT TO TITLE";
		body = L"Return to the main menu.\nYour current progress will be lost.";
		break;
	}

	m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), title, 48.0f, x, y, 0xFFFFFFFF, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
	m_fontWrapper->DrawString(m_graphicsDevice->GetDeviceContext(), body, 24.0f, x, y + 80.0f, 0xFFAAAAAA, FW1_LEFT | FW1_TOP | FW1_RESTORESTATE);
}
