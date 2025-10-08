#pragma once
#include "Direct3D.h"
#include "Texture.h"
#include <assimp/scene.h>
#include <vector>
#include <string>

class Model
{
public:
    Model();
    ~Model();

    bool Initialize(ID3D11Device* device, const char* modelFilename);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

private:
    // ���b�V����\���\����
    struct Mesh {
        ID3D11Buffer* vertexBuffer = nullptr; 
        ID3D11Buffer* indexBuffer = nullptr;  
        int indexCount = 0;
        Texture* texture = nullptr;        
    };

    // Assimp���g���ă��f����ǂݍ���
    bool LoadModel(ID3D11Device* device, const std::string& filename);
    // ProcessNode��ProcessMesh�ɂ̓e�N�X�`���̃p�X���������邽�߂�directory������ǉ����܂�
    void ProcessNode(ID3D11Device* device, aiNode* node, const aiScene* scene, const std::string& directory);
    Mesh ProcessMesh(ID3D11Device* device, aiMesh* mesh, const aiScene* scene, const std::string& directory);

    // RenderBuffers�͓���̃��b�V����`�悷��悤�Ɉ��������܂�
    void RenderBuffers(ID3D11DeviceContext*, const Mesh& mesh);

private:
    std::vector<Mesh> m_meshes; // ���f�����̃��b�V����ێ�����x�N�^�[
};