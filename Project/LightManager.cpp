#include "LightManager.h"
#include <vector>
#include <tuple>
#include <cmath>
#include <algorithm> 

LightManager::LightManager()
	: m_flashlightIndex(-1),
	m_playerLightIndex(-1), 
	m_flashlightBaseIntensity(0.0f),
	m_flickerTimer(0.0f),
	m_nextFlickerTime(0.0f),
	m_isFlickering(false),
	m_rng(std::random_device{}()),
	m_currentFlashlightPos(0.0f, 0.0f, 0.0f),
	m_currentFlashlightDir(0.0f, 0.0f, 1.0f),
	m_flashlightLagSpeed(5.0f),
	m_isFlashlightInitialized(false) {
}

LightManager::~LightManager() {
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight) {
	m_mazeData = mazeData;
	m_pathWidth = pathWidth;

	m_lights.clear();

	const int maze_width = static_cast<int>(mazeData[0].size());
	const int maze_height = static_cast<int>(mazeData.size());

	// --- 驛ｨ螻九・繝昴う繝ｳ繝医Λ繧､繝・---
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
		roomLight.enabled = true;
		roomLight.type = PointLight;
		roomLight.color = { 1.0f, 0.8f, 0.6f, 1.0f };
		roomLight.intensity = 2.0f;
		roomLight.range = 15.0f;
		roomLight.attenuation = { 0.1f, 0.4f, 0.2f };
		roomLight.position = { (roomX + 0.5f) * pathWidth, wallHeight - 1.0f, (roomZ + 0.5f) * pathWidth };
		m_lights.push_back(roomLight);
	}

	// --- 諛蝉ｸｭ髮ｻ轣ｯ・医せ繝昴ャ繝医Λ繧､繝茨ｼ・---
	Light flashlight = {};
	flashlight.enabled = true;
	flashlight.type = SpotLight;
	flashlight.color = { 1.0f, 1.0f, 0.9f, 1.0f };
	m_flashlightBaseIntensity = 1.2f;
	flashlight.intensity = m_flashlightBaseIntensity;
	flashlight.range = 30.0f;
	flashlight.spotAngle = 0.9f;
	flashlight.attenuation = { 1.0f, 0.08f, 0.01f };
	m_lights.push_back(flashlight);
	m_flashlightIndex = static_cast<int>(m_lights.size()) - 1;

	// 蜈ｨ譁ｹ菴阪ｒ蠑ｱ縺冗・繧峨☆縺薙→縺ｧ縲∫悄縺｣證鈴裸繧貞屓驕ｿ縺励▽縺､縲∵焔蜈・・螢√・雉ｪ諢溘↑縺ｩ繧貞ｼｷ隱ｿ縺吶ｋ
	Light playerLight = {};
	playerLight.enabled = true;
	playerLight.type = PointLight;
	playerLight.color = { 0.8f, 0.8f, 1.0f, 1.0f }; // 蟆代＠髱偵∩縺後°縺｣縺溷・
	playerLight.intensity = 1.0f;
	playerLight.range = 12.0f; // 諛蝉ｸｭ髮ｻ轣ｯ繧医ｊ迢ｭ縺・ｯ・峇
	playerLight.attenuation = { 0.4f, 0.4f, 0.2f }; // 貂幄｡ｰ繧貞ｼｷ繧√↓縺励※縲・□縺上↓縺ｯ螻翫°縺ｪ縺・ｈ縺・↓縺吶ｋ
	m_lights.push_back(playerLight);
	m_playerLightIndex = static_cast<int>(m_lights.size()) - 1;

	// --- 譛邨ょ・逅・---
	m_lightBuffer.numLights = static_cast<int>(m_lights.size());
	if (m_lightBuffer.numLights > MAX_LIGHTS) {
		m_lightBuffer.numLights = MAX_LIGHTS;
	}
	for (int i = 0; i < m_lightBuffer.numLights; ++i) {
		m_lightBuffer.lights[i] = m_lights[i];
	}

	float mazeWorldWidth = maze_width * pathWidth;
	float mazeWorldHeight = maze_height * pathWidth;
	DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 50.0f, mazeWorldHeight / 2.0f, 1.0f);
	DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 0.0f, mazeWorldHeight / 2.0f, 1.0f);
	DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
	m_lightProjectionMatrix = DirectX::XMMatrixOrthographicLH(mazeWorldWidth, mazeWorldHeight, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& cameraRotation) {
	m_lightBuffer.cameraPosition = cameraPosition;

	UpdateFlashlight(deltaTime, cameraPosition, cameraRotation);
	UpdatePlayerLight(cameraPosition); // 繝励Ξ繧､繝､繝ｼ繝ｩ繧､繝医・譖ｴ譁ｰ

	Frustum frustum;
	frustum.ConstructFrustum(viewMatrix, projectionMatrix);

	m_lightBuffer.numLights = 0;

	for (const auto& light : m_lights) {
		if (!light.enabled) { continue; }

		// 1. 繝輔Λ繧ｹ繧ｿ繝・育判髱｢蜀・ｼ峨メ繧ｧ繝・け
		if (frustum.CheckSphere(light.position, light.range)) {
			// 2. 繧ｪ繧ｯ繝ｫ繝ｼ繧ｸ繝ｧ繝ｳ・磯・阡ｽ・峨メ繧ｧ繝・け
			if (CheckOcclusion(cameraPosition, light.position, light.range)) {
				if (m_lightBuffer.numLights < MAX_LIGHTS) {
					m_lightBuffer.lights[m_lightBuffer.numLights] = light;
					m_lightBuffer.numLights++;
				}
			}
		}
	}
}

