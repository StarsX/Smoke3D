//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSDXBuffer.h"

using namespace DirectX;
using namespace DX;
using namespace XSDX;

Buffer::Buffer(const CPDXDevice &pDXDevice) :
	m_pSRV(nullptr),
	m_pDXDevice(pDXDevice)
{
}

Buffer::~Buffer(void)
{
}

const CPDXShaderResourceView &Buffer::GetSRV() const
{
	return m_pSRV;
}

void Buffer::CreateReadBuffer(const CPDXDevice &pDXDevice, CPDXBuffer &pDstBuffer, const CPDXBuffer &pSrcBuffer)
{
	auto desc = D3D11_BUFFER_DESC();
	pSrcBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	ThrowIfFailed(pDXDevice->CreateBuffer(&desc, nullptr, &pDstBuffer));
}

//--------------------------------------------------------------------------------------
// 2D Texture
//--------------------------------------------------------------------------------------

Texture2D::Texture2D(const CPDXDevice &pDXDevice) :
	Buffer(pDXDevice),
	m_pTexture(nullptr),
	m_vpUAVs(0)
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::Create(const uint32_t uWidth, const uint32_t uHeight,
	const uint32_t uArraySize, const DXGI_FORMAT eFormat, const bool bUAV,
	const bool bSRV, const bool bDyn, const uint8_t uMips,
	const lpcvoid pInitialData, const uint8_t uStride)
{
	// Setup the texture description.
	const auto textureDesc = CD3D11_TEXTURE2D_DESC(eFormat, uWidth, uHeight, uArraySize,
		uMips, D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA { pInitialData, uStride * uWidth, 0 };
		ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, &bufferInitData, &m_pTexture));
	}
	else ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, nullptr, &m_pTexture));

	// Create SRV
	if (bSRV) CreateSRV(uArraySize);

	// Create UAV
	if (bUAV) CreateUAV(uArraySize, uMips);
}

void Texture2D::Create(const uint32_t uWidth, const uint32_t uHeight, const DXGI_FORMAT eFormat,
	const bool bUAV, const bool bSRV, const bool bDyn, const uint8_t uMips,
	const lpcvoid pInitialData, const uint8_t uStride)
{
	Create(uWidth, uHeight, 1, eFormat, bUAV, bSRV, bDyn, uMips, pInitialData, uStride);
}

void Texture2D::CreateSRV(const uint32_t uArraySize, const uint8_t uSamples)
{
	const auto pTexture = m_pTexture.Get();

	// Setup the description of the shader resource view.
	const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pTexture, uArraySize > 1 ?
		(uSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY) :
		(uSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D));

	// Create the shader resource view.
	ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
}

void Texture2D::CreateUAV(const uint32_t uArraySize, const uint8_t uMips)
{
	const auto pTexture = m_pTexture.Get();

	auto uMip = 0ui8;
	VEC_ALLOC(m_vpUAVs, max(uMips, 1));
	for (auto &pUAV : m_vpUAVs)
	{
		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pTexture, uArraySize > 1 ?
			D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT_UNKNOWN, uMip++);

		// Create the unordered access view.
		ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pTexture, &uavDesc, &pUAV));
	}
}

const CPDXTexture2D &Texture2D::GetBuffer() const
{
	return m_pTexture;
}

const CPDXUnorderedAccessView &Texture2D::GetUAV(const uint8_t i) const
{
	return m_vpUAVs[i];
}

//--------------------------------------------------------------------------------------
// Render target
//--------------------------------------------------------------------------------------

RenderTarget::RenderTarget(const CPDXDevice &pDXDevice) :
	Texture2D(pDXDevice),
	m_vvpRTVs(0)
{
}

RenderTarget::~RenderTarget()
{
}

