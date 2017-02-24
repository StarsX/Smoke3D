//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "XSDXBuffer.h"

using namespace DirectX;
using namespace DX;
using namespace XSDX;

Buffer::Buffer(const CPDXDevice &pDXDevice) :
	m_pDXDevice(pDXDevice)
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
	desc.BindFlags = 0u;
	desc.MiscFlags = 0u;

	ThrowIfFailed(pDXDevice->CreateBuffer(&desc, nullptr, &pDstBuffer));
}

//--------------------------------------------------------------------------------------
// 2D Texture
//--------------------------------------------------------------------------------------

Texture2D::Texture2D(const CPDXDevice &pDXDevice) :
	Buffer(pDXDevice)
{
}

void Texture2D::Create(const bool bUAV, const bool bDyn, const uint32_t uWidth, const uint32_t uHeight,
	const DXGI_FORMAT format, const uint8_t uMips, const lpcvoid pInitialData, const uint8_t uStride)
{
	// Setup the texture description.
	const auto textureDesc = CD3D11_TEXTURE2D_DESC(format, uWidth, uHeight, 1u,
		uMips, D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0u),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0u);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA
		{ pInitialData, uStride * uWidth, 0u };
		DX::ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, &bufferInitData, &m_pTexture));
	}
	else DX::ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, nullptr, &m_pTexture));

	// Create SRV
	const auto pTexture = m_pTexture.Get();
	{
		// Setup the description of the shader resource view.
		const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pTexture, D3D11_SRV_DIMENSION_TEXTURE2D);

		// Create the shader resource view.
		ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
	}

	// Create UAV
	if (bUAV)
	{
		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pTexture, D3D11_UAV_DIMENSION_TEXTURE2D);

		// Create the unordered access view.
		VEC_ALLOC(m_vpUAVs, uMips);
		for (auto &pUAV : m_vpUAVs)
			DX::ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pTexture, &uavDesc, &pUAV));
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
	Texture2D(pDXDevice)
{
}

void RenderTarget::Create(const uint32_t uWidth, const uint32_t uHeight,
	const DXGI_FORMAT format, const uint8_t uSamples, const uint8_t uMips)
{
	{
		// Setup the render target texture description.
		const auto textureDesc = CD3D11_TEXTURE2D_DESC(format, uWidth, uHeight, 1u, uMips,
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT,
			0u, uSamples, 0u, uMips == 1u ? 0u : D3D11_RESOURCE_MISC_GENERATE_MIPS);

		// Create the render target texture.
		ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, nullptr, &m_pTexture));
	}

	const auto pTexture = m_pTexture.Get();
	{
		// Setup the description of the shader resource view.
		const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pTexture, uSamples > 1ui8 ?
			D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D);

		// Create the shader resource view.
		ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
	}

	{
		// Setup the description of the render target view.
		const auto rtvDesc = CD3D11_RENDER_TARGET_VIEW_DESC(pTexture, uSamples > 1ui8 ?
			D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D);

		// Create the render target view.
		ThrowIfFailed(m_pDXDevice->CreateRenderTargetView(pTexture, &rtvDesc, &m_pRTV));
	}
}

void RenderTarget::Populate(const CPDXShaderResourceView &pSRVSrc, const spShader &pShader, const uint8_t uSRSlot)
{
	auto pDXContext = CPDXContext();
	m_pDXDevice->GetImmediateContext(&pDXContext);

	// Change the render target and clear the frame-buffer
	auto pRTVBack = CPDXRenderTargetView();
	auto pDSVBack = CPDXDepthStencilView();
	pDXContext->OMGetRenderTargets(1u, &pRTVBack, &pDSVBack);
	pDXContext->OMSetRenderTargets(1u, m_pRTV.GetAddressOf(), nullptr);
	pDXContext->ClearRenderTargetView(m_pRTV.Get(), Colors::Transparent);

	// Change the viewport
	auto uNumVp = 1u;
	auto VpBack = D3D11_VIEWPORT();
	const auto VpDepth = CD3D11_VIEWPORT(m_pTexture.Get(), m_pRTV.Get());
	pDXContext->RSGetViewports(&uNumVp, &VpBack);
	pDXContext->RSSetViewports(uNumVp, &VpDepth);

	pDXContext->PSSetShaderResources(uSRSlot, 1u, pSRVSrc.GetAddressOf());
	pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pDXContext->VSSetShader(pShader->GetVertexShader(g_uVSScreenQuad).Get(), nullptr, 0u);
	pDXContext->GSSetShader(nullptr, nullptr, 0u);
	pDXContext->PSSetShader(pShader->GetPixelShader(g_uPSResample).Get(), nullptr, 0u);
	//pDXContext->PSSetSamplers(uSmpSlot, 1u, pState->AnisotropicWrap().GetAddressOf());

	pDXContext->Draw(3u, 0u);

	const auto pNullSRV = LPDXShaderResourceView(nullptr);
	pDXContext->PSSetShaderResources(uSRSlot, 1u, &pNullSRV);
	pDXContext->VSSetShader(nullptr, nullptr, 0u);
	pDXContext->PSSetShader(nullptr, nullptr, 0u);

	pDXContext->RSSetViewports(uNumVp, &VpBack);
	pDXContext->OMSetRenderTargets(1u, pRTVBack.GetAddressOf(), pDSVBack.Get());
}

