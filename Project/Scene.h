// Project/Scene.h
#pragma once
#include "Direct3D.h"
#include "Input.h"

// シーンの種類を定義する列挙型
enum class SceneState
{
	None,
	Title,
	Game,
};

// シーンの基底クラス（インターフェース）
class Scene
{
public:
	Scene() : m_D3D(nullptr), m_Input(nullptr), m_nextScene(SceneState::None) {}
	virtual ~Scene() = default;

	// 各シーンで実装する仮想関数
	virtual bool Initialize(Direct3D* d3d, Input* input) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;

	// 次に遷移するシーンの状態を取得
	SceneState GetNextScene() const { return m_nextScene; }

protected:
	Direct3D* m_D3D;      // Direct3Dオブジェクトへのポインタ
	Input* m_Input;         // Inputオブジェクトへのポインタ
	SceneState m_nextScene = SceneState::None; // 次のシーンの状態
};