void RenderTarget::Create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
	const DXGI_FORMAT eFormat, const uint8_t uSamples, const uint8_t uMips, const bool bUAV)
{
	create(uWidth, uHeight, uArraySize, eFormat, uSamples, uMips, bUAV);
	const auto pTexture = m_pTexture.Get();

	auto uMip = 0ui8, uSlice = 0ui8;
	VEC_ALLOC(m_vvpRTVs, uArraySize);
	for (auto &vpRTVs : m_vvpRTVs)
	{
		VEC_ALLOC(vpRTVs, max(uMips, 1));
		for (auto &pRTV : vpRTVs)
		{
			// Setup the description of the render target view.
			const auto rtvDesc = CD3D11_RENDER_TARGET_VIEW_DESC(pTexture, uArraySize > 1 ?
				(uSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY) :
				(uSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D),
				DXGI_FORMAT_UNKNOWN, uMip++, uSlice++, 1);

			// Create the render target view.
			ThrowIfFailed(m_pDXDevice->CreateRenderTargetView(pTexture, &rtvDesc, &pRTV));
		}
	}
}

void RenderTarget::Create(const uint32_t uWidth, const uint32_t uHeight, const DXGI_FORMAT eFormat,
	const uint8_t uSamples, const uint8_t uMips, const bool bUAV)
{
	Create(uWidth, uHeight, 1, eFormat, uSamples, uMips, bUAV);
}

void RenderTarget::CreateArray(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
	const DXGI_FORMAT eFormat, const uint8_t uSamples, const uint8_t uMips, const bool bUAV)
{
	create(uWidth, uHeight, uArraySize, eFormat, uSamples, uMips, bUAV);
	const auto pTexture = m_pTexture.Get();

	VEC_ALLOC(m_vvpRTVs, 1);
	VEC_ALLOC(m_vvpRTVs[0], max(uMips, 1));

	auto uMip = 0ui8;
	for (auto &pRTV : m_vvpRTVs[0])
	{
		// Setup the description of the render target view.
		const auto rtvDesc = CD3D11_RENDER_TARGET_VIEW_DESC(pTexture, uArraySize > 1 ?
			(uSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY) :
			(uSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D),
			DXGI_FORMAT_UNKNOWN, uMip++);

		// Create the render target view.
		ThrowIfFailed(m_pDXDevice->CreateRenderTargetView(pTexture, &rtvDesc, &pRTV));
	}
}

void RenderTarget::Populate(const CPDXShaderResourceView &pSRVSrc, const spShader &pShader,
	const uint8_t uSRVSlot, const uint8_t uSlice, const uint8_t uMip)
{
	auto pDXContext = CPDXContext();
	m_pDXDevice->GetImmediateContext(&pDXContext);

	// Change the render target and clear the frame-buffer
	auto pRTVBack = CPDXRenderTargetView();
	auto pDSVBack = CPDXDepthStencilView();
	pDXContext->OMGetRenderTargets(1, &pRTVBack, &pDSVBack);
	pDXContext->OMSetRenderTargets(1, m_vvpRTVs[uSlice][uMip].GetAddressOf(), nullptr);
	pDXContext->ClearRenderTargetView(m_vvpRTVs[uSlice][uMip].Get(), Colors::Transparent);

	// Change the viewport
	auto uNumViewports = 1u;
	auto VpBack = D3D11_VIEWPORT();
	const auto VpDepth = CD3D11_VIEWPORT(m_pTexture.Get(), m_vvpRTVs[uSlice][uMip].Get());
	pDXContext->RSGetViewports(&uNumViewports, &VpBack);
	pDXContext->RSSetViewports(uNumViewports, &VpDepth);

	pDXContext->PSSetShaderResources(uSRVSlot, 1, pSRVSrc.GetAddressOf());
	pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pDXContext->VSSetShader(pShader->GetVertexShader(g_uVSScreenQuad).Get(), nullptr, 0);
	pDXContext->GSSetShader(nullptr, nullptr, 0);
	pDXContext->PSSetShader(pShader->GetPixelShader(g_uPSResample).Get(), nullptr, 0);
	//pDXContext->PSSetSamplers(uSmpSlot, 1, pState->AnisotropicWrap().GetAddressOf());

	pDXContext->Draw(3, 0);

	const auto pNullSRV = LPDXShaderResourceView(nullptr);
	pDXContext->PSSetShaderResources(uSRVSlot, 1, &pNullSRV);
	pDXContext->VSSetShader(nullptr, nullptr, 0);
	pDXContext->PSSetShader(nullptr, nullptr, 0);

	pDXContext->RSSetViewports(uNumViewports, &VpBack);
	pDXContext->OMSetRenderTargets(1, pRTVBack.GetAddressOf(), pDSVBack.Get());
}

