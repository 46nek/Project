// Project/UI.cpp

#include "UI.h"
#include "Camera.h"
#include "Enemy.h"
#include "Orb.h"
#include "Game.h" // Game::SCREEN_WIDTH/HEIGHT �̂��߂ɒǉ�
#include <string>

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

	// �X�^�~�i�Q�[�W�p�̃X�v���C�g��������
	// Assets/minimap_frame.png �� Assets/minimap_path.png ���Q�[�W�̉摜�Ƃ��čė��p���܂��B
	m_staminaBarFrame = std::make_unique<Sprite>();
	if (!m_staminaBarFrame->Initialize(device, L"Assets/minimap_frame.png")) return false;

	m_staminaBarFill = std::make_unique<Sprite>();
	if (!m_staminaBarFill->Initialize(device, L"Assets/minimap_path.png")) return false;

	return true;
}

void UI::Shutdown()
{
	if (m_minimap) m_minimap->Shutdown();
	if (m_orbIcon) m_orbIcon->Shutdown();
	if (m_staminaBarFrame) m_staminaBarFrame->Shutdown();
	if (m_staminaBarFill) m_staminaBarFill->Shutdown();

	if (m_fontWrapper) m_fontWrapper->Release();
	if (m_fontFactory) m_fontFactory->Release();
}

void UI::Update(float deltaTime, int remainingOrbs, int totalOrbs, float staminaPercentage, bool showEnemiesOnMinimap)
{
	m_remainingOrbs = remainingOrbs;
	m_totalOrbs = totalOrbs;
	m_staminaPercentage = staminaPercentage;
	m_showEnemiesOnMinimap = showEnemiesOnMinimap;
}


void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs, const std::vector<std::unique_ptr<Orb>>& specialOrbs)
{
	m_minimap->Render(camera, enemies, orbs, specialOrbs, m_showEnemiesOnMinimap);

	// �I�[�u�J�E���^�[�ƃX�^�~�i�Q�[�W�̕`��J�n
	m_spriteBatch->Begin();

	// --- �I�[�u�J�E���^�[�̃A�C�R���`�� ---
	const DirectX::XMFLOAT2 iconPosition = { 280.0f, 40.0f };
	// ������ �G���[�ӏ����C�� ������
	m_orbIcon->Render(m_spriteBatch.get(), iconPosition, 0.5f);
	// ������ �C�������܂� ������

	// --- �X�^�~�i�Q�[�W�̕`�� ---
	const float barWidth = 200.0f;
	const float barHeight = 20.0f;
	const DirectX::XMFLOAT2 barPosition = {
		(Game::SCREEN_WIDTH - barWidth) / 2.0f,
		Game::SCREEN_HEIGHT - 50.0f
	};

	// �w�i�i�t���[���j
	RECT frameRect = {
		(LONG)barPosition.x,
		(LONG)barPosition.y,
		(LONG)(barPosition.x + barWidth),
		(LONG)(barPosition.y + barHeight)
	};
	// ������ �G���[�ӏ����C�� ������
	m_staminaBarFrame->RenderFill(m_spriteBatch.get(), frameRect);
	// ������ �C�������܂� ������

	// �h��Ԃ�����
	// �X�^�~�i�̊����ɉ����ĕ���ς���
	RECT fillRect = {
		(LONG)barPosition.x,
		(LONG)barPosition.y,
		(LONG)(barPosition.x + barWidth * m_staminaPercentage),
		(LONG)(barPosition.y + barHeight)
	};

	// �X�^�~�i�c�ʂɉ����ĐF��ς���
	DirectX::XMFLOAT4 fillColor = { 0.0f, 1.0f, 0.0f, 1.0f }; // ��
	if (m_staminaPercentage < 0.3f)
	{
		fillColor = { 0.8f, 0.2f, 0.2f, 1.0f }; // ��
	}

	// RenderFill �ɐF�w��̃I�[�o�[���[�h���Ȃ����߁A�F��t���ĕ`�悵�܂�
	m_staminaBarFill->RenderFill(m_spriteBatch.get(), fillRect, fillColor);


	// �`��I��
	m_spriteBatch->End();

	// �I�[�u�J�E���^�[�̃e�L�X�g�`��
	if (m_fontWrapper)
	{
		std::wstring remainingText = std::to_wstring(m_remainingOrbs);

		float fontSize = 32.0f;
		float textPosX = iconPosition.x + 30.0f;
		float textPosY = iconPosition.y;
		UINT32 textColor = 0xFFFFFFFF; // ��

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