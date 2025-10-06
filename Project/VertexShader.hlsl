// VertexShader.hlsl

// C++��MatrixBufferType�\���̂ƈ�v������
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
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // �ϊ����ʂ��ꎞ�ϐ��Ɋi�[���A���Ԃɏ�Z���Ă���
    float4 pos = mul(input.Pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    output.Pos = pos;

    // �F�͂��̂܂ܓn��
    output.Color = input.Color;

    return output;
}