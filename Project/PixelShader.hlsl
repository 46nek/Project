// PixelShader.hlsl

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

// ���C�g�̏����i�[����V�����R���X�^���g�o�b�t�@
cbuffer LightBuffer : register(b1)
{
    // C++���̍\���̂Ə��Ԃ����킹��
    float4 DiffuseColor;
    float3 LightDirection;
};


struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
};

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // �e�N�X�`������F���擾
    textureColor = shaderTexture.Sample(SampleType, input.Tex);

    // ���C�g�̕����x�N�g���𔽓]
    lightDir = -normalize(LightDirection); //�O�̂��ߐ��K��

    // �@���ƃ��C�g�����̓��ς��v�Z���Č��̋��x������
    lightIntensity = saturate(dot(input.Normal, lightDir));

    // �����iAmbient Light�j����������������
    float4 ambientColor = float4(0.65f, 0.65f, 0.65f, 1.0f);
    
    // ���̋��x�ƐF���ŏI�I�ȐF�ɏ�Z���A���������Z
    color = saturate((textureColor * DiffuseColor * lightIntensity) + (textureColor * ambientColor));


    return color;
}