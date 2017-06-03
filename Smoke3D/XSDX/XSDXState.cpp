//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "XSDXState.h"

using namespace DX;
using namespace XSDX;

State::State(const CPDXDevice &pDXDevice) :
	m_pDXDevice(pDXDevice)
{
}

void State::CreateBlendState(CPDXBlendState &pState, const bool bAlphaToCov,
	const D3D11_BLEND eSrc, const D3D11_BLEND eDst, const D3D11_BLEND_OP eOp)
{
	auto desc = D3D11_BLEND_DESC{};

	desc.AlphaToCoverageEnable = bAlphaToCov;
	desc.RenderTarget[0].BlendEnable = !bAlphaToCov && (eSrc != D3D11_BLEND_ONE || eDst != D3D11_BLEND_ZERO);

	desc.RenderTarget[0].SrcBlend = desc.RenderTarget[0].SrcBlendAlpha = eSrc;
	desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = eDst;
	desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = eOp;

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ThrowIfFailed(m_pDXDevice->CreateBlendState(&desc, &pState));
}

void State::CreateDepthStencilState(CPDXDepthStencilState &pState, const bool bEnable,
	const bool bWriteEnable, const D3D11_COMPARISON_FUNC eDepthFunc)
{
	const auto desc = CD3D11_DEPTH_STENCIL_DESC(
		bEnable, bWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO, eDepthFunc,
		false, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS);

	ThrowIfFailed(m_pDXDevice->CreateDepthStencilState(&desc, &pState));
}

void State::CreateRasterizerState(CPDXRasterizerState &pState, const D3D11_CULL_MODE eCull,
	const D3D11_FILL_MODE eFill, const bool bAALine)
{
	auto desc = D3D11_RASTERIZER_DESC{};

	desc.CullMode = eCull;
	desc.FillMode = eFill;
	desc.DepthClipEnable = true;
	desc.MultisampleEnable = true;
	desc.AntialiasedLineEnable = bAALine;

	ThrowIfFailed(m_pDXDevice->CreateRasterizerState(&desc, &pState));
}

void State::CreateSamplerState(CPDXSamplerState &pState, const D3D11_FILTER eFilter,
	const D3D11_TEXTURE_ADDRESS_MODE eAddressMode, const D3D11_COMPARISON_FUNC eCmpFunc,
	const float fBorder)
{
	auto desc = D3D11_SAMPLER_DESC{};

	desc.Filter = eFilter;

	desc.AddressU = eAddressMode;
	desc.AddressV = eAddressMode;
	desc.AddressW = eAddressMode;

	desc.MaxAnisotropy = (m_pDXDevice->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1) ? 16u : 2u;

	desc.MaxLOD = D3D11_FLOAT32_MAX;
	desc.ComparisonFunc = eCmpFunc;
	desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = fBorder;

	ThrowIfFailed(m_pDXDevice->CreateSamplerState(&desc, &pState));
}

//--------------------------------------------------------------------------------------
// Blend states
//--------------------------------------------------------------------------------------

