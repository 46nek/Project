#include "TitleScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetLoader.h"
#include "AssetPaths.h"

// グローバル変数のGameインスタンスを参照
extern Game* g_game;

TitleScene::TitleScene()
	: m_fontFactory(nullptr),
	m_glitchTimer(0.0f),
	m_glitchUpdateInterval(0.1f),
	m_isPlayHovered(false) {
}

TitleScene::~TitleScene() {
}

bool TitleScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	// タイトル画面：カーソルを表示してロック解除
	input->SetCursorLock(false);

	std::srand(static_cast<unsigned int>(time(nullptr)));

	ID3D11Device* device = m_graphicsDevice->GetDevice();

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	m_gameScene = std::make_unique<GameScene>();

	if (!m_gameScene->InitializeEnvironment(graphicsDevice, input, audioEngine)) { return false; }

	m_gameScene->SetCameraForTitle();

	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) { return false; }

	const wchar_t* fontNames[] = {
		L"Impact",
		L"Times New Roman",
		L"Courier New",
		L"Comic Sans MS",
		L"Arial Black"
	};

	for (const auto& name : fontNames) {
		IFW1FontWrapper* wrapper = nullptr;
		hr = m_fontFactory->CreateFontWrapper(device, name, &wrapper);
		if (SUCCEEDED(hr)) {
			m_fonts.push_back(wrapper);
		}
	}
	if (m_fonts.empty()) { return false; }

	m_titleText = L"THE UNSEEN";
	m_charStates.resize(m_titleText.length());
	for (auto& state : m_charStates) { state.fontIndex = 0; }

	m_playText = L"PLAY";
	m_playCharStates.resize(m_playText.length());
	for (auto& state : m_playCharStates) { state.fontIndex = 0; }

	m_settingText = L"SETTINGS";
	m_settingCharStates.resize(m_settingText.length());
	for (auto& state : m_settingCharStates) { state.fontIndex = 0; }

	return true;
}

void TitleScene::Shutdown() {
	for (auto* wrapper : m_fonts) {
		if (wrapper) { wrapper->Release(); }
	}
	m_fonts.clear();

	if (m_fontFactory) {
		m_fontFactory->Release();
		m_fontFactory = nullptr;
	}

	if (m_gameScene) {
		GameScene::s_transferInstance = std::move(m_gameScene);
	}
}

