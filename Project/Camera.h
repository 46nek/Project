#pragma once
#include <DirectXMath.h>

/**
 * @brief プレイヤーの視点を管理するカメラクラス
 */
class Camera
{
public:
    Camera();
    ~Camera();

    /**
     * @brief カメラの位置を設定します。
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetPosition(float x, float y, float z);

    /**
     * @brief カメラの回転角度を設定します。
     * @param x X軸周りの回転（ピッチ）
     * @param y Y軸周りの回転（ヨー）
     * @param z Z軸周りの回転（ロール）
     */
    void SetRotation(float x, float y, float z);

    /**
     * @brief 現在のカメラの位置を取得します。
     * @return DirectX::XMFLOAT3型の位置座標
     */
    DirectX::XMFLOAT3 GetPosition() const;

    /**
     * @brief 現在のカメラの回転角度を取得します。
     * @return DirectX::XMFLOAT3型の回転角度
     */
    DirectX::XMFLOAT3 GetRotation() const;

    /**
     * @brief カメラの状態を更新し、ビュー行列を再計算します。
     */
    void Update();

    /**
    * @brief 計算済みのビュー行列を取得します。
    * @return DirectX::XMMATRIX型のビュー行列
    */
    DirectX::XMMATRIX GetViewMatrix() const;

    /**
    * @brief 計算済みの前フレームのビュープロジェクション行列を取得します。
    * @return DirectX::XMMATRIX型のビュープロジェクション行列
    */
    DirectX::XMMATRIX GetPreviousViewProjectionMatrix() const;

    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void Turn(int mouseX, int mouseY, float deltaTime);
    void UpdateBobbing(float deltaTime, bool isMoving);

private:
    // 位置
    float m_positionX, m_positionY, m_positionZ;
    DirectX::XMFLOAT3 m_basePosition; // ヘッドボブの基準位置
    // 回転
    float m_rotationX, m_rotationY, m_rotationZ;
    // ビュー行列
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;
    DirectX::XMMATRIX m_previousViewProjectionMatrix;
    // 速度
    float m_moveSpeed;
    float m_rotationSpeed;

    // ヘッドボブ用
    float m_bobbingTimer;
    float m_bobbingAmount;
    float m_bobbingSpeed;
    float m_swayAmount;
    float m_swaySpeed;
    float m_rollAmount;
    float m_rollSpeed;
};