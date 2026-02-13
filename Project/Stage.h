#pragma once
#include "GraphicsDevice.h"
#include "Model.h"
#include "MazeGenerator.h"
#include <vector>
#include <memory>

/**
 * @class Stage
 * @brief 繧ｲ繝ｼ繝縺ｮ繧ｹ繝・・繧ｸ・郁ｿｷ霍ｯ縲∝慍蠖｢繝｢繝・Ν縺ｪ縺ｩ・峨・讒狗ｯ峨→邂｡逅・ｒ陦後≧繧ｯ繝ｩ繧ｹ
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

	// 螳壽焚
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
