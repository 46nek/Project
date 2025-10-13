cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
};

// �s�N�Z���V�F�[�_�[�ɂ�SV_POSITION�i�X�N���[����̍��W�j�݂̂�n��
float4 main(VS_INPUT input) : SV_POSITION
{
    // ���_���W�����[���h�A���C�g�r���[�A���C�g�v���W�F�N�V�����̏��ɕϊ�����
    float4 worldPos = mul(input.Pos, worldMatrix);
    float4 viewPos = mul(worldPos, lightViewMatrix);
    float4 projPos = mul(viewPos, lightProjectionMatrix);

    return projPos;
}