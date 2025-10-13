// PostProcessVertexShader.hlsl (���̓��e�Ŋ��S�ɒu�������Ă�������)

// 3D���f���Ɠ����A���S�Ȓ��_�\������͂Ƃ��Ď󂯎��
struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    // �󂯎�����f�[�^�̂����A�ʒu�ƃe�N�X�`�����W�������s�N�Z���V�F�[�_�[�֓n��
    output.Pos = input.Pos;
    output.Tex = input.Tex;
    return output;
}