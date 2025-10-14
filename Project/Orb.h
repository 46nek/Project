#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"

/**
 * @brief 回収可能なオーブを管理するクラス
 */
class Orb
{
public:
    Orb();
    ~Orb();

    /**
     * @brief オーブを初期化します
     * @param device Direct3Dデバイス
     * @param position オーブの初期位置
     * @param lightIndex LightManagerで管理されるライトのインデックス
     * @return 初期化に成功した場合はtrue
     */
    bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex);

    /**
     * @brief リソースを解放します
     */
    void Shutdown();

    /**
     * @brief オーブの状態を更新します（浮遊アニメーション、プレイヤーとの当たり判定）
     * @param deltaTime 前フレームからの経過時間
     * @param player プレイヤーオブジェクト
     * @param lightManager ライト管理オブジェクト
     */
    void Update(float deltaTime, Player* player, LightManager* lightManager);

    /**
     * @brief 描画用のモデルを取得します（未回収の場合のみ）
     * @return モデルへのポインタ。回収済みの場合はnullptr
     */
    Model* GetModel();

    /**
     * @brief オーブが回収されたかどうかを取得します
     * @return 回収済みの場合はtrue
     */
    bool IsCollected() const;

private:
    std::unique_ptr<Model> m_model;
    DirectX::XMFLOAT3 m_position;
    bool m_isCollected;
    int m_lightIndex;

    // 浮遊アニメーション用のタイマー
    float m_animationTimer;
};