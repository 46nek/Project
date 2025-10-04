#include "Model.h"

Model::Model()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* device)
{
    // ���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@��������
    return InitializeBuffers(device);
}

void Model::Shutdown()
{
    // �o�b�t�@�����
    ShutdownBuffers();
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
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

    // �����̂�8�̒��_�f�[�^ (�ʒu�ƐF)
    vertices[0].Pos = { -1.0f, -1.0f, -1.0f }; vertices[0].Color = { 1.0f, 0.0f, 0.0f, 1.0f };
    vertices[1].Pos = { -1.0f,  1.0f, -1.0f }; vertices[1].Color = { 0.0f, 1.0f, 0.0f, 1.0f };
    vertices[2].Pos = { 1.0f,  1.0f, -1.0f }; vertices[2].Color = { 0.0f, 0.0f, 1.0f, 1.0f };
    vertices[3].Pos = { 1.0f, -1.0f, -1.0f }; vertices[3].Color = { 1.0f, 1.0f, 0.0f, 1.0f };
    vertices[4].Pos = { -1.0f, -1.0f,  1.0f }; vertices[4].Color = { 1.0f, 0.0f, 1.0f, 1.0f };
    vertices[5].Pos = { -1.0f,  1.0f,  1.0f }; vertices[5].Color = { 0.0f, 1.0f, 1.0f, 1.0f };
    vertices[6].Pos = { 1.0f,  1.0f,  1.0f }; vertices[6].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    vertices[7].Pos = { 1.0f, -1.0f,  1.0f }; vertices[7].Color = { 0.0f, 0.0f, 0.0f, 1.0f };

    // �����̂�12�̎O�p�`���`����C���f�b�N�X�f�[�^
    // �O��
    indices[0] = 0; indices[1] = 1; indices[2] = 2;
    indices[3] = 0; indices[4] = 2; indices[5] = 3;
    // �w��
    indices[6] = 4; indices[7] = 6; indices[8] = 5;
    indices[9] = 4; indices[10] = 7; indices[11] = 6;
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