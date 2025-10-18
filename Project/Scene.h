#pragma once
#include "GraphicsDevice.h"
#include "Input.h"
#include "Audio.h" 

// �V�[���̏��
enum class SceneState
{
	None,
	Title, 
	Loading,
	Game,
};

/**
 * @brief �e�V�[���̊��ƂȂ钊�ۃN���X
 */
class Scene
{
public:
	Scene() : m_graphicsDevice(nullptr), m_input(nullptr), m_audioEngine(nullptr), m_nextScene(SceneState::None) {}
	virtual ~Scene() = default;

	virtual bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;

	SceneState GetNextScene() const { return m_nextScene; }

protected:
	GraphicsDevice* m_graphicsDevice;
	Input* m_input;
	DirectX::AudioEngine* m_audioEngine; // <--- ���̍s��ǉ�
	SceneState m_nextScene;
};