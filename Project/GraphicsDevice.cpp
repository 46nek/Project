#include "GraphicsDevice.h"
#include "LightManager.h"

GraphicsDevice::GraphicsDevice()
	: m_d3dDevice(nullptr), m_immediateContext(nullptr),
	m_matrixBuffer(nullptr), m_lightBuffer(nullptr), m_motionBlurBuffer(nullptr),
	m_materialBuffer(nullptr), m_postProcessBuffer(nullptr),
	m_samplerState(nullptr),
	m_alphaBlendState(nullptr),
	m_defaultBlendState(nullptr),
	m_defaultRasterizerState(nullptr) {
}

GraphicsDevice::~GraphicsDevice() {
}

bool GraphicsDevice::Initialize(HWND hWnd, int screenWidth, int screenHeight) {
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
		featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		&m_d3dDevice, nullptr, &m_immediateContext);
	if (FAILED(hr)) { return false; }

	m_swapChain = std::make_unique<SwapChain>();
	if (!m_swapChain->Initialize(m_d3dDevice, hWnd, screenWidth, screenHeight)) { return false; }

	m_shaderManager = std::make_unique<ShaderManager>();
	if (!m_shaderManager->Initialize(m_d3dDevice)) { return false; }

	m_shadowMapper = std::make_unique<ShadowMapper>();
	if (!m_shadowMapper->Initialize(m_d3dDevice)) { return false; }

	m_renderTarget = std::make_unique<RenderTarget>();
	if (!m_renderTarget->Initialize(m_d3dDevice, screenWidth, screenHeight, true)) { return false; }

	m_orthoWindow = std::make_unique<OrthoWindow>();
	if (!m_orthoWindow->Initialize(m_d3dDevice, screenWidth, screenHeight)) { return false; }

	D3D11_BUFFER_DESC matrixBufferDesc = {};
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3dDevice->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);
	if (FAILED(hr)) { return false; }

	D3D11_BUFFER_DESC lightBufferDesc = {};
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3dDevice->CreateBuffer(&lightBufferDesc, nullptr, &m_lightBuffer);
	if (FAILED(hr)) { return false; }

	D3D11_BUFFER_DESC motionBlurBufferDesc = {};
	motionBlurBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	motionBlurBufferDesc.ByteWidth = sizeof(MotionBlurBufferType);
	motionBlurBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	motionBlurBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3dDevice->CreateBuffer(&motionBlurBufferDesc, nullptr, &m_motionBlurBuffer);
	if (FAILED(hr)) { return false; }

	D3D11_BUFFER_DESC materialBufferDesc = {};
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3dDevice->CreateBuffer(&materialBufferDesc, nullptr, &m_materialBuffer);
	if (FAILED(hr)) { return false; }

	D3D11_BUFFER_DESC postProcessBufferDesc = {};
	postProcessBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	postProcessBufferDesc.ByteWidth = sizeof(PostProcessBufferType);
	postProcessBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	postProcessBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3dDevice->CreateBuffer(&postProcessBufferDesc, nullptr, &m_postProcessBuffer);
	if (FAILED(hr)) { return false; }

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_d3dDevice->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(hr)) { return false; }

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;
	hr = m_d3dDevice->CreateRasterizerState(&rasterDesc, &m_defaultRasterizerState);
	if (FAILED(hr)) { return false; }

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_d3dDevice->CreateBlendState(&blendDesc, &m_defaultBlendState);
	if (FAILED(hr)) { return false; }

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	hr = m_d3dDevice->CreateBlendState(&blendDesc, &m_alphaBlendState);
	if (FAILED(hr)) { return false; }

	return true;
}

