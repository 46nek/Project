#include "LightManager.h"
#include <vector>
#include <tuple>

LightManager::LightManager()
	: m_flashlightIndex(-1),
	m_flashlightBaseIntensity(0.0f), 
	m_flickerTimer(0.0f),
	m_nextFlickerTime(0.0f),
	m_isFlickering(false),
	m_rng(std::random_device{}()), 
	m_currentFlashlightPos(0.0f, 0.0f, 0.0f),
	m_currentFlashlightDir(0.0f, 0.0f, 1.0f) ,
	m_flashlightLagSpeed(5.0f), 
	m_isFlashlightInitialized(false)
{
}

LightManager::~LightManager()
{
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight)
{
	m_lights.clear();

	const int maze_width = static_cast<int>(mazeData[0].size());
	const int maze_height = static_cast<int>(mazeData.size());

	// --- 部屋のポイントライト ---
	const int roomSize = 3;
	const int cornerOffset = 1;
	using Rect = std::tuple<int, int, int, int>;
	const std::vector<Rect> rooms = {
		{cornerOffset, cornerOffset, roomSize, roomSize},
		{maze_width - cornerOffset - roomSize, cornerOffset, roomSize, roomSize},
		{cornerOffset, maze_height - cornerOffset - roomSize, roomSize, roomSize},
		{maze_width - cornerOffset - roomSize, maze_height - cornerOffset - roomSize, roomSize, roomSize},
		{(maze_width - roomSize) / 2, (maze_height - roomSize) / 2, roomSize, roomSize}
	};

	for (const auto& r : rooms) {
		int roomX = std::get<0>(r) + std::get<2>(r) / 2;
		int roomZ = std::get<1>(r) + std::get<3>(r) / 2;
		Light roomLight = {};
		roomLight.Enabled = true;
		roomLight.Type = PointLight;
		roomLight.Color = { 1.0f, 0.8f, 0.6f, 1.0f };
		roomLight.Intensity = 2.0f;
		roomLight.Range = 15.0f;
		roomLight.Attenuation = { 0.1f, 0.4f, 0.2f };
		roomLight.Position = { (roomX + 0.5f) * pathWidth, wallHeight - 1.0f, (roomZ + 0.5f) * pathWidth };
		m_lights.push_back(roomLight);
	}

	// --- 懐中電灯（スポットライト）を追加 ---
	Light flashlight = {};
	flashlight.Enabled = true;
	flashlight.Type = SpotLight;
	flashlight.Color = { 1.0f, 1.0f, 0.9f, 1.0f }; m_flashlightBaseIntensity = 1.2f; // 元(0.2f)より明るく
	flashlight.Intensity = m_flashlightBaseIntensity;
	flashlight.Range = 30.0f; 
	flashlight.SpotAngle = 0.75f; 
	flashlight.Attenuation = { 1.0f, 0.08f, 0.01f };
	m_lights.push_back(flashlight);
	m_flashlightIndex = static_cast<int>(m_lights.size()) - 1;

	// --- 最終処理 ---
	m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
	if (m_lightBuffer.NumLights > MAX_LIGHTS) {
		m_lightBuffer.NumLights = MAX_LIGHTS;
	}
	for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
		m_lightBuffer.Lights[i] = m_lights[i];
	}

	float mazeWorldWidth = maze_width * pathWidth;
	float mazeWorldHeight = maze_height * pathWidth;
	DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 50.0f, mazeWorldHeight / 2.0f, 1.0f);
	DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 0.0f, mazeWorldHeight / 2.0f, 1.0f);
	DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
	m_lightProjectionMatrix = DirectX::XMMatrixOrthographicLH(mazeWorldWidth, mazeWorldHeight, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& cameraRotation)
{
	m_lightBuffer.CameraPosition = cameraPosition;
	UpdateFlashlight(deltaTime, cameraPosition, cameraRotation);
	m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
	if (m_lightBuffer.NumLights > MAX_LIGHTS)
	{
		m_lightBuffer.NumLights = MAX_LIGHTS;
	}

	for (int i = 0; i < m_lightBuffer.NumLights; ++i)
	{
		m_lightBuffer.Lights[i] = m_lights[i];
	}
}

