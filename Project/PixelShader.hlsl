Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

// ���C�g�̎��
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// �X�̃��C�g�̏���ێ�����\����
struct Light
{
    float4 Color;
    float3 Direction;
    float SpotAngle;
    float3 Position;
    float Range;
    float3 Attenuation;
    int Type;
    bool Enabled;
    float Intensity;
    float2 Padding;
};

// ���C�g�̏����i�[����R���X�^���g�o�b�t�@
cbuffer LightBuffer : register(b1)
{
    Light Lights[16];
    int NumLights;
    float3 CameraPosition;
};


struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
};


float4 PS(VS_OUTPUT input) : SV_Target
{
    // �e�N�X�`������I�u�W�F�N�g�̊�{�F���擾
    float4 textureColor = shaderTexture.Sample(SampleType, input.Tex);

    // �������v�Z (�e�N�X�`���F�𔽉f)
    float4 ambient = textureColor * float4(0.3f, 0.3f, 0.3f, 1.0f);

    // ���ꂩ��v�Z����g�U���Ƌ��ʔ��ˌ���������
    float4 totalDiffuse = float4(0, 0, 0, 0);
    float4 totalSpecular = float4(0, 0, 0, 0);

    // �J�����ւ̎����x�N�g�����v�Z
    float3 viewDir = normalize(CameraPosition - input.WorldPos);

    // �V�[�����̂��ׂẴ��C�g�����[�v����
    for (int i = 0; i < NumLights; ++i)
    {
        // ���C�g�������Ȃ�X�L�b�v
        if (!Lights[i].Enabled)
        {
            continue;
        }

        // --- ���C�g���Ƃ̊�{�����v�Z ---
        float3 lightDir;
        float distance;

        if (Lights[i].Type == DIRECTIONAL_LIGHT)
        {
            lightDir = -normalize(Lights[i].Direction);
            distance = 1.0f; // �����͖���
        }
        else
        {
            lightDir = normalize(Lights[i].Position - input.WorldPos);
            distance = length(Lights[i].Position - input.WorldPos);
        }

        // --- �����ɂ�錸�����v�Z ---
        float attenuation = 1.0f;
        if (Lights[i].Type != DIRECTIONAL_LIGHT)
        {
            if (distance > Lights[i].Range)
            {
                continue; // �͈͊O�Ȃ�X�L�b�v
            }
            attenuation = 1.0f / (Lights[i].Attenuation.x + Lights[i].Attenuation.y * distance + Lights[i].Attenuation.z * (distance * distance));
        }

        // --- �X�|�b�g���C�g�͈̔͂��v�Z ---
        float spotFactor = 1.0f;
        if (Lights[i].Type == SPOT_LIGHT)
        {
            float spotCos = dot(-lightDir, normalize(Lights[i].Direction));
            if (spotCos < Lights[i].SpotAngle)
            {
                continue; // �͈͊O�Ȃ�X�L�b�v
            }
            spotFactor = smoothstep(Lights[i].SpotAngle, Lights[i].SpotAngle - 0.05f, spotCos);
        }

        // --- �g�U��(Diffuse)���v�Z ---
        float diffuseIntensity = saturate(dot(input.Normal, lightDir));
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor;

        // --- ���ʔ��ˌ�(Specular)���v�Z ---
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(input.Normal, halfwayDir));
        float specular = pow(specAngle, 32.0f); // ����̋���
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor;
    }

    // --- �ŏI�I�ȐF������ ---
    // (�e�N�X�`���F x �g�U��) + ���ʔ��ˌ� + ����
    float4 finalColor = (textureColor * totalDiffuse) + totalSpecular + ambient;
    finalColor.a = textureColor.a; // ���̃A���t�@�l��ێ�

    return finalColor;
}