const CPDXBlendState &State::Opaque()
{
	if (!m_pOpaque)
		CreateBlendState(m_pOpaque, false, D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
	
	return m_pOpaque;
}

const CPDXBlendState &State::AlphaBlend()
{
	if (!m_pAlphaBlend)
		CreateBlendState(m_pAlphaBlend, false, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA);
	
	return m_pAlphaBlend;
}

const CPDXBlendState &State::Additive()
{
	if (!m_pAdditive)
		CreateBlendState(m_pAdditive, false, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);
	
	return m_pAdditive;
}

const CPDXBlendState &State::NonPremultiplied()
{
	if (!m_pNonPremultiplied)
		CreateBlendState(m_pNonPremultiplied, false, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	
	return m_pNonPremultiplied;
}

const CPDXBlendState &State::AlphaToCoverage()
{
	if (!m_pAlphaToCoverage)
		CreateBlendState(m_pAlphaToCoverage, true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	
	return m_pAlphaToCoverage;
}

const CPDXBlendState &State::Accumulative()
{
	if (!m_pAccumulative)
		CreateBlendState(m_pAccumulative, false, D3D11_BLEND_ONE, D3D11_BLEND_ONE);

	return m_pAccumulative;
}

const CPDXBlendState &State::AutoAlphaBlend()
{
	if (!m_pAutoAlphaBlend)
	{
		const auto desc = D3D11_BLEND_DESC {
			false,								// AlphaToCoverageEnable
			false,								// IndependentBlendEnable
			D3D11_RENDER_TARGET_BLEND_DESC {
				true,							// BlendEnable
				D3D11_BLEND_SRC_ALPHA,			// SrcBlend
				D3D11_BLEND_INV_SRC_ALPHA,		// DestBlend
				D3D11_BLEND_OP_ADD,				// BlendOp
				D3D11_BLEND_ONE,				// SrcBlendAlpha
				D3D11_BLEND_INV_SRC_ALPHA,		// DestBlendAlpha
				D3D11_BLEND_OP_ADD,				// BlendOpAlpha
				D3D11_COLOR_WRITE_ENABLE_ALL	// RenderTargetWriteMask
			}
		};

		ThrowIfFailed(m_pDXDevice->CreateBlendState(&desc, &m_pAutoAlphaBlend));
	}

	return m_pAutoAlphaBlend;
}

const CPDXBlendState &State::Multiplied()
{
	if (!m_pMultiplied)
	{
		const auto desc = D3D11_BLEND_DESC
		{
			false,								// AlphaToCoverageEnable
			false,								// IndependentBlendEnable
			D3D11_RENDER_TARGET_BLEND_DESC
			{
				true,							// BlendEnable
				D3D11_BLEND_DEST_COLOR,			// SrcBlend
				D3D11_BLEND_ZERO,				// DestBlend
				D3D11_BLEND_OP_ADD,				// BlendOp
				D3D11_BLEND_DEST_ALPHA,			// SrcBlendAlpha
				D3D11_BLEND_ZERO,				// DestBlendAlpha
				D3D11_BLEND_OP_ADD,				// BlendOpAlpha
				D3D11_COLOR_WRITE_ENABLE_ALL	// RenderTargetWriteMask
			}
		};

		ThrowIfFailed(m_pDXDevice->CreateBlendState(&desc, &m_pMultiplied));
	}

	return m_pMultiplied;
}

const CPDXBlendState &State::WeightBlend()
{
	if (!m_pWeightBlend)
	{
		auto desc = D3D11_BLEND_DESC {
			false,								// AlphaToCoverageEnable
			true,								// IndependentBlendEnable
			// Accumulation
			D3D11_RENDER_TARGET_BLEND_DESC
			{
				true,							// BlendEnable
				D3D11_BLEND_SRC_ALPHA,			// SrcBlend
				D3D11_BLEND_ONE,				// DestBlend
				D3D11_BLEND_OP_ADD,				// BlendOp
				D3D11_BLEND_ONE,				// SrcBlendAlpha
				D3D11_BLEND_ONE,				// DestBlendAlpha
				D3D11_BLEND_OP_ADD,				// BlendOpAlpha
				D3D11_COLOR_WRITE_ENABLE_ALL	// RenderTargetWriteMask
			},
			// Production
			D3D11_RENDER_TARGET_BLEND_DESC
			{
				true,							// BlendEnable
				D3D11_BLEND_DEST_COLOR,			// SrcBlend
				D3D11_BLEND_ZERO,				// DestBlend
				D3D11_BLEND_OP_ADD,				// BlendOp
				D3D11_BLEND_ZERO,				// SrcBlendAlpha
				D3D11_BLEND_ZERO,				// DestBlendAlpha
				D3D11_BLEND_OP_ADD				// BlendOpAlpha
			}
		};
		// RenderTargetWriteMask
		desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED;

		ThrowIfFailed(m_pDXDevice->CreateBlendState(&desc, &m_pWeightBlend));
	}

	return m_pWeightBlend;
}

//--------------------------------------------------------------------------------------
// Depth stencil states
//--------------------------------------------------------------------------------------

const CPDXDepthStencilState &State::DepthNone()
{
	if (!m_pDepthNone)
		CreateDepthStencilState(m_pDepthNone, false, false);
	
	return m_pDepthNone;
}

const CPDXDepthStencilState &State::DepthDefault()
{
	if (!m_pDepthDefault)
		CreateDepthStencilState(m_pDepthDefault, true, true);
	
	return m_pDepthDefault;
}

const CPDXDepthStencilState &State::DepthRead()
{
	if (!m_pDepthRead)
		CreateDepthStencilState(m_pDepthRead, true, false);
	
	return m_pDepthRead;
}

const CPDXDepthStencilState &State::DepthReadLess()
{
	if (!m_pDepthReadLess)
		CreateDepthStencilState(m_pDepthReadLess, true, false, D3D11_COMPARISON_LESS);

	return m_pDepthReadLess;
}

const CPDXDepthStencilState &State::DepthReadEqual()
{
	if (!m_pDepthReadEqual)
		CreateDepthStencilState(m_pDepthReadEqual, true, false, D3D11_COMPARISON_EQUAL);

	return m_pDepthReadEqual;
}

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

const CPDXRasterizerState &State::CullNone()
{
	if (!m_pCullNone)
		CreateRasterizerState(m_pCullNone, D3D11_CULL_NONE, D3D11_FILL_SOLID);
	
	return m_pCullNone;
}

const CPDXRasterizerState &State::CullClockwise()
{
	if (!m_pCullClockwise)
		CreateRasterizerState(m_pCullClockwise, D3D11_CULL_FRONT, D3D11_FILL_SOLID);

	return m_pCullClockwise;
}

const CPDXRasterizerState &State::CullCounterClockwise()
{
	if (!m_pCullCounterClockwise)
		CreateRasterizerState(m_pCullCounterClockwise, D3D11_CULL_BACK, D3D11_FILL_SOLID);

	return m_pCullCounterClockwise;
}

const CPDXRasterizerState &State::Wireframe()
{
	if (!m_pWireframe)
		CreateRasterizerState(m_pWireframe, D3D11_CULL_BACK, D3D11_FILL_WIREFRAME);

	return m_pWireframe;
}

//--------------------------------------------------------------------------------------
// Sampler states
//--------------------------------------------------------------------------------------

const CPDXSamplerState &State::PointWrap()
{
	if (!m_pPointWrap)
		CreateSamplerState(m_pPointWrap, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);

	return m_pPointWrap;
}

const CPDXSamplerState &State::PointClamp()
{
	if (!m_pPointClamp)
		CreateSamplerState(m_pPointClamp, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);

	return m_pPointClamp;
}

const CPDXSamplerState &State::PointBorder()
{
	if (!m_pPointBorder)
		CreateSamplerState(m_pPointBorder, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_BORDER);

	return m_pPointBorder;
}

const CPDXSamplerState &State::PointComparison()
{
	if (!m_pPointComparison)
		CreateSamplerState(m_pPointComparison, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_LESS_EQUAL, 1.0f);

	return m_pPointComparison;
}

const CPDXSamplerState &State::LinearWrap()
{
	if (!m_pLinearWrap)
		CreateSamplerState(m_pLinearWrap, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

	return m_pLinearWrap;
}

const CPDXSamplerState &State::LinearClamp()
{
	if (!m_pLinearClamp)
		CreateSamplerState(m_pLinearClamp, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);

	return m_pLinearClamp;
}

const CPDXSamplerState &State::LinearBorder()
{
	if (!m_pLinearBorder)
		CreateSamplerState(m_pLinearBorder, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER);

	return m_pLinearBorder;
}

const CPDXSamplerState &State::LinearComparison()
{
	if (!m_pLinearComparison)
		CreateSamplerState(m_pLinearComparison, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
			D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_LESS_EQUAL, 1.0f);

	return m_pLinearComparison;
}

const CPDXSamplerState &State::AnisotropicWrap()
{
	if (!m_pAnisotropicWrap)
		CreateSamplerState(m_pAnisotropicWrap, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);

	return m_pAnisotropicWrap;
}

const CPDXSamplerState &State::AnisotropicClamp()
{
	if (!m_pAnisotropicClamp)
		CreateSamplerState(m_pAnisotropicClamp, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_CLAMP);

	return m_pAnisotropicClamp;
}

const CPDXSamplerState &State::AnisotropicBorder()
{
	if (!m_pAnisotropicBorder)
		CreateSamplerState(m_pAnisotropicBorder, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_BORDER);

	return m_pAnisotropicBorder;
}

const CPDXSamplerState &State::AnisotropicComparison()
{
	if (!m_pAnisotropicComparison)
		CreateSamplerState(m_pAnisotropicComparison, D3D11_FILTER_COMPARISON_ANISOTROPIC,
			D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_LESS_EQUAL, 1.0f);

	return m_pAnisotropicComparison;
}
