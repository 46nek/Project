// ���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[�̊ԂŃf�[�^��n�����߂̍\����
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION; // SV_POSITION �͕K�{�B���_�̍ŏI�I�ȍ��W
    float4 Color : COLOR; // COLOR �͔C�ӁB�s�N�Z���V�F�[�_�[�ɓn���F
};

// �s�N�Z���V�F�[�_�[
// ���_�V�F�[�_�[����󂯎�����������ƂɁA�s�N�Z���̍ŏI�I�ȐF�����肵�܂�
float4 PS(VS_OUTPUT input) : SV_Target
{
    return input.Color; // ���_�̐F�����̂܂܏o��
}