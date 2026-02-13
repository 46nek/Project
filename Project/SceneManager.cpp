#include "SceneManager.h"
#include "TitleScene.h"
#include "SettingScene.h"
#include "GameScene.h"
#include "LoadingScene.h"
#include "ResultScene.h"

SceneManager::SceneManager()
	: m_currentScene(nullptr),
	m_graphicsDevice(nullptr),
	m_input(nullptr),
	m_audioEngine(nullptr) {
}

SceneManager::~SceneManager() {
}

bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	return ChangeScene(SceneState::Title);
}

void SceneManager::Shutdown() {
	if (m_currentScene) {
		m_currentScene->Shutdown();
		m_currentScene.reset();
	}
}

void SceneManager::Update(float deltaTime) {
	if (m_currentScene) {
		m_currentScene->Update(deltaTime);
		SceneState nextScene = m_currentScene->GetNextScene();
		if (nextScene != SceneState::None) {
			ChangeScene(nextScene);
		}
	}
}

void SceneManager::Render() {
	if (m_currentScene) {
		m_currentScene->Render();
	}
}

bool SceneManager::ChangeScene(SceneState nextState) {
	if (nextState == SceneState::None) {
		return true;
	}

	if (nextState == SceneState::Game) {
		LoadingScene* loadingScene = dynamic_cast<LoadingScene*>(m_currentScene.get());
		if (loadingScene) {
			// 繝ｭ繝ｼ繝・ぅ繝ｳ繧ｰ貂医∩縺ｮGameScene縺ｮ謇譛画ｨｩ繧貞叙蠕・
			std::unique_ptr<Scene> nextScene = loadingScene->GetGameScene();

			// 迴ｾ蝨ｨ縺ｮ繧ｷ繝ｼ繝ｳ繧堤ｴ譽・
			m_currentScene->Shutdown();
			m_currentScene = nullptr;

			// GameScene繧偵そ繝・ヨ
			m_currentScene = std::move(nextScene);
			return true;
		}
	}

	// 騾壼ｸｸ縺ｮ繧ｷ繝ｼ繝ｳ驕ｷ遘ｻ
	if (m_currentScene) {
		m_currentScene->Shutdown();
		m_currentScene = nullptr;
	}

	switch (nextState) {
	case SceneState::Title:
		m_currentScene = std::make_unique<TitleScene>();
		break;
	case SceneState::Setting:
		m_currentScene = std::make_unique<SettingScene>();
		break;
	case SceneState::Loading:
		m_currentScene = std::make_unique<LoadingScene>();
		break;
	case SceneState::Game:
		m_currentScene = std::make_unique<GameScene>();
		break;
	case SceneState::Result:
		m_currentScene = std::make_unique<ResultScene>();
		break;
	default:
		return false;
	}

	if (m_currentScene) {
		if (!m_currentScene->Initialize(m_graphicsDevice, m_input, m_audioEngine)) {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}