const CPDXRenderTargetView &RenderTarget::GetRTV() const
{
	return m_pRTV;
}

//--------------------------------------------------------------------------------------
// Depth-stencil
//--------------------------------------------------------------------------------------

DepthStencil::DepthStencil(const CPDXDevice &pDXDevice) :
	Texture2D(pDXDevice)
{
}

void DepthStencil::Create(const uint32_t uWidth, const uint32_t uHeight,
	const bool bRead, DXGI_FORMAT format, const uint8_t uSamples)
{
	// Map formats
	auto fmtTexture = DXGI_FORMAT_R24G8_TYPELESS;
	auto fmtSRV = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	if (format = DXGI_FORMAT_D32_FLOAT)
	{
		fmtTexture = DXGI_FORMAT_R32_TYPELESS;
		fmtSRV = DXGI_FORMAT_R32_FLOAT;
	}
	else if (format = DXGI_FORMAT_D16_UNORM)
	{
		fmtTexture = DXGI_FORMAT_R16_TYPELESS;
		fmtSRV = DXGI_FORMAT_R16_UNORM;
	}

	// Setup the render depth stencil description.
	{
		const auto textureDesc = CD3D11_TEXTURE2D_DESC(
			DXGI_FORMAT_R24G8_TYPELESS, uWidth, uHeight, 1u, 1u, bRead ?
			(D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE) :
			D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0u, uSamples
			);

		// Create the depth stencil texture.
		ThrowIfFailed(m_pDXDevice->CreateTexture2D(&textureDesc, nullptr, &m_pTexture));
	}

	const auto pTexture = m_pTexture.Get();
	if (bRead)
	{
		// Setup the description of the shader resource view.
		const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(
			uSamples > 1ui8 ? D3D11_SRV_DIMENSION_TEXTURE2DMS :
			D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

		// Create the shader resource view.
		ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
	}
	else m_pSRV = nullptr;

	{
		// Setup the description of the depth stencil view.
		const auto dsvDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(
			uSamples > 1ui8 ? D3D11_DSV_DIMENSION_TEXTURE2DMS :
			D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT
			);

		// Create the depth stencil view.
		ThrowIfFailed(m_pDXDevice->CreateDepthStencilView(pTexture, &dsvDesc, &m_pDSV));
	}

	if (bRead)
	{
		auto dsvDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(
			uSamples > 1ui8 ? D3D11_DSV_DIMENSION_TEXTURE2DMS :
			D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT
			);
		dsvDesc.Flags = format == DXGI_FORMAT_D24_UNORM_S8_UINT ?
			D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL :
			D3D11_DSV_READ_ONLY_DEPTH;

		// Create the depth stencil view.
		ThrowIfFailed(m_pDXDevice->CreateDepthStencilView(pTexture, &dsvDesc, &m_pDSVRO));
	}
	else m_pDSVRO = m_pDSV;
}

const CPDXDepthStencilView &DepthStencil::GetDSV() const
{
	return m_pDSV;
}

const CPDXDepthStencilView &DepthStencil::GetDSVRO() const
{
	return m_pDSVRO;
}

//--------------------------------------------------------------------------------------
// Raw buffer
//--------------------------------------------------------------------------------------

RawBuffer::RawBuffer(const CPDXDevice & pDXDevice) :
	Buffer(pDXDevice)
{
}

void RawBuffer::Create(const bool bVB, const bool bSO, const bool bSRV,
	const bool bUAV, const bool bDyn, const uint32_t uByteWidth,
	const lpcvoid pInitialData)
{
	// Create RB
	auto uBindFlag = bVB ? D3D11_BIND_VERTEX_BUFFER : 0u;
	uBindFlag |= bSO ? D3D11_BIND_STREAM_OUTPUT : 0u;
	uBindFlag |= bSRV ? D3D11_BIND_SHADER_RESOURCE : 0u;
	uBindFlag |= bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0u;

	auto bufferDesc = CD3D11_BUFFER_DESC(uByteWidth, uBindFlag,
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
		bDyn ? D3D11_CPU_ACCESS_WRITE : 0u);
	bufferDesc.MiscFlags = bSRV || bUAV ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : bufferDesc.MiscFlags;

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA{ pInitialData, 0u, 0u };
		DX::ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, &bufferInitData, &m_pBuffer));
	}
	else DX::ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, nullptr, &m_pBuffer));

	// Create SRV
	if (bSRV) CreateSRV(uByteWidth);
	else m_pSRV = nullptr;

	// Create UAV
	if (bUAV)
	{
		const auto pBuffer = m_pBuffer.Get();

		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pBuffer, DXGI_FORMAT_R32_TYPELESS,
			0u, uByteWidth / 4u, D3D11_BUFFER_UAV_FLAG_RAW);

		// Create the unordered access view.
		DX::ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pBuffer, &uavDesc, &m_pUAV));
	}
}

