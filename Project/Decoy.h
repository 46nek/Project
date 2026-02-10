// Decoy.h
#pragma once
#include <DirectXMath.h>
#include <memory>
#include "Model.h"
#include "AssetLoader.h"
#include "AssetPaths.h"

class Decoy {
public:
    Decoy(const DirectX::XMFLOAT3& pos, float duration = 7.0f)
        : m_position(pos), m_timer(duration), m_isActive(true) {
    }

    bool Initialize(ID3D11Device* device) {
        m_model = AssetLoader::LoadModelFromFile(device, AssetPaths::MODEL_CUBE_OBJ);
        if (!m_model) return false;

        m_model->SetScale(0.4f, 0.4f, 0.4f); // 少し大きめにして目立たせる
        m_model->SetPosition(m_position.x, m_position.y, m_position.z);
        m_model->SetEmissiveColor({ 1.0f, 0.5f, 0.0f, 1.0f }); // 警告色（オレンジ）
        m_model->SetUseTexture(false);
        return true;
    }

    void Update(float deltaTime) {
        if (!m_isActive) return;
        m_timer -= deltaTime;
        if (m_timer <= 0.0f) m_isActive = false;
    }

    bool IsActive() const { return m_isActive; }
    DirectX::XMFLOAT3 GetPosition() const { return m_position; }
    Model* GetModel() { return m_model.get(); }

private:
    DirectX::XMFLOAT3 m_position;
    float m_timer;
    bool m_isActive;
    std::unique_ptr<Model> m_model;
};