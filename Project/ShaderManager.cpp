// ShaderManager.cpp (この内容で完全に置き換えてください)

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
	// 各シェーダーのコンパイル済みバイナリデータを保持するための変数
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* postProcessVsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* texturePsBlob = nullptr;
	ID3DBlob* motionBlurPsBlob = nullptr;
	ID3DBlob* depthVsBlob = nullptr;
	ID3DBlob* simpleVsBlob = nullptr;
	ID3DBlob* simplePsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr; // エラーメッセージ用

	HRESULT hr;

	// 通常の頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr)) { vsBlob->Release(); return false; }

	// ポストプロセス用頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"PostProcessVertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &postProcessVsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(postProcessVsBlob->GetBufferPointer(), postProcessVsBlob->GetBufferSize(), nullptr, &m_postProcessVertexShader);
	if (FAILED(hr)) { postProcessVsBlob->Release(); return false; }

	// 通常のピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr)) { psBlob->Release(); return false; }

	// テクスチャ用ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"TexturePixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &texturePsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(texturePsBlob->GetBufferPointer(), texturePsBlob->GetBufferSize(), nullptr, &m_texturePixelShader);
	if (FAILED(hr)) { texturePsBlob->Release(); return false; }

	// モーションブラー用ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"MotionBlur.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &motionBlurPsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(motionBlurPsBlob->GetBufferPointer(), motionBlurPsBlob->GetBufferSize(), nullptr, &m_motionBlurPixelShader);
	if (FAILED(hr)) { motionBlurPsBlob->Release(); return false; }

	// 深度用頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"DepthVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &depthVsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(depthVsBlob->GetBufferPointer(), depthVsBlob->GetBufferSize(), nullptr, &m_depthVertexShader);
	if (FAILED(hr)) { depthVsBlob->Release(); return false; }

	// UI用のシンプルな頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"SimpleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &simpleVsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreateVertexShader(simpleVsBlob->GetBufferPointer(), simpleVsBlob->GetBufferSize(), nullptr, &m_simpleVertexShader);
	if (FAILED(hr)) { simpleVsBlob->Release(); return false; }

	// UI用のシンプルなピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"SimplePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &simplePsBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) errorBlob->Release(); return false; }
	hr = device->CreatePixelShader(simplePsBlob->GetBufferPointer(), simplePsBlob->GetBufferSize(), nullptr, &m_simplePixelShader);
	if (FAILED(hr)) { simplePsBlob->Release(); return false; }


	// ▼▼▼ ここが重要な修正点 ▼▼▼
	// インプットレイアウトの作成 (vsBlobはこの後不要になる)
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// vsBlobはここで使われる
	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
	if (FAILED(hr))
	{
		// ここでエラーが発生した場合に備えて、すべてのblobを解放する
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

	// 全てのシェーダーオブジェクトの作成が成功したので、不要になったBlobをすべて解放する
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