// MotionBlur.hlsl (�C����)

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

    // �����`����Ă��Ȃ��s�N�Z��(��)�͉������Ȃ�
    if (depth >= 1.0f)
    {
        return sceneTexture.Sample(Sampler, tex);
    }

    // --- 1. ���݂̃s�N�Z���̃��[���h���W�𕜌� ---
    // �X�N���[�����W(tex)����-1�`+1�̍��W�n(NDC)�ɕϊ�
    float2 ndc = float2(tex.x * 2.0f - 1.0f, (1.0f - tex.y) * 2.0f - 1.0f);
    float4 currentPosH = float4(ndc, depth, 1.0f);
    // ���݂̃J�����̏����g���āA���[���h���W�ɖ߂�
    currentPosH = mul(currentPosH, currentViewProjectionInverse);
    currentPosH /= currentPosH.w;

    // --- 2. ���̃��[���h���W���u�O�̃t���[���v�ł͉�ʂ̂ǂ��ɂ��������v�Z ---
    float4 previousPosH = mul(currentPosH, previousViewProjection);
    previousPosH /= previousPosH.w;

    // --- 3.�u���݁v�Ɓu1�t���[���O�v�̃X�N���[�����W�̍������瓮���̃x�N�g�����v�Z ---
    float2 velocity = (ndc - previousPosH.xy) / 2.0f;

    // �u���[�̋�����K�p
    velocity *= blurAmount;

    // --- 4. �����̃x�N�g���ɉ����ĕ�����e�N�X�`�����T���v�����O���A���ς��Ƃ� ---
    float4 finalColor = 0;
    const int SAMPLES = 32; 
    for (int i = 0; i < SAMPLES; ++i)
    {
        finalColor += sceneTexture.Sample(Sampler, tex + velocity * (float(i) / (SAMPLES - 1)));
    }
    finalColor /= SAMPLES;

    return finalColor;
}