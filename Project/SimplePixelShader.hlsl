cbuffer MaterialBuffer : register(b2)
{
    float4 EmissiveColor;
    bool UseTexture;
    bool UseNormalMap;
    float2 Padding;
};

float4 main() : SV_TARGET
{
    // �}�e���A���o�b�t�@���玩�Ȕ����F���󂯎��A���̂܂܏o�͂���
    return EmissiveColor;
}