#include "Frustum.h"

Frustum::Frustum() {
}

Frustum::~Frustum() {
}

void Frustum::ConstructFrustum(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix) {
	DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);

	// ç∂ïΩñ 
	m_planes[0].normal.x = viewProjMatrix.r[0].m128_f32[3] + viewProjMatrix.r[0].m128_f32[0];
	m_planes[0].normal.y = viewProjMatrix.r[1].m128_f32[3] + viewProjMatrix.r[1].m128_f32[0];
	m_planes[0].normal.z = viewProjMatrix.r[2].m128_f32[3] + viewProjMatrix.r[2].m128_f32[0];
	m_planes[0].distance = viewProjMatrix.r[3].m128_f32[3] + viewProjMatrix.r[3].m128_f32[0];

	// âEïΩñ 
	m_planes[1].normal.x = viewProjMatrix.r[0].m128_f32[3] - viewProjMatrix.r[0].m128_f32[0];
	m_planes[1].normal.y = viewProjMatrix.r[1].m128_f32[3] - viewProjMatrix.r[1].m128_f32[0];
	m_planes[1].normal.z = viewProjMatrix.r[2].m128_f32[3] - viewProjMatrix.r[2].m128_f32[0];
	m_planes[1].distance = viewProjMatrix.r[3].m128_f32[3] - viewProjMatrix.r[3].m128_f32[0];

	// è„ïΩñ 
	m_planes[2].normal.x = viewProjMatrix.r[0].m128_f32[3] - viewProjMatrix.r[0].m128_f32[1];
	m_planes[2].normal.y = viewProjMatrix.r[1].m128_f32[3] - viewProjMatrix.r[1].m128_f32[1];
	m_planes[2].normal.z = viewProjMatrix.r[2].m128_f32[3] - viewProjMatrix.r[2].m128_f32[1];
	m_planes[2].distance = viewProjMatrix.r[3].m128_f32[3] - viewProjMatrix.r[3].m128_f32[1];

	// â∫ïΩñ 
	m_planes[3].normal.x = viewProjMatrix.r[0].m128_f32[3] + viewProjMatrix.r[0].m128_f32[1];
	m_planes[3].normal.y = viewProjMatrix.r[1].m128_f32[3] + viewProjMatrix.r[1].m128_f32[1];
	m_planes[3].normal.z = viewProjMatrix.r[2].m128_f32[3] + viewProjMatrix.r[2].m128_f32[1];
	m_planes[3].distance = viewProjMatrix.r[3].m128_f32[3] + viewProjMatrix.r[3].m128_f32[1];

	// ãﬂïΩñ 
	m_planes[4].normal.x = viewProjMatrix.r[0].m128_f32[2];
	m_planes[4].normal.y = viewProjMatrix.r[1].m128_f32[2];
	m_planes[4].normal.z = viewProjMatrix.r[2].m128_f32[2];
	m_planes[4].distance = viewProjMatrix.r[3].m128_f32[2];

	// âìïΩñ 
	m_planes[5].normal.x = viewProjMatrix.r[0].m128_f32[3] - viewProjMatrix.r[0].m128_f32[2];
	m_planes[5].normal.y = viewProjMatrix.r[1].m128_f32[3] - viewProjMatrix.r[1].m128_f32[2];
	m_planes[5].normal.z = viewProjMatrix.r[2].m128_f32[3] - viewProjMatrix.r[2].m128_f32[2];
	m_planes[5].distance = viewProjMatrix.r[3].m128_f32[3] - viewProjMatrix.r[3].m128_f32[2];

	// ïΩñ Çê≥ãKâª
	for (int i = 0; i < 6; i++) {
		float length = sqrtf((m_planes[i].normal.x * m_planes[i].normal.x) + (m_planes[i].normal.y * m_planes[i].normal.y) + (m_planes[i].normal.z * m_planes[i].normal.z));
		if (length == 0.0f) { continue; } // É[ÉçèúéZÇñhé~
		m_planes[i].normal.x /= length;
		m_planes[i].normal.y /= length;
		m_planes[i].normal.z /= length;
		m_planes[i].distance /= length;
	}
}

bool Frustum::CheckPoint(const DirectX::XMFLOAT3& point) const {
	for (int i = 0; i < 6; i++) {
		DirectX::XMFLOAT4 plane(m_planes[i].normal.x, m_planes[i].normal.y, m_planes[i].normal.z, m_planes[i].distance);
		if (DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(DirectX::XMLoadFloat4(&plane), DirectX::XMLoadFloat3(&point))) < 0.0f) {
			return false;
		}
	}
	return true;
}

bool Frustum::CheckSphere(const DirectX::XMFLOAT3& center, float radius) const {
	for (int i = 0; i < 6; i++) {
		DirectX::XMFLOAT4 plane(m_planes[i].normal.x, m_planes[i].normal.y, m_planes[i].normal.z, m_planes[i].distance);
		if (DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(DirectX::XMLoadFloat4(&plane), DirectX::XMLoadFloat3(&center))) < -radius) {
			return false;
		}
	}
	return true;
}