void GraphicsDevice::Shutdown() {
	if (m_defaultRasterizerState) m_defaultRasterizerState->Release();
	if (m_alphaBlendState) m_alphaBlendState->Release();
	if (m_defaultBlendState) m_defaultBlendState->Release();
	if (m_samplerState) m_samplerState->Release();
	if (m_lightBuffer) m_lightBuffer->Release();
	if (m_matrixBuffer) m_matrixBuffer->Release();
	if (m_materialBuffer) m_materialBuffer->Release();
	if (m_motionBlurBuffer) m_motionBlurBuffer->Release();
	if (m_postProcessBuffer) m_postProcessBuffer->Release();
	if (m_orthoWindow) m_orthoWindow->Shutdown();
	if (m_renderTarget) m_renderTarget->Shutdown();
	if (m_shadowMapper) m_shadowMapper->Shutdown();
	if (m_shaderManager) m_shaderManager->Shutdown();
	if (m_swapChain) m_swapChain->Shutdown();
	if (m_immediateContext) m_immediateContext->Release();
	if (m_d3dDevice) m_d3dDevice->Release();
}

void GraphicsDevice::BeginScene(float r, float g, float b, float a) {
	m_swapChain->BeginScene(m_immediateContext, r, g, b, a);
}

void GraphicsDevice::EndScene() {
	m_swapChain->EndScene();
}

bool GraphicsDevice::UpdateMatrixBuffer(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMMATRIX& lightView, const DirectX::XMMATRIX& lightProjection) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_immediateContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) { return false; }
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = DirectX::XMMatrixTranspose(world);
	dataPtr->view = DirectX::XMMatrixTranspose(view);
	dataPtr->projection = DirectX::XMMatrixTranspose(projection);
	dataPtr->worldInverseTranspose = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));
	dataPtr->lightView = DirectX::XMMatrixTranspose(lightView);
	dataPtr->lightProjection = DirectX::XMMatrixTranspose(lightProjection);
	m_immediateContext->Unmap(m_matrixBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	return true;
}

bool GraphicsDevice::UpdateLightBuffer(const LightBufferType& lightBuffer) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_immediateContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) { return false; }
	LightBufferType* dataPtr = (LightBufferType*)mappedResource.pData;
	*dataPtr = lightBuffer;
	m_immediateContext->Unmap(m_lightBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(1, 1, &m_lightBuffer);
	return true;
}

bool GraphicsDevice::UpdateMotionBlurBuffer(const DirectX::XMMATRIX& prevViewProj, const DirectX::XMMATRIX& currentViewProjInv, float blurAmount) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_immediateContext->Map(m_motionBlurBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) { return false; }
	MotionBlurBufferType* dataPtr = (MotionBlurBufferType*)mappedResource.pData;
	dataPtr->previousViewProjection = DirectX::XMMatrixTranspose(prevViewProj);
	dataPtr->currentViewProjectionInverse = DirectX::XMMatrixTranspose(currentViewProjInv);
	dataPtr->blurAmount = blurAmount;
	m_immediateContext->Unmap(m_motionBlurBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_motionBlurBuffer);
	return true;
}

bool GraphicsDevice::UpdateMaterialBuffer(const MaterialBufferType& materialBuffer) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_immediateContext->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) { return false; }
	MaterialBufferType* dataPtr = (MaterialBufferType*)mappedResource.pData;
	*dataPtr = materialBuffer;
	m_immediateContext->Unmap(m_materialBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(2, 1, &m_materialBuffer);
	return true;
}

bool GraphicsDevice::UpdatePostProcessBuffer(const PostProcessBufferType& postProcessBuffer) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_immediateContext->Map(m_postProcessBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) { return false; }
	PostProcessBufferType* dataPtr = (PostProcessBufferType*)mappedResource.pData;
	*dataPtr = postProcessBuffer;
	m_immediateContext->Unmap(m_postProcessBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(3, 1, &m_postProcessBuffer);
	return true;
}

ID3D11BlendState* GraphicsDevice::GetAlphaBlendState() const {
	return m_alphaBlendState;
}

ID3D11BlendState* GraphicsDevice::GetDefaultBlendState() const {
	return m_defaultBlendState;
}

ID3D11RasterizerState* GraphicsDevice::GetDefaultRasterizerState() const {
	return m_defaultRasterizerState;
}