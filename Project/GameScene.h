#pragma once
#include <memory>
#include <vector> 
#include "Scene.h"
#include "Camera.h"
#include "Model.h"
#include "MazeGenerator.h" 

class GameScene : public Scene
{
public:
    GameScene();
    ~GameScene();

    bool Initialize(Direct3D* d3d, Input* input) override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    void Render() override;

private:
    void HandleInput(float deltaTime);
    void UpdateCamera(float deltaTime);

    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<MazeGenerator> m_mazeGenerator; // MazeGeneratorのインスタンス
    std::unique_ptr<Model> m_wallModel; // 壁モデルを1つだけ保持する
    std::vector<DirectX::XMFLOAT3> m_wallPositions; // 壁の位置情報を保持する
    std::unique_ptr<Model> m_floorModel; // 床モデル
};