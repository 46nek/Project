#include "LightManager.h"
#include <vector>
#include <tuple>
#include <cmath>
#include <algorithm> // min/max用

LightManager::LightManager()
	: m_flashlightIndex(-1),
	m_playerLightIndex(-1), // 追加
	m_flashlightBaseIntensity(0.0f),
	m_flickerTimer(0.0f),
	m_nextFlickerTime(0.0f),
	m_isFlickering(false),
	m_rng(std::random_device{}()),
	m_currentFlashlightPos(0.0f, 0.0f, 0.0f),
	m_currentFlashlightDir(0.0f, 0.0f, 1.0f),
	m_flashlightLagSpeed(5.0f),
	m_isFlashlightInitialized(false)
{
}

LightManager::~LightManager()
{
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight)
{
	m_mazeData = mazeData;
	m_pathWidth = pathWidth;

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

	// --- 懐中電灯（スポットライト） ---
	Light flashlight = {};
	flashlight.Enabled = true;
	flashlight.Type = SpotLight;
	flashlight.Color = { 1.0f, 1.0f, 0.9f, 1.0f }; 
	m_flashlightBaseIntensity = 1.2f;
	flashlight.Intensity = m_flashlightBaseIntensity;
	flashlight.Range = 30.0f;
	flashlight.SpotAngle = 0.9f;
	flashlight.Attenuation = { 1.0f, 0.08f, 0.01f };
	m_lights.push_back(flashlight);
	m_flashlightIndex = static_cast<int>(m_lights.size()) - 1;

	// 全方位を弱く照らすことで、真っ暗闇を回避しつつ、手元の壁の質感などを強調する
	Light playerLight = {};
	playerLight.Enabled = true;
	playerLight.Type = PointLight;
	playerLight.Color = { 0.8f, 0.8f, 1.0f, 1.0f }; // 少し青みがかった光
	playerLight.Intensity = 1.0f;
	playerLight.Range = 12.0f; // 懐中電灯より狭い範囲
	playerLight.Attenuation = { 0.4f, 0.4f, 0.2f }; // 減衰を強めにして、遠くには届かないようにする
	m_lights.push_back(playerLight);
	m_playerLightIndex = static_cast<int>(m_lights.size()) - 1;


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

void LightManager::Update(float deltaTime, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& cameraRotation)
{
	m_lightBuffer.CameraPosition = cameraPosition;

	UpdateFlashlight(deltaTime, cameraPosition, cameraRotation);
	UpdatePlayerLight(cameraPosition); // プレイヤーライトの更新

	Frustum frustum;
	frustum.ConstructFrustum(viewMatrix, projectionMatrix);

	m_lightBuffer.NumLights = 0;

	for (const auto& light : m_lights)
	{
		if (!light.Enabled) continue;

		// 1. フラスタム（画面内）チェック
		if (frustum.CheckSphere(light.Position, light.Range))
		{
			// 2. オクルージョン（遮蔽）チェック
			if (CheckOcclusion(cameraPosition, light.Position, light.Range))
			{
				if (m_lightBuffer.NumLights < MAX_LIGHTS)
				{
					m_lightBuffer.Lights[m_lightBuffer.NumLights] = light;
					m_lightBuffer.NumLights++;
				}
			}
		}
	}
}

// プレイヤーライトの位置更新用メソッド
void LightManager::UpdatePlayerLight(const DirectX::XMFLOAT3& position)
{
	if (m_playerLightIndex != -1 && m_playerLightIndex < m_lights.size())
	{
		// プレイヤーの少し上に配置
		m_lights[m_playerLightIndex].Position = { position.x, position.y + 0.5f, position.z };
	}
}

bool LightManager::CheckOcclusion(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, float range)
{
	if (m_mazeData.empty() || m_pathWidth <= 0.0f) return true;
	int x0 = static_cast<int>(start.x / m_pathWidth);
	int y0 = static_cast<int>(start.z / m_pathWidth);
	int x1 = static_cast<int>(end.x / m_pathWidth);
	int y1 = static_cast<int>(end.z / m_pathWidth);
	auto isValid = [&](int x, int y) {
		return x >= 0 && x < m_mazeData[0].size() && y >= 0 && y < m_mazeData.size();
		};
	if (!isValid(x0, y0) || !isValid(x1, y1)) return true;
	int dx = std::abs(x1 - x0);
	int dy = std::abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	while (true)
	{
		if (m_mazeData[y0][x0] != MazeGenerator::Path)
		{
			float wallX = (static_cast<float>(x0) + 0.5f) * m_pathWidth;
			float wallZ = (static_cast<float>(y0) + 0.5f) * m_pathWidth;
			float distX = end.x - wallX;
			float distZ = end.z - wallZ;
			float distSq = distX * distX + distZ * distZ;
			float checkRange = range * 3.0f;
			if (distSq <= (checkRange * checkRange)) return true;
			else return false;
		}
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x0 += sx; }
		if (e2 < dx) { err += dx; y0 += sy; }
	}
	return true;
}

