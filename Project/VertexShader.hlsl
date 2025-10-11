// �萔�o�b�t�@: CPU����GPU�֍s��f�[�^��n��
cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
};

// ���_�V�F�[�_�[�ւ̓��̓f�[�^�\��
struct VS_INPUT
{
    float4 Pos : POSITION; // ���_���W
    float4 Color : COLOR; // ���_�J���[
    float2 Tex : TEXCOORD0; // �e�N�X�`�����W
    float3 Normal : NORMAL; // �@���x�N�g��
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
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // ���W�ϊ�
    float4 worldPos = mul(input.Pos, WorldMatrix);
    output.WorldPos = worldPos.xyz;
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Pos = mul(viewPos, ProjectionMatrix);

    // �@���x�N�g�������[���h��Ԃɕϊ�
    output.Normal = mul(input.Normal, (float3x3) WorldInverseTransposeMatrix);
    output.Normal = normalize(output.Normal);

    // �V���h�E�}�b�s���O�̂��߂̍��W�ϊ�
    output.LightViewPos = mul(worldPos, LightViewMatrix);
    output.LightViewPos = mul(output.LightViewPos, LightProjectionMatrix);
    
    // �e�N�X�`�����W�ƃJ���[�����̂܂܃s�N�Z���V�F�[�_�[�֓n��
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}