void LightManager::UpdateFlashlight(float deltaTime, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation)
{
	if (m_flashlightIndex == -1) return;

	ApplyFlicker(m_flashlightIndex, deltaTime);

	// --- ターゲットとなるカメラの位置と向きを計算 ---
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		rotation.x * (DirectX::XM_PI / 180.0f),
		rotation.y * (DirectX::XM_PI / 180.0f),
		rotation.z * (DirectX::XM_PI / 180.0f)
	);
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR targetDirectionVec = DirectX::XMVector3TransformNormal(forward, rotationMatrix);
	DirectX::XMVECTOR targetPositionVec = DirectX::XMLoadFloat3(&position);

	// --- 初回実行時のみ、現在位置をターゲット位置にスナップさせる ---
	if (!m_isFlashlightInitialized)
	{
		DirectX::XMStoreFloat3(&m_currentFlashlightPos, targetPositionVec);
		DirectX::XMStoreFloat3(&m_currentFlashlightDir, targetDirectionVec);
		m_isFlashlightInitialized = true;
	}

	// --- 現在の位置と向きをロード ---
	DirectX::XMVECTOR currentPositionVec = DirectX::XMLoadFloat3(&m_currentFlashlightPos);
	DirectX::XMVECTOR currentDirectionVec = DirectX::XMLoadFloat3(&m_currentFlashlightDir);

	// --- Lerp（線形補間）で滑らかに追従 ---
	// m_flashlightLagSpeed が小さいほど滑らか（追従が遅い）
	// std::min で補間係数が 1.0f を超えないようにクランプ
	float lerpFactor = std::min(deltaTime * m_flashlightLagSpeed, 1.0f);

	// 位置の補間
	currentPositionVec = DirectX::XMVectorLerp(currentPositionVec, targetPositionVec, lerpFactor);

	// 向きの補間 (Lerp後に正規化)
	currentDirectionVec = DirectX::XMVectorLerp(currentDirectionVec, targetDirectionVec, lerpFactor);
	currentDirectionVec = DirectX::XMVector3Normalize(currentDirectionVec); // 正規化が重要

	// --- 結果をメンバ変数に保存 ---
	DirectX::XMStoreFloat3(&m_currentFlashlightPos, currentPositionVec);
	DirectX::XMStoreFloat3(&m_currentFlashlightDir, currentDirectionVec);

	// --- ライトバッファ（m_lights配列）に反映 ---
	m_lights[m_flashlightIndex].Position = m_currentFlashlightPos;
	DirectX::XMStoreFloat3(&m_lights[m_flashlightIndex].Direction, currentDirectionVec);
}

void LightManager::ApplyFlicker(int lightIndex, float deltaTime)
{
	m_flickerTimer += deltaTime;

	if (m_isFlickering)
	{
		// ちらつき中
		if (m_flickerTimer > 0.1f) // 0.1秒間ちらつく
		{
			m_isFlickering = false;
			m_flickerTimer = 0.0f;
			m_lights[lightIndex].Intensity = m_flashlightBaseIntensity; // 通常の明るさに戻す
		}
		else
		{
			// ランダムに明るさを変える (基準より暗く)
			std::uniform_real_distribution<float> intensityDist(m_flashlightBaseIntensity * 0.1f, m_flashlightBaseIntensity * 0.5f);
			m_lights[lightIndex].Intensity = intensityDist(m_rng);
		}
	}
	else
	{
		// 次のちらつきまでの待機中
		if (m_flickerTimer > m_nextFlickerTime)
		{
			m_isFlickering = true;
			m_flickerTimer = 0.0f;
			// 次のちらつきまでの時間をランダムに設定 (2秒～10秒後)
			std::uniform_real_distribution<float> timeDist(2.0f, 10.0f);
			m_nextFlickerTime = timeDist(m_rng);
		}
	}
}

int LightManager::AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity)
{
	if (m_lights.size() >= MAX_LIGHTS)
	{
		return -1;
	}
	Light newLight = {};
	newLight.Enabled = true;
	newLight.Type = PointLight;
	newLight.Position = position;
	newLight.Color = color;
	newLight.Range = range;
	newLight.Intensity = intensity;
	newLight.Attenuation = { 0.2f, 0.4f, 0.1f };
	m_lights.push_back(newLight);
	return static_cast<int>(m_lights.size()) - 1;
}

void LightManager::SetLightEnabled(int index, bool enabled)
{
	if (index >= 0 && index < m_lights.size())
	{
		m_lights[index].Enabled = enabled;
	}
}

int LightManager::GetFlashlightIndex() const
{
	return m_flashlightIndex;
}