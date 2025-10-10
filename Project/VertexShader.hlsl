cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};
struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS; // ���[���h���W�ł̈ʒu��ǉ�
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    // ���_�ʒu�̕ϊ�
    float4 worldPos = mul(input.Pos, WorldMatrix);
    output.WorldPos = worldPos.xyz; // ���[���h���W���s�N�Z���V�F�[�_�[��

    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Pos = mul(viewPos, ProjectionMatrix);

    // �@�������[���h��Ԃɕϊ����ăs�N�Z���V�F�[�_�[�ɓn��
    output.Normal = mul(input.Normal, (float3x3) WorldMatrix);
    output.Normal = normalize(output.Normal);

    // �e�N�X�`�����W�ƐF��n��
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}