void RawBuffer::CreateSRV(const uint32_t uByteWidth)
{
	// Create SRV
	const auto pBuffer = m_pBuffer.Get();

	// Setup the description of the shader resource view.
	const auto desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pBuffer, DXGI_FORMAT_R32_TYPELESS,
		0u, uByteWidth / 4u, D3D11_BUFFEREX_SRV_FLAG_RAW);

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
// Structured buffer
//--------------------------------------------------------------------------------------

StructuredBuffer::StructuredBuffer(const CPDXDevice &pDXDevice) :
	RawBuffer(pDXDevice)
{
}

void StructuredBuffer::Create(const bool bUAV, const bool bDyn,
	const uint32_t uNumElement, const uint32_t uStride, const lpcvoid pInitialData)
{
	// Create SB
	const auto bufferDesc = CD3D11_BUFFER_DESC(uNumElement * uStride,
		D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0u),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0u,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, uStride
		);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA{ pInitialData, 0u, 0u };
		DX::ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, &bufferInitData, &m_pBuffer));
	}
	else DX::ThrowIfFailed(m_pDXDevice->CreateBuffer(&bufferDesc, nullptr, &m_pBuffer));

	// Create SRV
	CreateSRV(uNumElement);

	// Create UAV
	if (bUAV)
	{
		const auto pBuffer = m_pBuffer.Get();

		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pBuffer, DXGI_FORMAT_UNKNOWN, 0u, uNumElement);

		// Create the unordered access view.
		DX::ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pBuffer, &uavDesc, &m_pUAV));
	}
}

void StructuredBuffer::CreateSRV(const uint32_t uNumElement)
{
	// Create SRV
	const auto pBuffer = m_pBuffer.Get();

	// Setup the description of the shader resource view.
	const auto desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pBuffer, DXGI_FORMAT_UNKNOWN, 0u, uNumElement);

	// Create the shader resource view.
	ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pBuffer, &desc, &m_pSRV));
}

//--------------------------------------------------------------------------------------
// 2D Texture
//--------------------------------------------------------------------------------------

Texture3D::Texture3D(const CPDXDevice &pDXDevice) :
	Buffer(pDXDevice)
{
}

void Texture3D::Create(const bool bUAV, const bool bDyn,
	const uint32_t uWidth, const uint32_t uHeight, const uint32_t uDepth,
	const DXGI_FORMAT format, const uint8_t uMips,
	const lpcvoid pInitialData, const uint8_t uStride)
{
	// Setup the texture description.
	const auto textureDesc = CD3D11_TEXTURE3D_DESC(format, uWidth, uHeight, uDepth,
		uMips, D3D11_BIND_SHADER_RESOURCE | (bUAV ? D3D11_BIND_UNORDERED_ACCESS : 0u),
		bDyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, bDyn ? D3D11_CPU_ACCESS_WRITE : 0u);

	if (pInitialData)
	{
		const auto bufferInitData = D3D11_SUBRESOURCE_DATA
		{ pInitialData, uStride * uWidth, uStride * uWidth * uHeight };
		DX::ThrowIfFailed(m_pDXDevice->CreateTexture3D(&textureDesc, &bufferInitData, &m_pTexture));
	}
	else DX::ThrowIfFailed(m_pDXDevice->CreateTexture3D(&textureDesc, nullptr, &m_pTexture));
	
	// Create SRV
	const auto pTexture = m_pTexture.Get();
	{
		// Setup the description of the shader resource view.
		const auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pTexture);

		// Create the shader resource view.
		ThrowIfFailed(m_pDXDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pSRV));
	}

	// Create UAV
	if (bUAV)
	{
		// Setup the description of the unordered access view.
		const auto uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(pTexture);

		// Create the unordered access view.
		VEC_ALLOC(m_vpUAVs, uMips);
		for (auto &pUAV : m_vpUAVs)
			DX::ThrowIfFailed(m_pDXDevice->CreateUnorderedAccessView(pTexture, &uavDesc, &pUAV));
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