void TitleScene::Update(float deltaTime) {
	if (m_gameScene) {
		m_gameScene->UpdateTitleLoop(deltaTime);
	}

	int rawMx, rawMy;
	m_input->GetMousePosition(rawMx, rawMy);

	// --- ウィンドウの現在のサイズを取得して座標を変換 ---
	HWND hwnd = g_game->GetWindow()->GetHwnd();
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	float actualWidth = static_cast<float>(clientRect.right - clientRect.left);
	float actualHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	// 0除算防止
	if (actualWidth <= 0) actualWidth = 1.0f;
	if (actualHeight <= 0) actualHeight = 1.0f;

	// 物理座標(ウィンドウサイズ)から論理座標(1280x720)へ変換
	float mx = rawMx * (static_cast<float>(Game::SCREEN_WIDTH) / actualWidth);
	float my = rawMy * (static_cast<float>(Game::SCREEN_HEIGHT) / actualHeight);

	float padding = 10.0f;

	// --- CalculateGlitchTextWidth は変更なし ---
	auto CalculateGlitchTextWidth = [&](const std::wstring& text, float fontSize) {
		float totalWidth = 0.0f;
		for (size_t i = 0; i < text.length(); ++i) {
			wchar_t c = text[i];
			if (c == L' ') {
				totalWidth += fontSize * 0.3f;
			}
			else {
				wchar_t str[2] = { c, L'\0' };
				FW1_RECTF r = { 0, 0, 0, 0 };
				m_fonts[0]->MeasureString(str, nullptr, fontSize, &r, 0);
				float w = r.Right - r.Left;
				if (w <= 0.1f) w = fontSize * 0.5f;
				else w += fontSize * 0.1f;
				totalWidth += w;
			}
		}
		return totalWidth;
		};

	// --- PLAYボタン (mx, my を変換後の float 版で使用) ---
	float playFontSize = 60.0f;
	float playY = 500.0f;
	float playWidth = CalculateGlitchTextWidth(m_playText, playFontSize);
	float playX = (Game::SCREEN_WIDTH - playWidth) / 2.0f;

	m_isPlayHovered = (mx >= playX - padding && mx <= playX + playWidth + padding &&
		my >= playY - padding && my <= playY + playFontSize + padding);

	// --- SETTINGSボタン (Renderと合わせ 60.0f を使用) ---
	float setFontSize = 60.0f;
	float setY = 600.0f;
	float setWidth = CalculateGlitchTextWidth(m_settingText, setFontSize);
	float setX = (Game::SCREEN_WIDTH - setWidth) / 2.0f;

	m_isSettingHovered = (mx >= setX - padding && mx <= setX + setWidth + padding &&
		my >= setY - padding && my <= setY + setFontSize + padding);

	// --- グリッチ更新処理と遷移判定 ---
	m_glitchTimer += deltaTime;
	if (m_glitchTimer > m_glitchUpdateInterval) {
		m_glitchTimer = 0.0f;
		for (auto& state : m_charStates) {
			state.fontIndex = (std::rand() % 100 < 10) ? (std::rand() % m_fonts.size()) : 0;
		}
		for (auto& state : m_playCharStates) {
			state.fontIndex = (m_isPlayHovered && std::rand() % 100 < 30) ? (std::rand() % m_fonts.size()) : 0;
		}
		for (auto& state : m_settingCharStates) {
			state.fontIndex = (m_isSettingHovered && std::rand() % 100 < 30) ? (std::rand() % m_fonts.size()) : 0;
		}
	}

	if (!m_isPlayHovered) { for (auto& state : m_playCharStates) state.fontIndex = 0; }
	if (!m_isSettingHovered) { for (auto& state : m_settingCharStates) state.fontIndex = 0; }

	// Inputクラスに統合（GetAsyncKeyStateは削除）
	if ((m_isPlayHovered && m_input->IsKeyPressed(VK_LBUTTON)) || m_input->IsKeyPressed(VK_RETURN)) {
		m_nextScene = SceneState::Loading;
		m_input->SetCursorVisible(false);
	}
	if (m_isSettingHovered && m_input->IsKeyPressed(VK_LBUTTON)) {
		m_nextScene = SceneState::Setting;
	}
}

void TitleScene::Render() {
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());

	if (m_gameScene) {
		m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
		m_gameScene->RenderStageOnly();
		m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());
	}

	if (!m_fonts.empty()) {
		auto DrawGlitchText = [&](const std::wstring& text, std::vector<CharState>& states, float fontSize, float startY) {
			float totalWidth = 0.0f;
			std::vector<float> charWidths;

			for (size_t i = 0; i < text.length(); ++i) {
				wchar_t c = text[i];
				float w = 0.0f;
				if (c == L' ') {
					w = fontSize * 0.3f;
				}
				else {
					wchar_t str[2] = { c, L'\0' };
					FW1_RECTF rect = { 0, 0, 0, 0 };
					m_fonts[states[i].fontIndex]->MeasureString(str, nullptr, fontSize, &rect, 0);
					w = rect.Right - rect.Left;
					if (w <= 0.1f) { w = fontSize * 0.5f; }
					else { w += fontSize * 0.1f; }
				}
				charWidths.push_back(w);
				totalWidth += w;
			}

			float currentX = (Game::SCREEN_WIDTH - totalWidth) / 2.0f;
			for (size_t i = 0; i < text.length(); ++i) {
				wchar_t str[2] = { text[i], L'\0' };
				int fontIdx = states[i].fontIndex;
				UINT32 color = 0xFFFFFFFF;

				if (text[i] != L' ') {
					m_fonts[fontIdx]->DrawString(
						m_graphicsDevice->GetDeviceContext(),
						str,
						fontSize,
						currentX,
						startY,
						color,
						FW1_LEFT | FW1_TOP | FW1_RESTORESTATE
					);
				}
				currentX += charWidths[i];
			}
		};

		DrawGlitchText(m_titleText, m_charStates, 120.0f, 150.0f);
		DrawGlitchText(m_playText, m_playCharStates, 60.0f, 500.0f);
		DrawGlitchText(m_settingText, m_settingCharStates, 60.0f, 600.0f);
	}

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}