const CPDXRenderTargetView &RenderTarget::GetRTV(const uint8_t uSlice, const uint8_t uMip) const
{
	assert(m_vvpRTVs.size() > uSlice);
	assert(m_vvpRTVs[uSlice].size() > uMip);
	return m_vvpRTVs[uSlice][uMip];
}

const uint8_t RenderTarget::GetArraySize() const
{
	return static_cast<uint8_t>(m_vvpRTVs.size());
}

const uint8_t RenderTarget::GetNumMips(const uint8_t uSlice) const
{
	return static_cast<uint8_t>(m_vvpRTVs[uSlice].size());
}

void RenderTarget::create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
	const DXGI_FORMAT eFormat, const uint8_t uSamples, const uint8_t uMips, const bool bUAV)
{
	// Setup the render target texture description.
	const auto textureDesc = CD3D11_TEXTURE2D_DESC(eFormat, uWidth, uHeight, uArraySize, uMips,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0),
		D3D11_USAGE_DEFAULT, 0, uSamples, 0, uMips == 1 ? 0 : D3D11_RESOURCE_MISC_GENERATE_MIPS);

	// Create the render target texture.
	ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, nullptr, &m_pTexture));

	// Create SRV
	CreateSRV(uArraySize, uSamples);

	// Create UAV
	if (bUAV) CreateUAV(uArraySize, uMips);
}

//--------------------------------------------------------------------------------------
// Depth-stencil
//--------------------------------------------------------------------------------------

DepthStencil::DepthStencil(const CPDXDevice &pDXDevice) :
	Texture2D(pDXDevice),
	m_vpDSVs(0),
	m_vpDSVROs(0)
{
}

DepthStencil::~DepthStencil()
{
}

void DepthStencil::Create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
	const bool bSRV, DXGI_FORMAT eFormat, const uint8_t uSamples, const uint8_t uMips)
{
	// Map formats
	auto fmtTexture = DXGI_FORMAT_R24G8_TYPELESS;
	auto fmtSRV = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	if (eFormat = DXGI_FORMAT_D32_FLOAT)
	{
		fmtTexture = DXGI_FORMAT_R32_TYPELESS;
		fmtSRV = DXGI_FORMAT_R32_FLOAT;
	}
	else if (eFormat = DXGI_FORMAT_D16_UNORM)
	{
		fmtTexture = DXGI_FORMAT_R16_TYPELESS;
		fmtSRV = DXGI_FORMAT_R16_UNORM;
	}

	// Setup the render depth stencil description.
	{
		const auto textureDesc = CD3D11_TEXTURE2D_DESC(
			DXGI_FORMAT_R24G8_TYPELESS, uWidth, uHeight, uArraySize, uMips, bSRV ?
			(D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE) :
			D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, uSamples);

		// Create the depth stencil texture.
		ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, nullptr, &m_pTexture));
	}

	const auto pTexture = m_pTexture.Get();
	if (bSRV)
	{
		// Setup the description of the shader resource view.
		const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(uArraySize > 1 ?
			(uSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY) :
			(uSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D),
			DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

		// Create the shader resource view.
		ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
	}

	const auto uNumMips = max(uMips, 1);
	VEC_ALLOC(m_vpDSVs, uNumMips);
	VEC_ALLOC(m_vpDSVROs, uNumMips);

	for (auto i = 0ui8; i < uNumMips; ++i)
	{
		// Setup the description of the depth stencil view.
		auto dsvDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(uArraySize > 1 ?
			(uSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY) :
			(uSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D),
			DXGI_FORMAT_D24_UNORM_S8_UINT, i);

		// Create the depth stencil view.
		ThrowIfFailed(m_pDXDevice->CreateDepthStencilView(pTexture, &dsvDesc, &m_vpDSVs[i]));

		if (bSRV)
		{
			dsvDesc.Flags = eFormat == DXGI_FORMAT_D24_UNORM_S8_UINT ?
				D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL :
				D3D11_DSV_READ_ONLY_DEPTH;

			// Create the depth stencil view.
			ThrowIfFailed(m_pDXDevice->CreateDepthStencilView(pTexture, &dsvDesc, &m_vpDSVROs[i]));
		}
		else m_vpDSVROs[i] = m_vpDSVs[i];
	}
}

