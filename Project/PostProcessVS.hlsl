struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    // ���_���W�ƃe�N�X�`�����W�����̂܂ܓn��
    output.Pos = input.Pos;
    output.Tex = input.Tex;
    return output;
}