// 繝励Ξ繧､繝､繝ｼ繝ｩ繧､繝医・菴咲ｽｮ譖ｴ譁ｰ逕ｨ繝｡繧ｽ繝・ラ
void LightManager::UpdatePlayerLight(const DirectX::XMFLOAT3& position) {
	if (m_playerLightIndex != -1 && m_playerLightIndex < m_lights.size()) {
		// 繝励Ξ繧､繝､繝ｼ縺ｮ蟆代＠荳翫↓驟咲ｽｮ
		m_lights[m_playerLightIndex].position = { position.x, position.y + 0.5f, position.z };
	}
}

bool LightManager::CheckOcclusion(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, float range) {
	if (m_mazeData.empty() || m_pathWidth <= 0.0f) { return true; }
	int x0 = static_cast<int>(start.x / m_pathWidth);
	int y0 = static_cast<int>(start.z / m_pathWidth);
	int x1 = static_cast<int>(end.x / m_pathWidth);
	int y1 = static_cast<int>(end.z / m_pathWidth);
	auto isValid = [&](int x, int y) {
		return x >= 0 && x < m_mazeData[0].size() && y >= 0 && y < m_mazeData.size();
		};
	if (!isValid(x0, y0) || !isValid(x1, y1)) { return true; }
	int dx = std::abs(x1 - x0);
	int dy = std::abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	while (true) {
		if (m_mazeData[y0][x0] != MazeGenerator::Path) {
			float wallX = (static_cast<float>(x0) + 0.5f) * m_pathWidth;
			float wallZ = (static_cast<float>(y0) + 0.5f) * m_pathWidth;
			float distX = end.x - wallX;
			float distZ = end.z - wallZ;
			float distSq = distX * distX + distZ * distZ;
			float checkRange = range * 3.0f;
			if (distSq <= (checkRange * checkRange)) { return true; }
			else { return false; }
		}
		if (x0 == x1 && y0 == y1) { break; }
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x0 += sx; }
		if (e2 < dx) { err += dx; y0 += sy; }
	}
	return true;
}

void LightManager::UpdateFlashlight(float deltaTime, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation) {
	if (m_flashlightIndex == -1) { return; }
	ApplyFlicker(m_flashlightIndex, deltaTime);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		rotation.x * (DirectX::XM_PI / 180.0f),
		rotation.y * (DirectX::XM_PI / 180.0f),
		rotation.z * (DirectX::XM_PI / 180.0f)
	);
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR targetDirectionVec = DirectX::XMVector3TransformNormal(forward, rotationMatrix);
	DirectX::XMVECTOR targetPositionVec = DirectX::XMLoadFloat3(&position);
	if (!m_isFlashlightInitialized) {
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
	m_lights[m_flashlightIndex].position = m_currentFlashlightPos;
	DirectX::XMStoreFloat3(&m_lights[m_flashlightIndex].direction, currentDirectionVec);
}

void LightManager::ApplyFlicker(int lightIndex, float deltaTime) {
	m_flickerTimer += deltaTime;
	if (m_isFlickering) {
		if (m_flickerTimer > 0.1f) {
			m_isFlickering = false;
			m_flickerTimer = 0.0f;
			m_lights[lightIndex].intensity = m_flashlightBaseIntensity;
		}
		else {
			std::uniform_real_distribution<float> intensityDist(m_flashlightBaseIntensity * 0.1f, m_flashlightBaseIntensity * 0.5f);
			m_lights[lightIndex].intensity = intensityDist(m_rng);
		}
	}
	else {
		if (m_flickerTimer > m_nextFlickerTime) {
			m_isFlickering = true;
			m_flickerTimer = 0.0f;
			std::uniform_real_distribution<float> timeDist(2.0f, 10.0f);
			m_nextFlickerTime = timeDist(m_rng);
		}
	}
}

int LightManager::AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity) {
	if (m_lights.size() >= MAX_LIGHTS) { return -1; }
	Light newLight = {};
	newLight.enabled = true;
	newLight.type = PointLight;
	newLight.position = position;
	newLight.color = color;
	newLight.range = range;
	newLight.intensity = intensity;
	newLight.attenuation = { 0.2f, 0.4f, 0.1f };
	m_lights.push_back(newLight);
	return static_cast<int>(m_lights.size()) - 1;
}

void LightManager::SetLightEnabled(int index, bool enabled) {
	if (index >= 0 && index < m_lights.size()) {
		m_lights[index].enabled = enabled;
	}
}

int LightManager::GetFlashlightIndex() const {
	return m_flashlightIndex;
}
