//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "Poisson3D.h"

#define VISC_ITERATION	0

class Fluid3D
{
public:
	Fluid3D(const XSDX::CPDXDevice &pDXDevice, const XSDX::spShader &pShader, const XSDX::spState &pState);

	void Init(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uDepth);
	void Simulate(
		const float fDeltaTime,
		const XSDX::CPDXShaderResourceView &pImpulseSRV,
		const uint8_t uItVisc = VISC_ITERATION
		);
	void Simulate(
		const float fDeltaTime,
		const DirectX::XMFLOAT4 vForceDens = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		const DirectX::XMFLOAT3 vImLoc = DirectX::XMFLOAT3(0, 0, 0),
		const uint8_t uItVisc = VISC_ITERATION
		);
	void Render(const XSDX::CPDXShaderResourceView &pDepthSRV);
	void Render();

protected:
	void createConstBuffers();
	void advect();
	void diffuse(const uint8_t uIteration);
	void impulse(const XSDX::CPDXShaderResourceView &pImpulseSRV);
	void impulse();
	void project();
	void bound();

	XSDX::spTexture3D	m_pTxVelocity;
	XSDX::spTexture3D	m_pTxAdvVelocity;
	XSDX::spTexture3D	m_pTxDensity;
	XSDX::spTexture3D	m_pTxAdvDensity;

	uint8_t				m_uCBPerFrame;
	uint8_t				m_uUASlot;
	uint8_t				m_uSRField;
	uint8_t				m_uSmpLinearClamp;

	DirectX::XMFLOAT3	m_vSimSize;
	DirectX::XMUINT3	m_vThreadGroupSize;

	upPoisson3D			m_pDiffuse;
	upPoisson3D			m_pPressure;

	XSDX::spShader		m_pShader;
	XSDX::spState		m_pState;

	XSDX::CPDXBuffer	m_pCBImmutable;
	XSDX::CPDXBuffer	m_pCBPerFrame;

	XSDX::CPDXDevice	m_pDXDevice;
	XSDX::CPDXContext	m_pDXContext;
};

using upFluid3D = std::unique_ptr<Fluid3D>;
using spFluid3D = std::shared_ptr<Fluid3D>;
using vuFluid3D = std::vector<upFluid3D>;
using vpFluid3D = std::vector<spFluid3D>;
