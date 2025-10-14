// �e�N�X�`���ƃT���v���[�X�e�[�g
Texture2D shaderTexture : register(t0); // �I�u�W�F�N�g�̃e�N�X�`��
Texture2D shadowMapTexture : register(t1); // �V���h�E�}�b�v�i�[�x�e�N�X�`���j
Texture2D normalMapTexture : register(t2);
SamplerState SampleType : register(s0); // �ʏ�̃e�N�X�`���p�T���v���[
SamplerComparisonState ShadowSampleType : register(s1); // �V���h�E�}�b�v��r�p�T���v���[

cbuffer MaterialBuffer : register(b2)
{
    float4 EmissiveColor;
    bool UseTexture;
    float3 Padding;
};

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
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

float4 PS(VS_OUTPUT input) : SV_Target
{
    // --- �e�N�X�`���F�̎擾�������C�� ---
    float4 textureColor;
    if (UseTexture)
    {
        // UseTexture��true�Ȃ�e�N�X�`������F���擾
        textureColor = shaderTexture.Sample(SampleType, input.Tex);
    }
    else
    {
        // false�Ȃ甒�F����{�F�Ƃ���
        textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    // --- �m�[�}���}�b�s���O ---
    float3x3 tbnMatrix = float3x3(normalize(input.Tangent), normalize(input.Binormal), normalize(input.Normal));
    float3 normalMapSample = normalMapTexture.Sample(SampleType, input.Tex).rgb;
    float3 normalFromMap = (2.0f * normalMapSample) - 1.0f;
    float3 finalNormal = normalize(mul(normalFromMap, tbnMatrix));
    // ����
    float4 ambient = textureColor * float4(0.3f, 0.3f, 0.3f, 1.0f);
    // �g�U���Ƌ��ʔ��ˌ��̍��v��������
    float4 totalDiffuse = float4(0, 0, 0, 0);
    float4 totalSpecular = float4(0, 0, 0, 0);
    // �J�����ւ̎����x�N�g��
    float3 viewDir = normalize(CameraPosition - input.WorldPos);
    // --- �e�̌v�Z ---
    float shadowFactor = 1.0f;
    float2 projectTexCoord;
    projectTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
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
            distance = 0.0f; // �f�B���N�V���i�����C�g�̋�����0�Ƃ���
        }
        else
        {
            lightDir = normalize(Lights[i].Position - input.WorldPos);
            distance = length(Lights[i].Position - input.WorldPos);
        }
        
        float attenuation = 1.0f;
        if (Lights[i].Type != DIRECTIONAL_LIGHT)
        {
            // �]���̋����ɂ�錸��
            attenuation = 1.0f / (Lights[i].Attenuation.x + Lights[i].Attenuation.y * distance + Lights[i].Attenuation.z * (distance * distance));

            // Range�Ɋ�Â��ăX���[�Y�Ƀt�F�[�h�A�E�g������v�Z��ǉ�
            // (1 - ����/�͈�)^2 ���v�Z���A���炩�Ȍ����J�[�u�����
            float falloff = pow(saturate(1.0 - distance / Lights[i].Range), 2);
            
            // 2�̌������|�����킹��
            attenuation *= falloff;
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

        // �g�U���̌v�Z (�e���C�g���Ƃɉe��K�p)
        float diffuseIntensity = saturate(dot(finalNormal, lightDir));
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor * shadowFactor;
        
        // ���ʔ��ˌ��̌v�Z (�e���C�g���Ƃɉe��K�p)
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(finalNormal, halfwayDir));
        float specular = pow(specAngle, 32.0f);
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor * shadowFactor;
    }

    // --- �ŏI�I�ȐF�̍����������C�� ---
    // ���C�e�B���O�v�Z���ʂƊ���������
    float4 finalColor = (textureColor * (totalDiffuse + ambient)) + totalSpecular;
    
    // ���Ȕ����F�����Z
    finalColor += EmissiveColor;

    finalColor.a = 1.0f; // �A���t�@�l��1�ŌŒ�

    // --- Vignette Effect ---
    float2 screenCenter = float2(0.5f, 0.5f);
    float2 texCoord = input.Pos.xy / float2(1280, 720);
    float dist = distance(texCoord, screenCenter);
    float vignetteIntensity = 1.0f;
    float vignetteSmoothness = 0.1f;
    float vignette = 1.0f - smoothstep(vignetteSmoothness, 1.0f, dist * vignetteIntensity);
    finalColor.rgb *= vignette;
    finalColor.a = textureColor.a;

    return finalColor;
}