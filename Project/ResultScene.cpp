#include "ResultScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetPaths.h"
#include <ctime>
#include <algorithm> // for std::max

extern Game* g_game;

ResultScene::ResultScene()
	: m_fontFactory(nullptr),
	m_glitchTimer(0.0f),
	m_glitchUpdateInterval(0.1f),
	m_isTitleHovered(false),
	m_isExitHovered(false) {
}

ResultScene::~ResultScene() {
}

bool ResultScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	// 繧ｫ繝ｼ繧ｽ繝ｫ繧定｡ｨ遉ｺ
	m_input->SetCursorLock(false);
	m_input->SetCursorVisible(true);

	std::srand(static_cast<unsigned int>(time(nullptr)));

	ID3D11Device* device = m_graphicsDevice->GetDevice();
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	m_background = std::make_unique<Sprite>();
	if (!m_background->Initialize(device, AssetPaths::TEX_BACKGROUND)) { return false; }

	// 繝輔か繝ｳ繝亥・譛溷喧
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) return false;

	const wchar_t* fontNames[] = { L"Impact", L"Times New Roman", L"Courier New", L"Comic Sans MS", L"Arial Black" };
	for (const auto& name : fontNames) {
		IFW1FontWrapper* wrapper = nullptr;
		if (SUCCEEDED(m_fontFactory->CreateFontWrapper(device, name, &wrapper))) {
			m_fonts.push_back(wrapper);
		}
	}

	m_escapeText = L"ESCAPE!!";
	m_escapeCharStates.resize(m_escapeText.length(), { 0 });

	m_toTitleText = L"BACK TO TITLE";
	m_toTitleCharStates.resize(m_toTitleText.length(), { 0 });

	m_exitText = L"EXIT GAME";
	m_exitCharStates.resize(m_exitText.length(), { 0 });

	return true;
}

void ResultScene::Shutdown() {
	m_background->Shutdown();
	for (auto* f : m_fonts) { if (f) f->Release(); }
	m_fonts.clear();
	if (m_fontFactory) m_fontFactory->Release();
}

void ResultScene::Update(float deltaTime) {
	int rawMx, rawMy;
	m_input->GetMousePosition(rawMx, rawMy);

	HWND hwnd = g_game->GetWindow()->GetHwnd();
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	float actualWidth = (std::max)(1.0f, (float)(clientRect.right - clientRect.left));
	float actualHeight = (std::max)(1.0f, (float)(clientRect.bottom - clientRect.top));

	float mx = rawMx * (static_cast<float>(Game::SCREEN_WIDTH) / actualWidth);
	float my = rawMy * (static_cast<float>(Game::SCREEN_HEIGHT) / actualHeight);

	// --- 繝帙ヰ繝ｼ蛻､螳・---
	float fontSize = 60.0f;
	float padding = 10.0f;

	// --- 繝帙ヰ繝ｼ蛻､螳・---
	float titleY = 450.0f;
	float titleW = CalculateTextWidth(m_toTitleText, fontSize);
	float titleX = (Game::SCREEN_WIDTH - titleW) / 2.0f;

	m_isTitleHovered = (mx >= titleX - padding && mx <= titleX + titleW + padding &&
		my >= titleY - padding && my <= titleY + fontSize + padding);

	float exitY = 550.0f;
	float exitW = CalculateTextWidth(m_exitText, fontSize);
	float exitX = (Game::SCREEN_WIDTH - exitW) / 2.0f;
	m_isExitHovered = (mx >= exitX - padding && mx <= exitX + exitW + padding &&
		my >= exitY - padding && my <= exitY + fontSize + padding);

	// --- 繧ｰ繝ｪ繝・メ譖ｴ譁ｰ (繝帙ヰ繝ｼ繝輔Λ繧ｰ縺ｮ縺ｿ繧定ｦ九ｋ繧医≧縺ｫ謨ｴ逅・ ---
	m_glitchTimer += deltaTime;
	if (m_glitchTimer > m_glitchUpdateInterval) {
		m_glitchTimer = 0.0f;
		auto updateStates = [&](std::vector<CharState>& states, bool active) {
			for (auto& s : states) {
				s.fontIndex = (active && std::rand() % 100 < 25) ? (std::rand() % m_fonts.size()) : 0;
			}
			};
		updateStates(m_escapeCharStates, true);
		updateStates(m_toTitleCharStates, m_isTitleHovered); 
		updateStates(m_exitCharStates, m_isExitHovered);
	}

	// 豎ｺ螳壼・逅・
	if (m_input->IsKeyPressed(VK_LBUTTON)) {
		if (m_isTitleHovered) m_nextScene = SceneState::Title;
		else if (m_isExitHovered) PostQuitMessage(0);
	}
}

void ResultScene::Render() {
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());

	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());
	RECT screenRect = { 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT };
	m_background->RenderFill(m_spriteBatch.get(), screenRect);
	m_spriteBatch->End();

	if (!m_fonts.empty()) {
		DrawGlitchText(m_escapeText, m_escapeCharStates, 120.0f, 150.0f, true);
		DrawGlitchText(m_toTitleText, m_toTitleCharStates, 60.0f, 450.0f, true);
		DrawGlitchText(m_exitText, m_exitCharStates, 60.0f, 550.0f, true);
	}

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}

float ResultScene::CalculateTextWidth(const std::wstring& text, float fontSize) {
	float totalWidth = 0.0f;
	for (wchar_t c : text) {
		wchar_t str[2] = { c, L'\0' };
		FW1_RECTF r = { 0, 0, 0, 0 };
		m_fonts[0]->MeasureString(str, nullptr, fontSize, &r, 0);
		float w = r.Right - r.Left;
		totalWidth += (w <= 0.1f) ? fontSize * 0.5f : w + fontSize * 0.05f;
	}
	return totalWidth;
}

void ResultScene::DrawGlitchText(const std::wstring& text, std::vector<CharState>& states, float fontSize, float startY, bool isHovered) {
	float totalWidth = CalculateTextWidth(text, fontSize);
	float currentX = (Game::SCREEN_WIDTH - totalWidth) / 2.0f;
	UINT32 color = 0xFFFFFFFF;

	for (size_t i = 0; i < text.length(); ++i) {
		wchar_t str[2] = { text[i], L'\0' };
		m_fonts[states[i].fontIndex]->DrawString(
			m_graphicsDevice->GetDeviceContext(),
			str, fontSize, currentX, startY, color,
			FW1_LEFT | FW1_TOP | FW1_RESTORESTATE
		);
		// 譁・ｭ鈴俣髫斐・邁｡譏楢ｨ育ｮ・
		FW1_RECTF r = { 0, 0, 0, 0 };
		m_fonts[states[i].fontIndex]->MeasureString(str, nullptr, fontSize, &r, 0);
		float w = r.Right - r.Left;
		currentX += (w <= 0.1f) ? fontSize * 0.5f : w + fontSize * 0.05f;
	}
}

