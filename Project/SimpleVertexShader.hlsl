// SimpleVertexShader.hlsl (���̓��e�Ŋ��S�ɒu�������Ă�������)

cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
};

// �A�v���P�[�V��������̓��͒��_�f�[�^
struct VS_INPUT
{
    float4 Pos : POSITION;
    // ��: ���̃V�F�[�_�[��POSITION�����g���܂��񂪁A���͍\���͑��̃V�F�[�_�[�ƍ��킹�܂�
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

// �s�N�Z���V�F�[�_�[�ւ̏o��
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 pos = input.Pos;

    // ���[���h�A�r���[�A�v���W�F�N�V�����s���K�p���č��W��ϊ�
    pos = mul(pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    output.Pos = pos;
    
    return output;
}