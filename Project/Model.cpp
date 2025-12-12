#include "Model.h"
#include "Texture.h"
#include <cmath>

Model::Model() {
	m_position = { 0.0f, 0.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.0f, 1.0f, 1.0f };
	m_emissiveColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_useTexture = true;
	m_useNormalMap = true;
	m_boundingSphereCenter = { 0.0f, 0.0f, 0.0f };
	m_boundingSphereRadius = 0.0f;
}

Model::~Model() {
	Shutdown();
}

bool Model::Initialize(ID3D11Device* device, const std::vector<SimpleVertex>& vertices, const std::vector<unsigned long>& indices) {
	if (vertices.empty() || indices.empty()) { return false; }

	CalculateBoundingSphere(vertices);

	Mesh newMesh = {};
	D3D11_BUFFER_DESC vertexBufferDesc = {}, indexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA vertexData = {}, indexData = {};
	HRESULT result;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(SimpleVertex) * vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexData.pSysMem = vertices.data();
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &newMesh.vertexBuffer);
	if (FAILED(result)) { return false; }

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned long) * indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexData.pSysMem = indices.data();
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &newMesh.indexBuffer);
	if (FAILED(result)) {
		newMesh.vertexBuffer->Release();
		return false;
	}

	newMesh.indexCount = static_cast<int>(indices.size());
	m_meshes.push_back(newMesh);

	return true;
}

void Model::CalculateBoundingSphere(const std::vector<SimpleVertex>& vertices) {
	if (vertices.empty()) { return; }

	DirectX::XMFLOAT3 minPos = vertices[0].pos;
	DirectX::XMFLOAT3 maxPos = vertices[0].pos;

	for (const auto& vertex : vertices) {
		minPos.x = (vertex.pos.x < minPos.x) ? vertex.pos.x : minPos.x;
		minPos.y = (vertex.pos.y < minPos.y) ? vertex.pos.y : minPos.y;
		minPos.z = (vertex.pos.z < minPos.z) ? vertex.pos.z : minPos.z;
		maxPos.x = (vertex.pos.x > maxPos.x) ? vertex.pos.x : maxPos.x;
		maxPos.y = (vertex.pos.y > maxPos.y) ? vertex.pos.y : maxPos.y;
		maxPos.z = (vertex.pos.z > maxPos.z) ? vertex.pos.z : maxPos.z;
	}

	m_boundingSphereCenter.x = (minPos.x + maxPos.x) / 2.0f;
	m_boundingSphereCenter.y = (minPos.y + maxPos.y) / 2.0f;
	m_boundingSphereCenter.z = (minPos.z + maxPos.z) / 2.0f;

	m_boundingSphereRadius = 0.0f;
	for (const auto& vertex : vertices) {
		float distance = sqrtf(
			pow(vertex.pos.x - m_boundingSphereCenter.x, 2) +
			pow(vertex.pos.y - m_boundingSphereCenter.y, 2) +
			pow(vertex.pos.z - m_boundingSphereCenter.z, 2)
		);
		if (distance > m_boundingSphereRadius) {
			m_boundingSphereRadius = distance;
		}
	}
}

DirectX::XMFLOAT3 Model::GetBoundingSphereCenter() const {
	DirectX::XMVECTOR center = DirectX::XMLoadFloat3(&m_boundingSphereCenter);
	DirectX::XMMATRIX world = GetWorldMatrix();
	center = DirectX::XMVector3TransformCoord(center, world);
	DirectX::XMFLOAT3 worldCenter;
	DirectX::XMStoreFloat3(&worldCenter, center);
	return worldCenter;
}

float Model::GetBoundingSphereRadius() const {
	float maxScale = m_scale.x;
	if (m_scale.y > maxScale) {
		maxScale = m_scale.y;
	}
	if (m_scale.z > maxScale) {
		maxScale = m_scale.z;
	}
	return m_boundingSphereRadius * maxScale;
}

void Model::SetTexture(std::shared_ptr<Texture> texture) {
	m_texture = texture;
}

void Model::SetNormalMap(std::shared_ptr<Texture> normalMap) {
	m_normalMap = normalMap;
}

void Model::Shutdown() {
	m_texture.reset();
	m_normalMap.reset();
	for (auto& mesh : m_meshes) {
		if (mesh.vertexBuffer) { mesh.vertexBuffer->Release(); }
		if (mesh.indexBuffer) { mesh.indexBuffer->Release(); }
	}
	m_meshes.clear();
}

void Model::Render(ID3D11DeviceContext* deviceContext) {
	if (m_texture) {
		ID3D11ShaderResourceView* textureView = m_texture->GetTexture();
		deviceContext->PSSetShaderResources(0, 1, &textureView);
	}
	if (m_normalMap) {
		ID3D11ShaderResourceView* normalMapView = m_normalMap->GetTexture();
		deviceContext->PSSetShaderResources(2, 1, &normalMapView);
	}
	for (auto& mesh : m_meshes) {
		RenderBuffers(deviceContext, mesh);
		deviceContext->DrawIndexed(mesh.indexCount, 0, 0);
	}
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext, const Mesh& mesh) {
	unsigned int stride = sizeof(SimpleVertex);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Model::SetPosition(float x, float y, float z) { m_position = { x, y, z }; }
void Model::SetRotation(float x, float y, float z) { m_rotation = { x, y, z }; }
void Model::SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

DirectX::XMMATRIX Model::GetWorldMatrix() const {
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	return scaleMatrix * rotationMatrix * translationMatrix;
}

void Model::SetEmissiveColor(const DirectX::XMFLOAT4& color) {
	m_emissiveColor = color;
}

void Model::SetUseTexture(bool useTexture) {
	m_useTexture = useTexture;
}

void Model::SetUseNormalMap(bool useNormalMap) {
	m_useNormalMap = useNormalMap;
}

DirectX::XMFLOAT4 Model::GetEmissiveColor() const {
	return m_emissiveColor;
}

bool Model::GetUseTexture() const {
	return m_useTexture;
}

bool Model::GetUseNormalMap() const {
	return m_useNormalMap;
}

bool Model::HasNormalMap() const {
	return m_normalMap != nullptr;
}