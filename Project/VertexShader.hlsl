// �萔�o�b�t�@: CPU����GPU�֍s��f�[�^��n��
cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
    matrix previousViewProjection;
};

// ���_�V�F�[�_�[�ւ̓��̓f�[�^�\��
struct VS_INPUT
{
    float4 Pos : POSITION; // ���_���W
    float4 Color : COLOR; // ���_�J���[
    float2 Tex : TEXCOORD0; // �e�N�X�`�����W
    float3 Normal : NORMAL; // �@���x�N�g��
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

// ���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̏o�̓f�[�^�\��
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION; // �X�N���[����̍��W
    float4 Color : COLOR; // ���_�J���[
    float2 Tex : TEXCOORD0; // �e�N�X�`�����W
    float3 Normal : NORMAL; // �@���x�N�g��
    float3 WorldPos : WORLDPOS; // ���[���h��Ԃł̍��W
    float4 LightViewPos : TEXCOORD1; // ���C�g���猩�����W
    float4 PrevPos : TEXCOORD2; // 1�t���[���O�̃X�N���[�����W
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    // ���݂̃t���[���̍��W�ϊ�
    float4 worldPos = mul(input.Pos, WorldMatrix);
    output.WorldPos = worldPos.xyz;
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Pos = mul(viewPos, ProjectionMatrix);
    
    // �@���A�ڐ��A�]�@�������[���h��Ԃɕϊ�
    output.Normal = normalize(mul(input.Normal, (float3x3) WorldInverseTransposeMatrix));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) WorldInverseTransposeMatrix));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) WorldInverseTransposeMatrix));

    // �V���h�E�}�b�s���O�̂��߂̍��W�ϊ�
    output.LightViewPos = mul(worldPos, LightViewMatrix);
    output.LightViewPos = mul(output.LightViewPos, LightProjectionMatrix);

    // 1�t���[���O�̃X�N���[�����W���v�Z
    output.PrevPos = mul(worldPos, previousViewProjection);

    // �e�N�X�`�����W�ƃJ���[�����̂܂܃s�N�Z���V�F�[�_�[�֓n��
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}