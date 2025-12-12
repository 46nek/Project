#include "ShaderManager.h"
#include <d3dcompiler.h>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

ShaderManager::ShaderManager()
	: m_vertexShader(nullptr), m_postProcessVertexShader(nullptr), m_pixelShader(nullptr),
	m_texturePixelShader(nullptr), m_motionBlurPixelShader(nullptr),
	m_depthVertexShader(nullptr), m_inputLayout(nullptr),
	m_simpleVertexShader(nullptr), m_simplePixelShader(nullptr) {
}
ShaderManager::~ShaderManager() {
}

// エラーメッセージを表示するヘルパー関数
void OutputShaderError(ID3DBlob* errorMessage, const char* shaderFilename) {
	if (errorMessage) {
		MessageBoxA(nullptr, (char*)errorMessage->GetBufferPointer(), shaderFilename, MB_OK); // NULL -> nullptr
		errorMessage->Release();
	}
	else {
		MessageBoxA(nullptr, "Shader Compilation Failed (No Error Message)", shaderFilename, MB_OK); // NULL -> nullptr
	}
}

bool ShaderManager::Initialize(ID3D11Device* device) {
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* postProcessVsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* texturePsBlob = nullptr;
	ID3DBlob* motionBlurPsBlob = nullptr;
	ID3DBlob* depthVsBlob = nullptr;
	ID3DBlob* simpleVsBlob = nullptr;
	ID3DBlob* simplePsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr;

	// 通常の頂点シェーダー
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "VertexShader.hlsl"); return false; }
	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr)) { vsBlob->Release(); return false; }

	// ポストプロセス用頂点シェーダー
	hr = D3DCompileFromFile(L"PostProcessVertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &postProcessVsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "PostProcessVertexShader.hlsl"); return false; }
	hr = device->CreateVertexShader(postProcessVsBlob->GetBufferPointer(), postProcessVsBlob->GetBufferSize(), nullptr, &m_postProcessVertexShader);
	if (FAILED(hr)) { postProcessVsBlob->Release(); return false; }

	// 通常のピクセルシェーダー
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "PixelShader.hlsl"); return false; }
	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr)) { psBlob->Release(); return false; }

	// テクスチャ用ピクセルシェーダー
	hr = D3DCompileFromFile(L"TexturePixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &texturePsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "TexturePixelShader.hlsl"); return false; }
	hr = device->CreatePixelShader(texturePsBlob->GetBufferPointer(), texturePsBlob->GetBufferSize(), nullptr, &m_texturePixelShader);
	if (FAILED(hr)) { texturePsBlob->Release(); return false; }

	// モーションブラー用ピクセルシェーダー
	hr = D3DCompileFromFile(L"MotionBlur.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &motionBlurPsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "MotionBlur.hlsl"); return false; }
	hr = device->CreatePixelShader(motionBlurPsBlob->GetBufferPointer(), motionBlurPsBlob->GetBufferSize(), nullptr, &m_motionBlurPixelShader);
	if (FAILED(hr)) { motionBlurPsBlob->Release(); return false; }

	// 深度用頂点シェーダー
	hr = D3DCompileFromFile(L"DepthVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &depthVsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "DepthVertexShader.hlsl"); return false; }
	hr = device->CreateVertexShader(depthVsBlob->GetBufferPointer(), depthVsBlob->GetBufferSize(), nullptr, &m_depthVertexShader);
	if (FAILED(hr)) { depthVsBlob->Release(); return false; }

	// UI用頂点シェーダー
	hr = D3DCompileFromFile(L"SimpleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &simpleVsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "SimpleVertexShader.hlsl"); return false; }
	hr = device->CreateVertexShader(simpleVsBlob->GetBufferPointer(), simpleVsBlob->GetBufferSize(), nullptr, &m_simpleVertexShader);
	if (FAILED(hr)) { simpleVsBlob->Release(); return false; }

	// UI用ピクセルシェーダー
	hr = D3DCompileFromFile(L"SimplePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &simplePsBlob, &errorBlob);
	if (FAILED(hr)) { OutputShaderError(errorBlob, "SimplePixelShader.hlsl"); return false; }
	hr = device->CreatePixelShader(simplePsBlob->GetBufferPointer(), simplePsBlob->GetBufferSize(), nullptr, &m_simplePixelShader);
	if (FAILED(hr)) { simplePsBlob->Release(); return false; }

	// インプットレイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
	if (FAILED(hr)) {
		vsBlob->Release();
		postProcessVsBlob->Release();
		psBlob->Release();
		texturePsBlob->Release();
		motionBlurPsBlob->Release();
		depthVsBlob->Release();
		simpleVsBlob->Release();
		simplePsBlob->Release();
		return false;
	}

	vsBlob->Release();
	postProcessVsBlob->Release();
	psBlob->Release();
	texturePsBlob->Release();
	motionBlurPsBlob->Release();
	depthVsBlob->Release();
	simpleVsBlob->Release();
	simplePsBlob->Release();

	return true;
}

void ShaderManager::Shutdown() {
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