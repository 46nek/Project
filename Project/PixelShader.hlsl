Texture2D shaderTexture : register(t0);
Texture2D shadowMapTexture : register(t1);
SamplerState SampleType : register(s0);
SamplerComparisonState ShadowSampleType : register(s1);

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
    float4 LightViewPos : TEXCOORD1;
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

    float shadowFactor = 1.0;
    // �e�̉e���W�� (1.0 = ����������, 0.0 = �e)

    // 1. �������_�ł̍��W�𐳋K���f�o�C�X���W(-1~+1)����e�N�X�`�����W(0~1)�ɕϊ�
    float2 projectTexCoord;
    projectTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;
    // 2. �e�N�X�`�����W��0~1�͈͓̔��ɂ���ꍇ�̂݉e�̌v�Z���s��
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // 3. �V���h�E�}�b�v����[�x�l���擾���A���݂̃s�N�Z���̐[�x�Ɣ�r����
        // SampleCmpLevelZero�́A�n�[�h�E�F�A�̋@�\���g���Ĕ�r���s���A�e�Ȃ�0.0�A�����łȂ����1.0��Ԃ�
        shadowFactor = shadowMapTexture.SampleCmpLevelZero(ShadowSampleType, projectTexCoord, input.LightViewPos.z / input.LightViewPos.w);
    }

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
        
        float spotFactor = 1.0f;
        if (Lights[i].Type == SPOT_LIGHT)
        {
            float3 lightToPixelDir = -lightDir;
            float spotCos = dot(lightToPixelDir, normalize(Lights[i].Direction));

            // �O���̃R�[���̊p�x�i�R�T�C���j
            float outerConeCos = Lights[i].SpotAngle;
            // �����̃R�[���̊p�x�i�R�T�C���j�B�������S�ɓ����镔���B
            // �R�T�C���l�͊p�x���������قǑ傫���Ȃ�̂ŁA�����l�𑫂��܂��B
            float innerConeCos = min(1.0f, outerConeCos + 0.05f);

            // smoothstep���g���āA�����ƊO���̊ԂŊ��炩�Ɍ��̋�����ω�������
            // ����ɂ��A�s�N�Z�����R�[���̊O���Ȃ�spotFactor��0�ɂȂ�܂��B
            spotFactor = smoothstep(outerConeCos, innerConeCos, spotCos);
        }

        // --- �g�U��(Diffuse)���v�Z ---
        float diffuseIntensity = saturate(dot(input.Normal, lightDir));
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor;
        // --- ���ʔ��ˌ�(Specular)���v�Z ---
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(input.Normal, halfwayDir));
        float specular = pow(specAngle, 32.0f);
        // ����̋���
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor;
    }

    float4 finalColor = (textureColor * (totalDiffuse + ambient)) * shadowFactor + (totalSpecular * shadowFactor);
    finalColor.a = textureColor.a;

    return finalColor;
}