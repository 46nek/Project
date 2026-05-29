#pragma once
#include <DirectXMath.h>

struct Plane {
	DirectX::XMFLOAT3 normal;
	float distance;
};

class Frustum {
public:
	Frustum();
	~Frustum();

	void ConstructFrustum(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix);

	bool CheckPoint(const DirectX::XMFLOAT3& point) const;

	bool CheckSphere(const DirectX::XMFLOAT3& center, float radius) const;

private:
	Plane m_planes[6];
};
