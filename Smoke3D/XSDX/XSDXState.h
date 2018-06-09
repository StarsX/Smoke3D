//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXType.h"

namespace XSDX
{
	class State
	{
	public:
		State(const CPDXDevice &pDXDevice);
		virtual ~State(void);

		void CreateBlendState(
			CPDXBlendState &pState, const bool bAlphaToCov,
			const D3D11_BLEND eSrc, const D3D11_BLEND eDst,
			const D3D11_BLEND_OP eOp = D3D11_BLEND_OP_ADD);
		void CreateDepthStencilState(
			CPDXDepthStencilState &pState, const bool bEnable, const bool bWriteEnable,
			const D3D11_COMPARISON_FUNC eDepthFunc = D3D11_COMPARISON_LESS_EQUAL);
		void CreateRasterizerState(
			CPDXRasterizerState &pState, const D3D11_CULL_MODE eCull,
			const D3D11_FILL_MODE eFill, const bool bAALine = false);
		void CreateSamplerState(
			CPDXSamplerState &pState, const D3D11_FILTER eFilter,
			const D3D11_TEXTURE_ADDRESS_MODE eAddressMode,
			const D3D11_COMPARISON_FUNC eCmpFunc = D3D11_COMPARISON_NEVER,
			const float fBorder = 0.0f);

		const CPDXBlendState		&Opaque();
		const CPDXBlendState		&AlphaBlend();
		const CPDXBlendState		&Additive();
		const CPDXBlendState		&NonPremultiplied();
		const CPDXBlendState		&NonPremultiplied0();
		const CPDXBlendState		&AlphaToCoverage();
		const CPDXBlendState		&Accumulative();
		const CPDXBlendState		&AutoAlphaBlend();
		const CPDXBlendState		&BlendAlphaZero();
		const CPDXBlendState		&Multiplied();
		const CPDXBlendState		&WeightBlend();
		const CPDXBlendState		&SelectMin();
		const CPDXBlendState		&SelectMax();

		const CPDXDepthStencilState	&DepthNone();
		const CPDXDepthStencilState	&DepthDefault();
		const CPDXDepthStencilState	&DepthRead();
		const CPDXDepthStencilState	&DepthReadLess();
		const CPDXDepthStencilState	&DepthReadEqual();

		const CPDXRasterizerState	&CullNone();
		const CPDXRasterizerState	&CullClockwise();
		const CPDXRasterizerState	&CullCounterClockwise();
		const CPDXRasterizerState	&Wireframe();

		const CPDXSamplerState		&PointWrap();
		const CPDXSamplerState		&PointClamp();
		const CPDXSamplerState		&PointBorder();
		const CPDXSamplerState		&PointComparison();
		const CPDXSamplerState		&LinearWrap();
		const CPDXSamplerState		&LinearClamp();
		const CPDXSamplerState		&LinearBorder();
		const CPDXSamplerState		&LinearComparison();
		const CPDXSamplerState		&AnisotropicWrap();
		const CPDXSamplerState		&AnisotropicClamp();
		const CPDXSamplerState		&AnisotropicBorder();
		const CPDXSamplerState		&AnisotropicComparison();
	protected:
		CPDXBlendState				m_pOpaque;
		CPDXBlendState				m_pAlphaBlend;
		CPDXBlendState				m_pAdditive;
		CPDXBlendState				m_pNonPremultiplied;
		CPDXBlendState				m_pNonPremultiplied0;
		CPDXBlendState				m_pAlphaToCoverage;
		CPDXBlendState				m_pAccumulative;
		CPDXBlendState				m_pAutoAlphaBlend;
		CPDXBlendState				m_pBlendAlphaZero;
		CPDXBlendState				m_pMultiplied;
		CPDXBlendState				m_pWeightBlend;
		CPDXBlendState				m_pSelectMin;
		CPDXBlendState				m_pSelectMax;

		CPDXDepthStencilState		m_pDepthNone;
		CPDXDepthStencilState		m_pDepthDefault;
		CPDXDepthStencilState		m_pDepthRead;
		CPDXDepthStencilState		m_pDepthReadLess;
		CPDXDepthStencilState		m_pDepthReadEqual;

		CPDXRasterizerState			m_pCullNone;
		CPDXRasterizerState			m_pCullClockwise;
		CPDXRasterizerState			m_pCullCounterClockwise;
		CPDXRasterizerState			m_pWireframe;

		CPDXSamplerState			m_pPointWrap;
		CPDXSamplerState			m_pPointClamp;
		CPDXSamplerState			m_pPointBorder;
		CPDXSamplerState			m_pPointComparison;
		CPDXSamplerState			m_pLinearWrap;
		CPDXSamplerState			m_pLinearClamp;
		CPDXSamplerState			m_pLinearBorder;
		CPDXSamplerState			m_pLinearComparison;
		CPDXSamplerState			m_pAnisotropicWrap;
		CPDXSamplerState			m_pAnisotropicClamp;
		CPDXSamplerState			m_pAnisotropicBorder;
		CPDXSamplerState			m_pAnisotropicComparison;

		CPDXDevice					m_pDXDevice;
	};

	using upState = std::unique_ptr<State>;
	using spState = std::shared_ptr<State>;
}
