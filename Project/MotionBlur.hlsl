Texture2D sceneTexture : register(t0);
Texture2D depthTexture : register(t1);
SamplerState Sampler : register(s0);

cbuffer MotionBlurBuffer : register(b0)
{
    matrix previousViewProjection;
    matrix currentViewProjectionInverse;
    float blurAmount;
    float3 padding;
};

float4 main(float4 position : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET
{
    // �[�x�e�N�X�`������[�x�l���擾
    float depth = depthTexture.Sample(Sampler, tex).r;

    // �[�x��1.0�i��j�̏ꍇ�̓u���[��K�p���Ȃ�
    if (depth >= 1.0f)
    {
        return sceneTexture.Sample(Sampler, tex);
    }

    // ���݂̃s�N�Z���̈ʒu��NDC�i���K���f�o�C�X���W�j����r���[��Ԃ֕ϊ�
    float4 currentPosH;
    currentPosH.x = tex.x * 2.0f - 1.0f;
    currentPosH.y = (1.0f - tex.y) * 2.0f - 1.0f;
    currentPosH.z = depth;
    currentPosH.w = 1.0f;

    // ���[���h���W�ɕϊ�
    currentPosH = mul(currentPosH, currentViewProjectionInverse);
    currentPosH /= currentPosH.w;

    // �O�̃t���[���ł̃X�N���[�����W���v�Z
    float4 previousPosH = mul(currentPosH, previousViewProjection);
    previousPosH /= previousPosH.w;

    // �O�̃t���[���ƌ��݂̃t���[���̃X�N���[�����W�̍������瑬�x���v�Z
    float2 velocity = (currentPosH.xy - previousPosH.xy) * blurAmount;

    // �V�[���e�N�X�`���𕡐���T���v�����O���ău���[���ʂ𐶐�
    float4 finalColor = 0;
    const int SAMPLES = 16; // �T���v�����O��
    for (int i = 0; i < SAMPLES; ++i)
    {
        finalColor += sceneTexture.Sample(Sampler, tex + velocity * (float(i) / float(SAMPLES - 1)));
    }

    // ���ς����
    finalColor /= SAMPLES;

    return finalColor;
}