#include "Minimap.h"
#include "Game.h" 

Minimap::Minimap()
    : m_graphicsDevice(nullptr), m_mazeData(nullptr), m_position({ 20.0f, 20.0f }), m_cellSize(8.0f), m_spriteScale(1.0f), m_mapWidth(0.0f), m_mapHeight(0.0f)
{
}

Minimap::~Minimap()
{
    Shutdown();
}

bool Minimap::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
{
    m_graphicsDevice = graphicsDevice;
    m_mazeData = &mazeData;

    ID3D11Device* device = m_graphicsDevice->GetDevice();
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

    // �A�Z�b�g�̓ǂݍ���
    m_wallSprite = std::make_unique<Sprite>();
    if (!m_wallSprite->Initialize(device, L"Assets/minimap_wall.png")) return false;

    m_pathSprite = std::make_unique<Sprite>();
    if (!m_pathSprite->Initialize(device, L"Assets/minimap_path.png")) return false;

    m_playerSprite = std::make_unique<Sprite>();
    if (!m_playerSprite->Initialize(device, L"Assets/minimap_player.png")) return false;

    // �}�b�v�̃T�C�Y�ƃX�v���C�g�̃X�P�[�����v�Z
    if (!m_mazeData->empty())
    {
        m_mapWidth = (*m_mazeData)[0].size() * m_cellSize;
        m_mapHeight = m_mazeData->size() * m_cellSize;
    }
    // ���摜�̃T�C�Y�Ɋւ�炸�A1�Z�����̃T�C�Y�ɂȂ�悤�ɃX�P�[�����v�Z
    m_spriteScale = m_cellSize / m_wallSprite->GetWidth();

    return true;
}

void Minimap::Shutdown()
{
    if (m_wallSprite) m_wallSprite->Shutdown();
    if (m_pathSprite) m_pathSprite->Shutdown();
    if (m_playerSprite) m_playerSprite->Shutdown();
}

void Minimap::Render(const Camera* camera)
{
    if (!m_graphicsDevice || !m_mazeData || !camera) return;

    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();

    // 2D�`��̂��߂�Z�o�b�t�@���I�t�ɂ���
    m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

    m_spriteBatch->Begin();

    // 1. ���H�̕`��
    for (size_t y = 0; y < m_mazeData->size(); ++y)
    {
        for (size_t x = 0; x < (*m_mazeData)[y].size(); ++x)
        {
            // �X�v���C�g�̒��S���W���v�Z
            DirectX::XMFLOAT2 cellPos = {
                m_position.x + x * m_cellSize + m_cellSize * 0.5f,
                m_position.y + y * m_cellSize + m_cellSize * 0.5f
            };

            if ((*m_mazeData)[y][x] == MazeGenerator::CellType::Wall)
            {
                m_wallSprite->Render(m_spriteBatch.get(), cellPos, m_spriteScale);
            }
            else
            {
                m_pathSprite->Render(m_spriteBatch.get(), cellPos, m_spriteScale);
            }
        }
    }

    // 2. �v���C���[�̕`��
    DirectX::XMFLOAT3 playerWorldPos = camera->GetPosition();
    const float pathWidth = 2.0f; // GameScene�Őݒ肳��Ă���ʘH�̕�

    // �v���C���[�̃��[���h���W���~�j�}�b�v���W�ɕϊ�
    DirectX::XMFLOAT2 playerMinimapPos = {
        m_position.x + (playerWorldPos.x / pathWidth) * m_cellSize,
        m_position.y + (playerWorldPos.z / pathWidth) * m_cellSize // ���[���h��Z�����}�b�v��Y���ɑΉ�
    };

    // �v���C���[�̌����iY����]�j�����W�A���ɕϊ�
    float playerRotation = camera->GetRotation().y * (DirectX::XM_PI / 180.0f);

    m_playerSprite->Render(m_spriteBatch.get(), playerMinimapPos, m_spriteScale, playerRotation);

    m_spriteBatch->End();

    // Z�o�b�t�@���I���ɖ߂�
    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);
}