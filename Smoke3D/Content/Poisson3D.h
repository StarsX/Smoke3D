//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXShader.h"
#include "XSDXState.h"
#include "XSDXBuffer.h"
#include "ShaderCommon3D.h"

class Poisson3D
{
public:
	Poisson3D(const XSDX::CPDXDevice &pDXDevice, const XSDX::spShader &pShader, const XSDX::spState &pState);

	void Init(const DirectX::XMUINT3 &vSimSize, const uint8_t uStride, const DXGI_FORMAT format);
	void Init(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uDepth,
		const uint8_t uStride, const DXGI_FORMAT format);
	void SetShaders(const uint8_t uCSIteration, const uint8_t uCSTemporal = 1ui8, const uint8_t uCSDiv = 2ui8);
	void ComputeDivergence(const XSDX::CPDXShaderResourceView &srvSource);
	void SolvePoisson(const uint8_t uIteration = 1ui8);
	void Advect(const XSDX::CPDXShaderResourceView &srvSource);
	void SwapTextures(const bool bUnknown = false);

	const XSDX::spTexture3D	&GetSrc() const;
	const XSDX::spTexture3D	&GetDst() const;
	const XSDX::spTexture3D	&GetTmp() const;

protected:
	void gaussSeidel();
	void jacobi();

	XSDX::spTexture3D	m_pSrcKnown;
	XSDX::spTexture3D	m_pSrcUnknown;
	XSDX::spTexture3D	m_pDstUnknown;

	uint8_t				m_uCSIteration;
	uint8_t				m_uCSTemporal;
	uint8_t				m_uCSDiv;

	uint8_t				m_uUASlot;
	uint8_t				m_uSRField;
	uint8_t				m_uSmpLinearClamp;

	DirectX::XMUINT3	m_vThreadGroupSize;

	XSDX::spShader		m_pShader;
	XSDX::spState		m_pState;

	XSDX::CPDXDevice	m_pDXDevice;
	XSDX::CPDXContext	m_pDXContext;
};

using upPoisson3D = std::unique_ptr<Poisson3D>;
using spPoisson3D = std::shared_ptr<Poisson3D>;