void DepthStencil::Create(const uint32_t uWidth, const uint32_t uHeight, const bool bSRV,
	DXGI_FORMAT eFormat, const uint8_t uSamples, const uint8_t uMips)
{
	Create(uWidth, uHeight, 1, bSRV, eFormat, uSamples, uMips);
}

const CPDXDepthStencilView &DepthStencil::GetDSV(const uint8_t uMip) const
{
	assert(m_vpDSVs.size() > uMip);
	return m_vpDSVs[uMip];
}

const CPDXDepthStencilView &DepthStencil::GetDSVRO(const uint8_t uMip) const
{
	assert(m_vpDSVROs.size() > uMip);
	return m_vpDSVROs[uMip];
}

const uint8_t DepthStencil::GetNumMips() const
{
	return static_cast<uint8_t>(m_vpDSVs.size());
}

//--------------------------------------------------------------------------------------
// 3D Texture
//--------------------------------------------------------------------------------------

Texture3D::Texture3D(const CPDXDevice &pDXDevice) :
	Buffer(pDXDevice),
	m_pTexture(nullptr),
	m_vpUAVs(0)
{
}

Texture3D::~Texture3D()
{
}

void Texture3D::Create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uDepth,
	const DXGI_FORMAT eFormat, const bool bUAV, const bool bSRV, const bool bDyn,
	const uint8_t uMips, const lpcvoid pInitialData, const uint8_t uStride)
{
	// Setup the texture description.
	const auto textureDesc = CD3D11_TEXTURE3D_DESC(eFormat, uWidth, uHeight, uDepth,
		uMips, D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA
		{ pInitialData, uStride * uWidth, uStride * uWidth * uHeight };
		ThrowIfFailed(m_pDXDevice->CreateTexture3D(&textureDesc, &bufferInitData, &m_pTexture));
	}
	else ThrowIfFailed(m_pDXDevice->CreateTexture3D(&textureDesc, nullptr, &m_pTexture));
	
	// Create SRV
	const auto pTexture = m_pTexture.Get();
	if (bSRV)
	{
		// Setup the description of the shader resource view.
		const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pTexture);

		// Create the shader resource view.
		ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
	}

	// Create UAV
	if (bUAV)
	{
		auto uMip = 0ui8;
		VEC_ALLOC(m_vpUAVs, max(uMips, 1));
		for (auto &pUAV : m_vpUAVs)
		{
			// Setup the description of the unordered access view.
			const auto uWSize = uDepth >> uMip;
			const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pTexture, DXGI_FORMAT_UNKNOWN, uMip++, 0, uWSize);

			// Create the unordered access view.
			ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pTexture, &uavDesc, &pUAV));
		}
	}
}

const CPDXTexture3D &::Texture3D::GetBuffer() const
{
	return m_pTexture;
}

const CPDXUnorderedAccessView &::Texture3D::GetUAV(const uint8_t i) const
{
	return m_vpUAVs[i];
}

//--------------------------------------------------------------------------------------
// Raw buffer
//--------------------------------------------------------------------------------------

RawBuffer::RawBuffer(const CPDXDevice &pDXDevice) :
	Buffer(pDXDevice),
	m_pBuffer(nullptr),
	m_pUAV(nullptr)
{
}

RawBuffer::~RawBuffer()
{
}

void RawBuffer::Create(const uint32_t uByteWidth, const bool bVB, const bool bSO,
	const bool bUAV, const bool bSRV, const bool bDyn,
	const lpcvoid pInitialData, const uint8_t uUAVFlags)
{
	// Create RB
	auto uBindFlag = bVB ? D3D11_BIND_VERTEX_BUFFER : 0;
	uBindFlag |= bSO ? D3D11_BIND_STREAM_OUTPUT : 0;
	uBindFlag |= bSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	uBindFlag |= bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;

	auto bufferDesc = CD3D11_BUFFER_DESC(uByteWidth, uBindFlag,
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
		bDyn ? D3D11_CPU_ACCESS_WRITE : 0);
	bufferDesc.MiscFlags = bSRV || bUAV ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : bufferDesc.MiscFlags;

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA{ pInitialData, 0, 0 };
		ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, &bufferInitData, &m_pBuffer));
	}
	else ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, nullptr, &m_pBuffer));

	// Create SRV
	if (bSRV) CreateSRV(uByteWidth);

	// Create UAV
	if (bUAV)
	{
		const auto pBuffer = m_pBuffer.Get();

		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pBuffer,
			DXGI_FORMAT_R32_TYPELESS, 0, uByteWidth / 4, uUAVFlags);

		// Create the unordered access view.
		ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pBuffer, &uavDesc, &m_pUAV));
	}
}

