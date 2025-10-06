#include "Model.h"

Model::Model()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_Texture = nullptr;
}

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* device)
{
    // ���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@��������
    if (!InitializeBuffers(device))
    {
        return false;
    }

    // Texture�I�u�W�F�N�g���쐬���ď�����
    m_Texture = new Texture;
    if (!m_Texture)
    {
        return false;
    }

    // �e�N�X�`����ǂݍ���
    if (!m_Texture->Initialize(device, L"crate.jpg"))
    {
        return false;
    }

    return true;
}

void Model::Shutdown()
{
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = nullptr;
    }

    // �o�b�t�@�����
    ShutdownBuffers();
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
    ID3D11ShaderResourceView* texture = m_Texture->GetTexture();
    deviceContext->PSSetShaderResources(0, 1, &texture);
    // �`��̂��߂Ƀo�b�t�@���p�C�v���C���ɐݒ�
    RenderBuffers(deviceContext);
}

int Model::GetIndexCount()
{
    return m_indexCount;
}

bool Model::InitializeBuffers(ID3D11Device* device)
{
    SimpleVertex* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    // �����̂�8�̒��_�ō\�������
    m_vertexCount = 8;

    // �����̂�12�̎O�p�`�i36�̃C���f�b�N�X�j�ō\�������
    m_indexCount = 36;

    // ���_�z����쐬
    vertices = new SimpleVertex[m_vertexCount];
    if (!vertices)
    {
        return false;
    }

    // �C���f�b�N�X�z����쐬
    indices = new unsigned long[m_indexCount];
    if (!indices)
    {
        return false;
    }
    // ���_�f�[�^�Ƀe�N�X�`�����W��ǉ� (�F�͔��œ���)
   // �O��
    vertices[0].Pos = { -1.0f, -1.0f, -1.0f }; vertices[0].Tex = { 0.0f, 1.0f }; vertices[0].Color = { 1,1,1,1 };
    vertices[1].Pos = { -1.0f,  1.0f, -1.0f }; vertices[1].Tex = { 0.0f, 0.0f }; vertices[1].Color = { 1,1,1,1 };
    vertices[2].Pos = { 1.0f,  1.0f, -1.0f }; vertices[2].Tex = { 1.0f, 0.0f }; vertices[2].Color = { 1,1,1,1 };
    vertices[3].Pos = { 1.0f, -1.0f, -1.0f }; vertices[3].Tex = { 1.0f, 1.0f }; vertices[3].Color = { 1,1,1,1 };

    // �w��
    vertices[4].Pos = { -1.0f, -1.0f,  1.0f }; vertices[4].Tex = { 1.0f, 1.0f }; vertices[4].Color = { 1,1,1,1 };
    vertices[5].Pos = { -1.0f,  1.0f,  1.0f }; vertices[5].Tex = { 1.0f, 0.0f }; vertices[5].Color = { 1,1,1,1 };
    vertices[6].Pos = { 1.0f,  1.0f,  1.0f }; vertices[6].Tex = { 0.0f, 0.0f }; vertices[6].Color = { 1,1,1,1 };
    vertices[7].Pos = { 1.0f, -1.0f,  1.0f }; vertices[7].Tex = { 0.0f, 1.0f }; vertices[7].Color = { 1,1,1,1 };

    // �e�N�X�`�����������\����悤�ɃC���f�b�N�X�f�[�^���X�V
    // �O��
    indices[0] = 0; indices[1] = 1; indices[2] = 2;
    indices[3] = 0; indices[4] = 2; indices[5] = 3;
    // �w��
    indices[6] = 7; indices[7] = 6; indices[8] = 5;
    indices[9] = 7; indices[10] = 5; indices[11] = 4;
    // ������
    indices[12] = 4; indices[13] = 5; indices[14] = 1;
    indices[15] = 4; indices[16] = 1; indices[17] = 0;
    // �E����
    indices[18] = 3; indices[19] = 2; indices[20] = 6;
    indices[21] = 3; indices[22] = 6; indices[23] = 7;
    // ���
    indices[24] = 1; indices[25] = 5; indices[26] = 6;
    indices[27] = 1; indices[28] = 6; indices[29] = 2;
    // ���
    indices[30] = 4; indices[31] = 0; indices[32] = 3;
    indices[33] = 4; indices[34] = 3; indices[35] = 7;

    // ���_�o�b�t�@�̐ݒ�
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexData.pSysMem = vertices;
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result)) return false;

    // �C���f�b�N�X�o�b�t�@�̐ݒ�
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexData.pSysMem = indices;
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result)) return false;

    // �쐬��͔z������
    delete[] vertices;
    vertices = nullptr;
    delete[] indices;
    indices = nullptr;

    return true;
}

void Model::ShutdownBuffers()
{
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride = sizeof(SimpleVertex);
    unsigned int offset = 0;

    // ���_�o�b�t�@���Z�b�g
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    // �C���f�b�N�X�o�b�t�@���Z�b�g
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    // �v���~�e�B�u�^�C�v���Z�b�g
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}