#pragma once

/**
 * @brief アセットのファイルパスを管理する名前空間
 */
namespace AssetPaths {
	// === モデル (Assimp用: char*) ===
	// 敵、オーブなどで使用
	const char* const MODEL_CUBE_OBJ = "Assets/cube.obj";

	// === テクスチャ (WIC/DirectX用: wchar_t*) ===
	// UI関連
	const wchar_t* const TEX_BUTTON = L"Assets/button.png";
	const wchar_t* const TEX_BACKGROUND = L"Assets/background.png";

	// ステージ関連
	const wchar_t* const TEX_WALL = L"Assets/wall.png";
	const wchar_t* const TEX_WALL_NORMAL = L"Assets/wall_normal.png";
	const wchar_t* const TEX_GATE = L"Assets/gate.jpg";
	const wchar_t* const TEX_GATE_NORMAL = L"Assets/gate_normal.png";

	// ミニマップ・HUD関連
	const wchar_t* const TEX_MINIMAP_ORB = L"Assets/minimap_orb.png";
	const wchar_t* const TEX_MINIMAP_FRAME = L"Assets/minimap_frame.png";
	const wchar_t* const TEX_MINIMAP_PATH = L"Assets/minimap_path.png";

	// === サウンド ===
	const wchar_t* const SOUND_ORB_GET = L"Assets/orb_get.wav";
	const wchar_t* const SOUND_WALK = L"Assets/walk.wav";

	// === パーティクル ===
	const wchar_t* const ENEMY_PARTICLE = L"Assets/e_particle.png";
}