void RawBuffer::CreateSRV(const uint32_t uByteWidth)
{
	// Create SRV
	const auto pBuffer = m_pBuffer.Get();

	// Setup the description of the shader resource view.
	const auto desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pBuffer, DXGI_FORMAT_R32_TYPELESS,
		0, uByteWidth / 4, D3D11_BUFFEREX_SRV_FLAG_RAW);

	// Create the shader resource view.
	ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pBuffer, &desc, &m_pSRV));
}

const CPDXBuffer &RawBuffer::GetBuffer() const
{
	return m_pBuffer;
}

const CPDXUnorderedAccessView &RawBuffer::GetUAV() const
{
	return m_pUAV;
}

//--------------------------------------------------------------------------------------
// Typed buffer
//--------------------------------------------------------------------------------------

TypedBuffer::TypedBuffer(const CPDXDevice &pDXDevice) :
	RawBuffer(pDXDevice)
{
}

TypedBuffer::~TypedBuffer()
{
}

void TypedBuffer::Create(const uint32_t uNumElements, const uint32_t uStride,
	const DXGI_FORMAT eFormat, const bool bUAV, const bool bSRV, const bool bDyn,
	const lpcvoid pInitialData, const uint8_t uUAVFlags)
{
	// Create SB
	const auto bufferDesc = CD3D11_BUFFER_DESC(uNumElements * uStride,
		D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA{ pInitialData };
		ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, &bufferInitData, &m_pBuffer));
	}
	else ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, nullptr, &m_pBuffer));

	// Create SRV
	if (bSRV) CreateSRV(uNumElements, eFormat);

	// Create UAV
	if (bUAV)
	{
		const auto pBuffer = m_pBuffer.Get();

		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pBuffer,
			eFormat, 0, uNumElements, uUAVFlags);

		// Create the unordered access view.
		ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pBuffer, &uavDesc, &m_pUAV));
	}
}

void TypedBuffer::CreateSRV(const uint32_t uNumElements, const DXGI_FORMAT eFormat)
{
	// Create SRV
	const auto pBuffer = m_pBuffer.Get();

	// Setup the description of the shader resource view.
	const auto desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pBuffer, eFormat, 0, uNumElements);

	// Create the shader resource view.
	ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pBuffer, &desc, &m_pSRV));
}

//--------------------------------------------------------------------------------------
// Structured buffer
//--------------------------------------------------------------------------------------

StructuredBuffer::StructuredBuffer(const CPDXDevice &pDXDevice) :
	RawBuffer(pDXDevice)
{
}

StructuredBuffer::~StructuredBuffer()
{
}

void StructuredBuffer::Create(const uint32_t uNumElements, const uint32_t uStride, const bool bUAV,
	const bool bDyn, const bool bSRV, const lpcvoid pInitialData, const uint8_t uUAVFlags)
{
	// Create SB
	const auto bufferDesc = CD3D11_BUFFER_DESC(uNumElements * uStride,
		D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, uStride);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA{ pInitialData };
		ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, &bufferInitData, &m_pBuffer));
	}
	else ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, nullptr, &m_pBuffer));

	// Create SRV
	if (bSRV) CreateSRV(uNumElements);

	// Create UAV
	if (bUAV)
	{
		const auto pBuffer = m_pBuffer.Get();

		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pBuffer,
			DXGI_FORMAT_UNKNOWN, 0, uNumElements, uUAVFlags);

		// Create the unordered access view.
		ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pBuffer, &uavDesc, &m_pUAV));
	}
}

void StructuredBuffer::CreateSRV(const uint32_t uNumElements)
{
	// Create SRV
	const auto pBuffer = m_pBuffer.Get();

	// Setup the description of the shader resource view.
	const auto desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pBuffer, DXGI_FORMAT_UNKNOWN, 0, uNumElements);

	// Create the shader resource view.
	ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pBuffer, &desc, &m_pSRV));
}
