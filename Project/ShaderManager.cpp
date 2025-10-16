// ShaderManager.cpp (���̓��e�Ŋ��S�ɒu�������Ă�������)

#include "ShaderManager.h"

ShaderManager::ShaderManager()
	: m_vertexShader(nullptr), m_postProcessVertexShader(nullptr), m_pixelShader(nullptr),
	m_texturePixelShader(nullptr), m_motionBlurPixelShader(nullptr),
	m_depthVertexShader(nullptr), m_inputLayout(nullptr),
	m_simpleVertexShader(nullptr), m_simplePixelShader(nullptr)
{
}
ShaderManager::~ShaderManager() {}

bool ShaderManager::Initialize(ID3D11Device* device)
{
	// �e�V�F�[�_�[�̃R���p�C���ς݃o�C�i���f�[�^��ێ����邽�߂̕ϐ�
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* postProcessVsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* texturePsBlob = nullptr;
	ID3DBlob* motionBlurPsBlob = nullptr;
	ID3DBlob* depthVsBlob = nullptr;
	ID3DBlob* simpleVsBlob = nullptr;
	ID3DBlob* simplePsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr; // �G���[���b�Z�[�W�p

	HRESULT hr;

	// �ʏ�̒��_�V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr)) { vsBlob->Release(); return false; }

	// �|�X�g�v���Z�X�p���_�V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"PostProcessVertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &postProcessVsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(postProcessVsBlob->GetBufferPointer(), postProcessVsBlob->GetBufferSize(), nullptr, &m_postProcessVertexShader);
	if (FAILED(hr)) { postProcessVsBlob->Release(); return false; }

	// �ʏ�̃s�N�Z���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr)) { psBlob->Release(); return false; }

	// �e�N�X�`���p�s�N�Z���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"TexturePixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &texturePsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(texturePsBlob->GetBufferPointer(), texturePsBlob->GetBufferSize(), nullptr, &m_texturePixelShader);
	if (FAILED(hr)) { texturePsBlob->Release(); return false; }

	// ���[�V�����u���[�p�s�N�Z���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"MotionBlur.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &motionBlurPsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(motionBlurPsBlob->GetBufferPointer(), motionBlurPsBlob->GetBufferSize(), nullptr, &m_motionBlurPixelShader);
	if (FAILED(hr)) { motionBlurPsBlob->Release(); return false; }

	// �[�x�p���_�V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"DepthVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &depthVsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(depthVsBlob->GetBufferPointer(), depthVsBlob->GetBufferSize(), nullptr, &m_depthVertexShader);
	if (FAILED(hr)) { depthVsBlob->Release(); return false; }

	// UI�p�̃V���v���Ȓ��_�V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"SimpleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &simpleVsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(simpleVsBlob->GetBufferPointer(), simpleVsBlob->GetBufferSize(), nullptr, &m_simpleVertexShader);
	if (FAILED(hr)) { simpleVsBlob->Release(); return false; }

	// UI�p�̃V���v���ȃs�N�Z���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"SimplePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &simplePsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(simplePsBlob->GetBufferPointer(), simplePsBlob->GetBufferSize(), nullptr, &m_simplePixelShader);
	if (FAILED(hr)) { simplePsBlob->Release(); return false; }


	// ������ �������d�v�ȏC���_ ������
	// �C���v�b�g���C�A�E�g�̍쐬 (vsBlob�͂��̌�s�v�ɂȂ�)
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// vsBlob�͂����Ŏg����
	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
	if (FAILED(hr))
	{
		// �����ŃG���[�����������ꍇ�ɔ����āA���ׂĂ�blob���������
		vsBlob->Release();
		postProcessVsBlob->Release();
		psBlob->Release();
		texturePsBlob->Release();
		motionBlurPsBlob->Release();
		depthVsBlob->Release();
		simpleVsBlob->Release();
		simplePsBlob->Release();
		if (errorBlob) errorBlob->Release();
		return false;
	}

	// �S�ẴV�F�[�_�[�I�u�W�F�N�g�̍쐬�����������̂ŁA�s�v�ɂȂ���Blob�����ׂĉ������
	vsBlob->Release();
	postProcessVsBlob->Release();
	psBlob->Release();
	texturePsBlob->Release();
	motionBlurPsBlob->Release();
	depthVsBlob->Release();
	simpleVsBlob->Release();
	simplePsBlob->Release();
	if (errorBlob) errorBlob->Release();

	return true;
}

void ShaderManager::Shutdown()
{
	if (m_simplePixelShader) { m_simplePixelShader->Release(); m_simplePixelShader = nullptr; }
	if (m_simpleVertexShader) { m_simpleVertexShader->Release(); m_simpleVertexShader = nullptr; }
	if (m_inputLayout) { m_inputLayout->Release(); m_inputLayout = nullptr; }
	if (m_depthVertexShader) { m_depthVertexShader->Release(); m_depthVertexShader = nullptr; }
	if (m_motionBlurPixelShader) { m_motionBlurPixelShader->Release(); m_motionBlurPixelShader = nullptr; }
	if (m_texturePixelShader) { m_texturePixelShader->Release(); m_texturePixelShader = nullptr; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = nullptr; }
	if (m_postProcessVertexShader) { m_postProcessVertexShader->Release(); m_postProcessVertexShader = nullptr; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }
}