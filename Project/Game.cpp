// Game.cpp (���̓��e�Ŋ��S�ɒu�������Ă�������)

#include "Game.h"
#include <comdef.h> // CoInitializeEx�p

// Game�̃R���X�g���N�^�ŁA�Q�[���J�n���Ƀ|�[�Y��Ԃɂ���
Game::Game() : m_isPaused(true)
{
}

Game::~Game() {}

void Game::SetPaused(bool isPaused)
{
	m_isPaused = isPaused;
	if (m_isPaused)
	{
		// �J�[�\�����\�������܂� ShowCursor(true) ���Ăяo��������
		while (ShowCursor(true) < 0);
		// ������ �I�[�f�B�I���ꎞ��~ ������
		if (m_audioEngine)
		{
			m_audioEngine->Suspend();
		}
	}
	else
	{
		// �J�[�\������\���ɂȂ�܂� ShowCursor(false) ���Ăяo��������
		while (ShowCursor(false) >= 0);
		// ������ �I�[�f�B�I���ĊJ ������
		if (m_audioEngine)
		{
			m_audioEngine->Resume();
		}
	}
}

bool Game::IsPaused() const
{
	return m_isPaused;
}

bool Game::Initialize(HINSTANCE hInstance)
{
	// ������ COM���C�u������������ ������
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		// �G���[����
		return false;
	}

	m_input = std::make_unique<Input>();
	m_input->Initialize();

	m_window = std::make_unique<Window>(L"The Unseen", SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!m_window->Initialize(hInstance, m_input.get())) {
		return false;
	}

	m_graphicsDevice = std::make_unique<GraphicsDevice>();
	if (!m_graphicsDevice->Initialize(m_window->GetHwnd(), SCREEN_WIDTH, SCREEN_HEIGHT)) {
		MessageBox(m_window->GetHwnd(), L"Could not initialize Graphics Device.", L"Error", MB_OK);
		return false;
	}

	m_audioEngine = std::make_unique<DirectX::AudioEngine>(DirectX::AudioEngine_Default);

	m_sceneManager = std::make_unique<SceneManager>();
	if (!m_sceneManager->Initialize(m_graphicsDevice.get(), m_input.get(), m_audioEngine.get())) {
		return false;
	}

	m_timer = std::make_unique<Timer>();
	if (!m_timer->Initialize()) {
		return false;
	}

	// SetPaused���Ăяo���ď����J�[�\����ԂƃI�[�f�B�I��Ԃ��m�肳����
	SetPaused(m_isPaused);

	return true;
}

void Game::Shutdown()
{
	if (m_audioEngine)
	{
		m_audioEngine->Suspend();
	}
	m_audioEngine.reset(); // �X�}�[�g�|�C���^�����

	if (m_sceneManager) m_sceneManager->Shutdown();
	if (m_graphicsDevice) m_graphicsDevice->Shutdown();
	if (m_window) m_window->Shutdown();

	CoUninitialize();
}

void Game::Run()
{
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (!Update()) {
				break;
			}
			Render();
		}
	}
}

bool Game::Update()
{
	m_timer->Tick();

	if (m_audioEngine && !m_audioEngine->Update())
	{
		// �I�[�f�B�I�f�o�C�X������ꂽ�ꍇ�Ȃǂ̃G���[����
		if (m_audioEngine->IsCriticalError())
		{
			// �G���[���b�Z�[�W��\������Ȃ�
		}
	}

	if (m_input->IsKeyPressed(VK_ESCAPE)) {
		SetPaused(!m_isPaused);
	}

	if (!m_isPaused)
	{
		m_sceneManager->Update(m_timer->GetDeltaTime());

		HWND hwnd = m_window->GetHwnd();
		if (GetFocus() == hwnd)
		{
			POINT centerPoint = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
			ClientToScreen(hwnd, &centerPoint);
			SetCursorPos(centerPoint.x, centerPoint.y);
		}
	}

	m_input->EndFrame();
	return true;
}

void Game::Render()
{
	m_sceneManager->Render();
}