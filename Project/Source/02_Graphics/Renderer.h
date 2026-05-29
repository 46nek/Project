#pragma once
#include <vector>
#include <memory>
#include "Source/03_DirectX/GraphicsDevice.h"
#include "Source/02_Graphics/Model.h"
#include "Source/02_Graphics/LightManager.h"
#include "Source/04_GamePlay/Camera.h"
#include "Source/02_Graphics/Frustum.h"
#include "Source/04_GamePlay/Stage/MazeGenerator.h"

/**
 * @brief 3Dシーンの描画処理を統括するクラス
 */
class Renderer {
public:
	Renderer(GraphicsDevice* graphicsDevice);
	~Renderer();

	// --- 新しい描画フロー ---
	// 1. シーンをテクスチャにレンダリングする
	void RenderSceneToTexture(
		const std::vector<Model*>& stageModels,
		const std::vector<Model*>& dynamicModels,
		const Camera* camera,
		LightManager* lightManager,
		const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
		float pathWidth
	);
	// 2. レンダリングされたテクスチャにポストプロセスを適用し、画面に描画する
	void RenderFinalPass(const Camera* camera, float vignetteIntensity);
private:
	// シャドウマップ生成パス
	void RenderDepthPass(
		const std::vector<Model*>& stageModels,
		const std::vector<Model*>& dynamicModels,
		LightManager* lightManager
	);
	// 通常の描画パス
	void RenderMainPass(
		const std::vector<Model*>& stageModels,
		const std::vector<Model*>& dynamicModels,
		const Camera* camera,
		LightManager* lightManager,
		const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
		float pathWidth
	);

	GraphicsDevice* m_graphicsDevice;
	std::unique_ptr<Frustum> m_frustum;
};
