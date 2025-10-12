Texture2D sceneTexture : register(t0); // �`�悳�ꂽ�V�[��
Texture2D velocityTexture : register(t1); // ���x�}�b�v
SamplerState Sampler : register(s0);

static const int NUM_SAMPLES = 12; // �u���[�̕i���i�傫���قǍ��i�������d���j

float4 MotionBlurPS(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_Target
{
    // ���x�}�b�v���炱�̃s�N�Z���̑��x�x�N�g����ǂݎ��
    float2 velocity = velocityTexture.Sample(Sampler, tex).xy;

    // ���x���قڃ[���Ȃ�u���[���������Ɍ��̐F��Ԃ�
    if (length(velocity) < 0.0001f)
    {
        return sceneTexture.Sample(Sampler, tex);
    }

    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // ���x�x�N�g���̕����ɉ����āA�e�N�X�`���𕡐���T���v�����O����
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float scale = (float) i / (float) (NUM_SAMPLES - 1);
        finalColor += sceneTexture.Sample(Sampler, tex + velocity * scale);
    }

    // ���ς�����ău���[�F������
    return finalColor / NUM_SAMPLES;
}