void LightManager::UpdateFlashlight(float deltaTime, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation)
{
	if (m_flashlightIndex == -1) return;
	ApplyFlicker(m_flashlightIndex, deltaTime);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		rotation.x * (DirectX::XM_PI / 180.0f),
		rotation.y * (DirectX::XM_PI / 180.0f),
		rotation.z * (DirectX::XM_PI / 180.0f)
	);
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR targetDirectionVec = DirectX::XMVector3TransformNormal(forward, rotationMatrix);
	DirectX::XMVECTOR targetPositionVec = DirectX::XMLoadFloat3(&position);
	if (!m_isFlashlightInitialized)
	{
		DirectX::XMStoreFloat3(&m_currentFlashlightPos, targetPositionVec);
		DirectX::XMStoreFloat3(&m_currentFlashlightDir, targetDirectionVec);
		m_isFlashlightInitialized = true;
	}
	DirectX::XMVECTOR currentPositionVec = DirectX::XMLoadFloat3(&m_currentFlashlightPos);
	DirectX::XMVECTOR currentDirectionVec = DirectX::XMLoadFloat3(&m_currentFlashlightDir);
	float lerpFactor = std::min(deltaTime * m_flashlightLagSpeed, 1.0f);
	currentPositionVec = DirectX::XMVectorLerp(currentPositionVec, targetPositionVec, lerpFactor);
	currentDirectionVec = DirectX::XMVectorLerp(currentDirectionVec, targetDirectionVec, lerpFactor);
	currentDirectionVec = DirectX::XMVector3Normalize(currentDirectionVec);
	DirectX::XMStoreFloat3(&m_currentFlashlightPos, currentPositionVec);
	DirectX::XMStoreFloat3(&m_currentFlashlightDir, currentDirectionVec);
	m_lights[m_flashlightIndex].Position = m_currentFlashlightPos;
	DirectX::XMStoreFloat3(&m_lights[m_flashlightIndex].Direction, currentDirectionVec);
}

void LightManager::ApplyFlicker(int lightIndex, float deltaTime)
{
	m_flickerTimer += deltaTime;
	if (m_isFlickering)
	{
		if (m_flickerTimer > 0.1f)
		{
			m_isFlickering = false;
			m_flickerTimer = 0.0f;
			m_lights[lightIndex].Intensity = m_flashlightBaseIntensity;
		}
		else
		{
			std::uniform_real_distribution<float> intensityDist(m_flashlightBaseIntensity * 0.1f, m_flashlightBaseIntensity * 0.5f);
			m_lights[lightIndex].Intensity = intensityDist(m_rng);
		}
	}
	else
	{
		if (m_flickerTimer > m_nextFlickerTime)
		{
			m_isFlickering = true;
			m_flickerTimer = 0.0f;
			std::uniform_real_distribution<float> timeDist(2.0f, 10.0f);
			m_nextFlickerTime = timeDist(m_rng);
		}
	}
}

int LightManager::AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity)
{
	if (m_lights.size() >= MAX_LIGHTS) return -1;
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