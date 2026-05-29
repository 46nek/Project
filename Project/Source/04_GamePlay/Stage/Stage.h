#pragma once
#include "Source/03_DirectX/GraphicsDevice.h"
#include "Source/02_Graphics/Model.h"
#include "Source/04_GamePlay/Stage/MazeGenerator.h"
#include <vector>
#include <memory>

/**
 * @class Stage
 * @brief ゲームのステージの構築と管理を行うクラス
 */
class Stage {
public:
	Stage();
	~Stage();

	bool Initialize(GraphicsDevice* graphicsDevice);
	void Shutdown();

	const std::vector<std::unique_ptr<Model>>& GetModels() const;
	Model* GetGateModel() const;

	const std::vector<std::vector<MazeGenerator::CellType>>& GetMazeData() const;
	const std::vector<MazeGenerator::Room>& GetRooms() const;
	std::pair<int, int> GetStartPosition() const;
	float GetPathWidth() const;
	void OpenExit();

	static constexpr int MAZE_WIDTH = 31;
	static constexpr int MAZE_HEIGHT = 31;
	static constexpr float PATH_WIDTH = 4.0f;
	static constexpr float WALL_HEIGHT = 8.0f;

private:
	std::unique_ptr<MazeGenerator> m_mazeGenerator;
	std::vector<std::unique_ptr<Model>> m_models;

	std::unique_ptr<Model> m_gateModel;
	int m_exitX;
	int m_exitY;
};