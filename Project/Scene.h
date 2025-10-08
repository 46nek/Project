// Project/Scene.h
#pragma once
#include "Direct3D.h"
#include "Input.h"

// �V�[���̎�ނ��`����񋓌^
enum class SceneState
{
	None,
	Title,
	Game,
};

// �V�[���̊��N���X�i�C���^�[�t�F�[�X�j
class Scene
{
public:
	Scene() : m_D3D(nullptr), m_Input(nullptr), m_nextScene(SceneState::None) {}
	virtual ~Scene() = default;

	// �e�V�[���Ŏ������鉼�z�֐�
	virtual bool Initialize(Direct3D* d3d, Input* input) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;

	// ���ɑJ�ڂ���V�[���̏�Ԃ��擾
	SceneState GetNextScene() const { return m_nextScene; }

protected:
	Direct3D* m_D3D;      // Direct3D�I�u�W�F�N�g�ւ̃|�C���^
	Input* m_Input;         // Input�I�u�W�F�N�g�ւ̃|�C���^
	SceneState m_nextScene = SceneState::None; // ���̃V�[���̏��
};