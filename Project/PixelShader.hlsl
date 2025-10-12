// �e�N�X�`���ƃT���v���[�X�e�[�g
Texture2D shaderTexture : register(t0); // �I�u�W�F�N�g�̃e�N�X�`��
Texture2D shadowMapTexture : register(t1); // �V���h�E�}�b�v�i�[�x�e�N�X�`���j
Texture2D normalMapTexture : register(t2);
SamplerState SampleType : register(s0); // �ʏ�̃e�N�X�`���p�T���v���[
SamplerComparisonState ShadowSampleType : register(s1); // �V���h�E�}�b�v��r�p�T���v���[


// --- ���C�g�̒�` ---
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

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

// ���C�g�����i�[����R���X�^���g�o�b�t�@
cbuffer LightBuffer : register(b1)
{
    Light Lights[16];
    int NumLights;
    float3 CameraPosition;
};

// �s�N�Z���V�F�[�_�[�ւ̓��̓f�[�^�\��
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
    float4 LightViewPos : TEXCOORD1;
    float4 PrevPos : TEXCOORD2; // 1�t���[���O�̃X�N���[�����W
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

// MRT�p�Ƀs�N�Z���V�F�[�_�[�̏o�͂�ύX
struct PS_OUTPUT
{
    float4 Color : SV_Target0; // 0�Ԗڂ̃����_�[�^�[�Q�b�g�i�F�j
    float4 Velocity : SV_Target1; // 1�Ԗڂ̃����_�[�^�[�Q�b�g�i���x�j
};

PS_OUTPUT PS(VS_OUTPUT input)
{
    // �e�N�X�`�������{�F���擾
    float4 textureColor = shaderTexture.Sample(SampleType, input.Tex);

    // --- �m�[�}���}�b�s���O ---
    // TBN�s����\�z
    float3x3 tbnMatrix = float3x3(normalize(input.Tangent), normalize(input.Binormal), normalize(input.Normal));
    
    // �m�[�}���}�b�v����@�����T���v�����O���A[-1, 1]�͈̔͂ɕϊ�
    float3 normalMapSample = normalMapTexture.Sample(SampleType, input.Tex).rgb;
    float3 normalFromMap = (2.0f * normalMapSample) - 1.0f;
    
    // �m�[�}���}�b�v�̖@����ڐ���Ԃ��烏�[���h��Ԃ֕ϊ�
    float3 finalNormal = normalize(mul(normalFromMap, tbnMatrix));

    // ����
    float4 ambient = textureColor * float4(0.3f, 0.3f, 0.3f, 1.0f);

    // �g�U���Ƌ��ʔ��ˌ��̍��v��������
    float4 totalDiffuse = float4(0, 0, 0, 0);
    float4 totalSpecular = float4(0, 0, 0, 0);

    // �J�����ւ̎����x�N�g��
    float3 viewDir = normalize(CameraPosition - input.WorldPos);

    // --- �e�̌v�Z ---
    float shadowFactor = 1.0; // 1.0 = ����������, 0.0 = �e
    
    // ���C�g���_�ł̍��W���e�N�X�`�����W(0~1)�ɕϊ�
    float2 projectTexCoord;
    projectTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;

    // �e�N�X�`�����W���͈͓��ɂ���ꍇ�̂݉e�̌v�Z���s��
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // �V���h�E�}�b�v�̐[�x�l�ƌ��݂̃s�N�Z���̐[�x���r
        shadowFactor = shadowMapTexture.SampleCmpLevelZero(ShadowSampleType, projectTexCoord, input.LightViewPos.z / input.LightViewPos.w);
    }

    // --- ���C�e�B���O�̌v�Z ---
    for (int i = 0; i < NumLights; ++i)
    {
        if (!Lights[i].Enabled)
            continue;

        float3 lightDir;
        float distance;

        if (Lights[i].Type == DIRECTIONAL_LIGHT)
        {
            lightDir = -normalize(Lights[i].Direction);
            distance = 1.0f;
        }
        else
        {
            lightDir = normalize(Lights[i].Position - input.WorldPos);
            distance = length(Lights[i].Position - input.WorldPos);
        }

        // �����ɂ�錸��
        float attenuation = 1.0f;
        if (Lights[i].Type != DIRECTIONAL_LIGHT)
        {
            if (distance > Lights[i].Range)
                continue;
            attenuation = 1.0f / (Lights[i].Attenuation.x + Lights[i].Attenuation.y * distance + Lights[i].Attenuation.z * (distance * distance));
        }
        
        // �X�|�b�g���C�g�̌���
        float spotFactor = 1.0f;
        if (Lights[i].Type == SPOT_LIGHT)
        {
            float3 lightToPixelDir = -lightDir;
            float spotCos = dot(lightToPixelDir, normalize(Lights[i].Direction));
            float outerConeCos = Lights[i].SpotAngle;
            float innerConeCos = min(1.0f, outerConeCos + 0.15f);
            spotFactor = smoothstep(outerConeCos, innerConeCos, spotCos);
        }

        // �g�U���̌v�Z
        float diffuseIntensity = saturate(dot(finalNormal, lightDir));
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor;

        // ���ʔ��ˌ��̌v�Z 
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(finalNormal, halfwayDir));
        float specular = pow(specAngle, 32.0f);
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor;
    }

    // �ŏI�I�ȐF�̍���
    float4 finalColor = (textureColor * (totalDiffuse + ambient)) * shadowFactor + (totalSpecular * shadowFactor);
    finalColor.a = textureColor.a;

    // --- ���x�x�N�g���̌v�Z ---
    // ���݂ƑO�̍��W��-1~+1�͈̔͂ɐ��K��
    float2 currentScreenPos = input.Pos.xy / input.Pos.w;
    float2 prevScreenPos = input.PrevPos.xy / input.PrevPos.w;
    // ���x�x�N�g�����v�Z
    float2 velocity = (currentScreenPos - prevScreenPos) * 0.5f;

    // �o�͍\���̂Ɍ��ʂ��l�߂�
    PS_OUTPUT output; // <- �����̃X�y�[�X�������Ă��܂����I
    output.Color = finalColor;
    output.Velocity = float4(velocity, 0.0f, 1.0f);

    return output;
}