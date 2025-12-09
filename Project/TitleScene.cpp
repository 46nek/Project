#include "TitleScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetLoader.h"
#include "AssetPaths.h"

TitleScene::TitleScene()
	: m_pressEnterScale(1.0f),
	m_fontFactory(nullptr),
	m_glitchTimer(0.0f),
	m_glitchUpdateInterval(0.1f)
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
		L"Impact",          // メイン: インパクトのある太字
		L"Times New Roman", // グリッチ: セリフ体
		L"Courier New",     // グリッチ: 等幅
		L"Comic Sans MS",   // グリッチ: 手書き風
		L"Arial Black"      // グリッチ: 極太
	};

	// 各フォントのラッパーを作成
	for (const auto& name : fontNames) {
		IFW1FontWrapper* wrapper = nullptr;
		hr = m_fontFactory->CreateFontWrapper(device, name, &wrapper);
		if (SUCCEEDED(hr)) {
			m_fonts.push_back(wrapper);
		}
	}
	// 少なくとも1つ（メイン）がないと失敗
	if (m_fonts.empty()) return false;

	// タイトルテキストの設定
	m_titleText = L"THE UNSEEN";

	// 各文字の状態を初期化
	m_charStates.resize(m_titleText.length());
	for (auto& state : m_charStates) {
		state.fontIndex = 0;
	}

	m_pressEnter = std::make_unique<Sprite>();
	if (!m_pressEnter->Initialize(device, AssetPaths::TEX_BUTTON)) return false;

	const float desiredButtonWidth = 400.0f;
	if (m_pressEnter->GetWidth() > 0)
	{
		m_pressEnterScale = desiredButtonWidth / m_pressEnter->GetWidth();
	}

	return true;
}

void TitleScene::Shutdown()
{
	m_pressEnter->Shutdown();

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

	// --- グリッチ更新処理 ---
	m_glitchTimer += deltaTime;
	if (m_glitchTimer > m_glitchUpdateInterval)
	{
		m_glitchTimer = 0.0f;

		// 文字ごとに確率でフォントを変更
		for (auto& state : m_charStates)
		{
			// 90%の確率でメインフォント(0)に戻す、10%でランダムなフォントに変化
			if ((std::rand() % 100) < 10)
			{
				state.fontIndex = std::rand() % m_fonts.size();
			}
			else
			{
				state.fontIndex = 0;
			}
		}
	}

	if (m_input->IsKeyPressed(VK_RETURN)) {
		m_nextScene = SceneState::Loading;
	}
}

void TitleScene::Render()
{
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());

	if (m_gameScene)
	{
		// ZバッファをONにして3D描画
		m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
		m_gameScene->RenderStageOnly();
		// 再びZバッファをOFFにして2D描画
		m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());
	}

	// SpriteBatchのBeginにアルファブレンド用のステートを渡す
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());
	m_pressEnter->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 600.0f }, m_pressEnterScale);
	m_spriteBatch->End();

	if (!m_fonts.empty())
	{
		float fontSize = 120.0f; // タイトルのフォントサイズ
		float startY = 150.0f;   // Y座標

		// 全体の幅を計算してセンタリングする
		float totalWidth = 0.0f;
		std::vector<float> charWidths;

		for (size_t i = 0; i < m_titleText.length(); ++i)
		{
			wchar_t c = m_titleText[i];
			float w = 0.0f;

			// スペースの場合は手動で幅を設定 (フォントサイズの30%程度)
			if (c == L' ') {
				w = fontSize * 0.3f;
			}
			else {
				wchar_t str[2] = { c, L'\0' };
				FW1_RECTF rect = { 0, 0, 0, 0 };

				// MeasureStringには描画フラグ(RESTORESTATE)ではなく 0 を渡すのが無難
				m_fonts[m_charStates[i].fontIndex]->MeasureString(str, nullptr, fontSize, &rect, 0);

				w = rect.Right - rect.Left;

				// 幅が取得できなかった場合の安全策 (最低限の幅を確保)
				if (w <= 0.1f) {
					w = fontSize * 0.5f;
				}
				else {
					// インクの幅ぴったりだとくっつきすぎるため、少し余白(パディング)を足す
					w += fontSize * 0.1f;
				}
			}

			charWidths.push_back(w);
			totalWidth += w;
		}

		// 描画開始X位置 (画面中央 - 文字列全体の半分)
		float currentX = (Game::SCREEN_WIDTH - totalWidth) / 2.0f;

		// 1文字ずつ描画
		for (size_t i = 0; i < m_titleText.length(); ++i)
		{
			wchar_t str[2] = { m_titleText[i], L'\0' };
			int fontIdx = m_charStates[i].fontIndex;
			float charW = charWidths[i];

			// スペース以外を描画
			if (m_titleText[i] != L' ')
			{
				// 文字ごとの中心位置を調整して描画したほうがブレが少ないが、
				// グリッチ感を出すために左寄せ(currentX)で描画
				m_fonts[fontIdx]->DrawString(
					m_graphicsDevice->GetDeviceContext(),
					str,
					fontSize,
					currentX,
					startY,
					0xFFFFFFFF, // 白
					FW1_RESTORESTATE
				);
			}

			// 次の文字のためにX座標を進める
			currentX += charW;
		}
	}

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}