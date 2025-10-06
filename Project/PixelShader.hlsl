// �e�N�X�`���ƃT���v���[���`
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

// ���_�V�F�[�_�[����n�����f�[�^�̍\����
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
};

// �s�N�Z���V�F�[�_�[�{��
float4 PS(VS_OUTPUT input) : SV_Target
{
    //  �T���v���[���g���ăe�N�X�`������s�N�Z���̐F���擾����
    return shaderTexture.Sample(SampleType, input.Tex);
}