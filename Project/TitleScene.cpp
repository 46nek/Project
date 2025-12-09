#include "TitleScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetLoader.h"
#include "AssetPaths.h"

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

	std::srand(static_cast<unsigned int>(time(nullptr)));

	ID3D11Device* device = m_graphicsDevice->GetDevice();

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	m_gameScene = std::make_unique<GameScene>();
	// Phase1（迷路とカメラの生成）のみ実行
	if (!m_gameScene->InitializePhase1(graphicsDevice, input, audioEngine)) return false;

	// カメラをタイトル画面用の位置（ゴール付近俯瞰）に設定
	m_gameScene->SetCameraForTitle();

	// --- フォントシステムの初期化 ---
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) return false;

	// 使用するフォント名のリスト (0番目がメイン、残りがグリッチ候補)
	const wchar_t* fontNames[] = {
		L"Impact",          // メイン
		L"Times New Roman", // グリッチ用
		L"Courier New",     // グリッチ用
		L"Comic Sans MS",   // グリッチ用
		L"Arial Black"      // グリッチ用
	};

	// 各フォントのラッパーを作成
	for (const auto& name : fontNames) {
		IFW1FontWrapper* wrapper = nullptr;
		hr = m_fontFactory->CreateFontWrapper(device, name, &wrapper);
		if (SUCCEEDED(hr)) {
			m_fonts.push_back(wrapper);
		}
	}
	if (m_fonts.empty()) return false;

	// タイトルテキストの設定
	m_titleText = L"THE UNSEEN";
	m_charStates.resize(m_titleText.length());
	for (auto& state : m_charStates) state.fontIndex = 0;

	// PLAYボタンテキストの設定
	m_playText = L"PLAY";
	m_playCharStates.resize(m_playText.length());
	for (auto& state : m_playCharStates) state.fontIndex = 0;

	return true;
}

void TitleScene::Shutdown()
{
	// フォントリソースの解放
	for (auto* wrapper : m_fonts) {
		if (wrapper) wrapper->Release();
	}
	m_fonts.clear();

	if (m_fontFactory) {
		m_fontFactory->Release();
		m_fontFactory = nullptr;
	}

	// シーン終了時に次へ渡す
	if (m_gameScene)
	{
		GameScene::s_transferInstance = std::move(m_gameScene);
	}
}

void TitleScene::Update(float deltaTime)
{
	if (m_gameScene)
	{
		m_gameScene->UpdateTitleLoop(deltaTime);
	}

	// --- マウス判定 ---
	int mx, my;
	m_input->GetMousePosition(mx, my);

	// PLAYボタンの大まかな矩形を定義 (レンダリング位置と合わせる)
	// フォントサイズ60、画面中央、Y座標600付近を想定
	float playFontSize = 60.0f;
	float playY = 600.0f;

	// 幅の計算 (簡易的にメインフォントで計測)
	FW1_RECTF rect = { 0, 0, 0, 0 };
	m_fonts[0]->MeasureString(m_playText.c_str(), nullptr, playFontSize, &rect, 0);
	float playWidth = rect.Right - rect.Left;
	float playX = (Game::SCREEN_WIDTH - playWidth) / 2.0f;
	float playHeight = playFontSize; // 近似値

	// 当たり判定 (少し余裕を持たせる)
	if (mx >= playX - 20 && mx <= playX + playWidth + 20 &&
		my >= playY && my <= playY + playHeight)
	{
		m_isPlayHovered = true;
	}
	else
	{
		m_isPlayHovered = false;
	}

	// --- グリッチ更新処理 ---
	m_glitchTimer += deltaTime;
	if (m_glitchTimer > m_glitchUpdateInterval)
	{
		m_glitchTimer = 0.0f;

		// タイトルのグリッチ
		for (auto& state : m_charStates)
		{
			if ((std::rand() % 100) < 10) state.fontIndex = std::rand() % m_fonts.size();
			else state.fontIndex = 0;
		}

		// PLAYボタンのグリッチ (ホバー中のみ)
		for (auto& state : m_playCharStates)
		{
			if (m_isPlayHovered && (std::rand() % 100) < 30) // タイトルより少し激しく
			{
				state.fontIndex = std::rand() % m_fonts.size();
			}
			else
			{
				state.fontIndex = 0;
			}
		}
	}

	// ホバーしていないときは即座にフォントを戻す
	if (!m_isPlayHovered)
	{
		for (auto& state : m_playCharStates) state.fontIndex = 0;
	}

	// クリックでゲーム開始 (左クリック: VK_LBUTTON)
	if (m_isPlayHovered && m_input->IsKeyPressed(VK_LBUTTON)) {
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
		// ヘルパーラムダ式: 文字列を描画する
		auto DrawGlitchText = [&](const std::wstring& text, std::vector<CharState>& states, float fontSize, float startY)
			{
				float totalWidth = 0.0f;
				std::vector<float> charWidths;

				// 幅計算
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

				// 描画
				float currentX = (Game::SCREEN_WIDTH - totalWidth) / 2.0f;
				for (size_t i = 0; i < text.length(); ++i)
				{
					wchar_t str[2] = { text[i], L'\0' };
					int fontIdx = states[i].fontIndex;

					// 文字色設定
					UINT32 color = 0xFFFFFFFF; // 白

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

		// タイトルの描画
		DrawGlitchText(m_titleText, m_charStates, 120.0f, 150.0f);

		// PLAYボタンの描画 (色はホバー時に少し変えるなどの演出も可能ですが、今回はグリッチのみ)
		// ホバー時は少しだけフォントサイズを変えるなどの演出を入れても面白いです
		DrawGlitchText(m_playText, m_playCharStates, 60.0f, 600.0f);
	}

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}