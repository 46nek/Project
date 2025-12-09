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
	m_isPlayHovered(false)
{
}

TitleScene::~TitleScene() {}

bool TitleScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	// ▼▼▼ タイトル画面設定：カーソルロック無効 ▼▼▼
	if (g_game) {
		g_game->m_cursorLockEnabled = false;
	}
	// 初期化時に念のためカーソルを表示させる
	while (ShowCursor(true) < 0);

	std::srand(static_cast<unsigned int>(time(nullptr)));

	ID3D11Device* device = m_graphicsDevice->GetDevice();

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	m_gameScene = std::make_unique<GameScene>();
	if (!m_gameScene->InitializePhase1(graphicsDevice, input, audioEngine)) return false;

	m_gameScene->SetCameraForTitle();

	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) return false;

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
	if (m_fonts.empty()) return false;

	m_titleText = L"THE UNSEEN";
	m_charStates.resize(m_titleText.length());
	for (auto& state : m_charStates) state.fontIndex = 0;

	m_playText = L"PLAY";
	m_playCharStates.resize(m_playText.length());
	for (auto& state : m_playCharStates) state.fontIndex = 0;

	return true;
}

void TitleScene::Shutdown()
{
	for (auto* wrapper : m_fonts) {
		if (wrapper) wrapper->Release();
	}
	m_fonts.clear();

	if (m_fontFactory) {
		m_fontFactory->Release();
		m_fontFactory = nullptr;
	}

	if (m_gameScene)
	{
		GameScene::s_transferInstance = std::move(m_gameScene);
	}
}

void TitleScene::Update(float deltaTime)
{
	// ▼▼▼ カーソル強制表示 ▼▼▼
	// 何らかの理由でカーソルが消えてしまうのを防ぐため、タイトル画面では常に表示状態にします
	while (ShowCursor(true) < 0);

	if (m_gameScene)
	{
		m_gameScene->UpdateTitleLoop(deltaTime);
	}

	int mx, my;
	m_input->GetMousePosition(mx, my);

	float playFontSize = 60.0f;
	float playY = 600.0f;

	FW1_RECTF rect = { 0, 0, 0, 0 };
	m_fonts[0]->MeasureString(m_playText.c_str(), nullptr, playFontSize, &rect, 0);
	float playWidth = rect.Right - rect.Left;
	float playX = (Game::SCREEN_WIDTH - playWidth) / 2.0f;
	float playHeight = playFontSize;

	// 当たり判定 (少し広めに取る)
	float padding = 40.0f;
	if (mx >= playX - padding && mx <= playX + playWidth + padding &&
		my >= playY - padding && my <= playY + playHeight + padding)
	{
		m_isPlayHovered = true;
	}
	else
	{
		m_isPlayHovered = false;
	}

	m_glitchTimer += deltaTime;
	if (m_glitchTimer > m_glitchUpdateInterval)
	{
		m_glitchTimer = 0.0f;

		for (auto& state : m_charStates)
		{
			if ((std::rand() % 100) < 10) state.fontIndex = std::rand() % m_fonts.size();
			else state.fontIndex = 0;
		}

		for (auto& state : m_playCharStates)
		{
			if (m_isPlayHovered && (std::rand() % 100) < 30)
			{
				state.fontIndex = std::rand() % m_fonts.size();
			}
			else
			{
				state.fontIndex = 0;
			}
		}
	}

	if (!m_isPlayHovered)
	{
		for (auto& state : m_playCharStates) state.fontIndex = 0;
	}

	// ▼▼▼ クリック判定の修正 ▼▼▼
	// Inputクラスの判定に加え、Windows標準のAPIでもチェックします（Inputクラスの仕様差異対策）
	bool isLeftClicked = m_input->IsKeyPressed(VK_LBUTTON) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	bool isEnterPressed = m_input->IsKeyPressed(VK_RETURN);

	// PLAYボタンクリック または Enterキー で開始
	if ((m_isPlayHovered && isLeftClicked) || isEnterPressed) {
		m_nextScene = SceneState::Loading;
	}
}

void TitleScene::Render()
{
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());

	if (m_gameScene)
	{
		m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
		m_gameScene->RenderStageOnly();
		m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());
	}

	if (!m_fonts.empty())
	{
		auto DrawGlitchText = [&](const std::wstring& text, std::vector<CharState>& states, float fontSize, float startY)
			{
				float totalWidth = 0.0f;
				std::vector<float> charWidths;

				for (size_t i = 0; i < text.length(); ++i)
				{
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
						if (w <= 0.1f) w = fontSize * 0.5f;
						else w += fontSize * 0.1f;
					}
					charWidths.push_back(w);
					totalWidth += w;
				}

				float currentX = (Game::SCREEN_WIDTH - totalWidth) / 2.0f;
				for (size_t i = 0; i < text.length(); ++i)
				{
					wchar_t str[2] = { text[i], L'\0' };
					int fontIdx = states[i].fontIndex;
					UINT32 color = 0xFFFFFFFF;

					if (text[i] != L' ')
					{
						m_fonts[fontIdx]->DrawString(
							m_graphicsDevice->GetDeviceContext(),
							str,
							fontSize,
							currentX,
							startY,
							color,
							FW1_RESTORESTATE
						);
					}
					currentX += charWidths[i];
				}
			};

		DrawGlitchText(m_titleText, m_charStates, 120.0f, 150.0f);
		DrawGlitchText(m_playText, m_playCharStates, 60.0f, 600.0f);
	}

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}