#include "Stage.h"
#include "AssetLoader.h"
#include "AssetPaths.h"
#include <windows.h>

namespace {
	// === 繧ｲ繝ｼ繝隱ｿ謨ｴ逕ｨ繝代Λ繝｡繝ｼ繧ｿ ===
	constexpr float GATE_MODEL_SCALE_BASE = 6.0f; // FBX繝ｭ繝ｼ繝画凾縺ｮ蝓ｺ貅悶せ繧ｱ繝ｼ繝ｫ
	constexpr float GATE_POS_Z_OFFSET = 0.1f;     // 驟咲ｽｮ譎ゅ・蠕ｮ隱ｿ謨ｴ
	constexpr float GATE_SCALE_MARGIN = 1.1f;     // 騾夊ｷｯ繧医ｊ縺ｩ繧後￥繧峨＞蟆上＆縺上☆繧九°
}

Stage::Stage() : m_exitX(0), m_exitY(0) {} // 繝｡繝ｳ繝仙・譛溷喧蟄舌Μ繧ｹ繝医ｒ菴ｿ逕ｨ
Stage::~Stage() {}

bool Stage::Initialize(GraphicsDevice* graphicsDevice) {
	m_mazeGenerator = std::make_unique<MazeGenerator>();
	m_mazeGenerator->Generate(MAZE_WIDTH, MAZE_HEIGHT);

	m_exitX = (MAZE_WIDTH - 1) / 2;
	m_exitY = 0;

	m_mazeGenerator->SetCell(m_exitX, m_exitY, MazeGenerator::Path);

	// 螳壽焚蛹悶＠縺溘ヱ繧ｹ繧剃ｽｿ逕ｨ
	std::shared_ptr<Texture> wallTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), AssetPaths::TEX_WALL);
	if (!wallTexture) { MessageBox(nullptr, L"Failed to load wall texture", L"Error", MB_OK); return false; }

	std::shared_ptr<Texture> wallNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), AssetPaths::TEX_WALL_NORMAL);
	if (!wallNormalMap) { MessageBox(nullptr, L"Failed to load wall normal map", L"Error", MB_OK); return false; }

	std::shared_ptr<Texture> gateTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), AssetPaths::TEX_GATE);
	if (!gateTexture) { MessageBox(nullptr, L"Failed to load gate texture", L"Error", MB_OK); return false; }

	std::shared_ptr<Texture> gateNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), AssetPaths::TEX_GATE_NORMAL);
	if (!gateNormalMap) { MessageBox(nullptr, L"Failed to load gate normal map", L"Error", MB_OK); return false; }

	// 螢√Δ繝・Ν (1谿ｵ逶ｮ)
	auto wallModel1 = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
	if (!wallModel1) { return false; }
	wallModel1->SetTexture(wallTexture);
	wallModel1->SetNormalMap(wallNormalMap);
	m_models.push_back(std::move(wallModel1));

	// 螢√Δ繝・Ν (2谿ｵ逶ｮ)
	auto wallModel2 = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
	if (!wallModel2) { return false; }
	wallModel2->SetTexture(wallTexture);
	wallModel2->SetNormalMap(wallNormalMap);
	wallModel2->SetPosition(0.0f, WALL_HEIGHT / 2.0f, 0.0f);
	m_models.push_back(std::move(wallModel2));

	// 螟ｩ莠・
	auto ceilingModel = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Ceiling);
	if (!ceilingModel) { return false; }
	ceilingModel->SetTexture(wallTexture);
	ceilingModel->SetNormalMap(wallNormalMap);
	m_models.push_back(std::move(ceilingModel));

	// 蠎・
	auto floorModel = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Floor);
	if (!floorModel) { return false; }
	floorModel->SetTexture(wallTexture);
	floorModel->SetNormalMap(wallNormalMap);
	m_models.push_back(std::move(floorModel));

	// 繧ｲ繝ｼ繝医Δ繝・Ν
	m_gateModel = AssetLoader::LoadModelFromFile(graphicsDevice->GetDevice(), AssetPaths::MODEL_CUBE_OBJ, GATE_MODEL_SCALE_BASE);

	if (m_gateModel) {
		float gateX = (static_cast<float>(m_exitX) + 0.5f) * PATH_WIDTH;
		float gateZ = (static_cast<float>(m_exitY) + GATE_POS_Z_OFFSET) * PATH_WIDTH;

		m_gateModel->SetScale(PATH_WIDTH - GATE_SCALE_MARGIN, WALL_HEIGHT, PATH_WIDTH - GATE_SCALE_MARGIN);
		m_gateModel->SetPosition(gateX, WALL_HEIGHT / 2.0f, gateZ);
		m_gateModel->SetTexture(gateTexture);
		m_gateModel->SetNormalMap(gateNormalMap);
	}

	m_mazeGenerator->SetCell(m_exitX, m_exitY, MazeGenerator::Wall);

	return true;
}

void Stage::OpenExit() {
	// 霑ｷ霍ｯ繝・・繧ｿ荳翫〒蜃ｺ蜿｣繧偵碁％縲阪↓縺吶ｋ・医・繝ｬ繧､繝､繝ｼ縺碁壹ｌ繧九ｈ縺・↓縺ｪ繧具ｼ・
	m_mazeGenerator->SetCell(m_exitX, m_exitY, MazeGenerator::Path);

	// 繧ｲ繝ｼ繝医Δ繝・Ν繧呈ｶ医☆・・ullptr縺ｫ縺励※謠冗判繝ｻ譖ｴ譁ｰ縺励↑縺・ｈ縺・↓縺吶ｋ・・
	if (m_gateModel) {
		m_gateModel->Shutdown();
		m_gateModel.reset();
	}
}

Model* Stage::GetGateModel() const {
	return m_gateModel.get();
}

void Stage::Shutdown() {
	for (auto& model : m_models) {
		if (model) { model->Shutdown(); }
	}
	m_models.clear();
	if (m_gateModel) { m_gateModel->Shutdown(); }
}

const std::vector<std::unique_ptr<Model>>& Stage::GetModels() const { return m_models; }
const std::vector<MazeGenerator::Room>& Stage::GetRooms() const { return m_mazeGenerator->GetRooms(); }
const std::vector<std::vector<MazeGenerator::CellType>>& Stage::GetMazeData() const { return m_mazeGenerator->GetMazeData(); }
std::pair<int, int> Stage::GetStartPosition() const { return m_mazeGenerator->GetStartPosition(); }
float Stage::GetPathWidth() const { return PATH_WIDTH; }
