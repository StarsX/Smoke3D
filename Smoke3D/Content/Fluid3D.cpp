//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "SharedMacros.h"
#include "Fluid3D.h"

using namespace DirectX;
using namespace DX;
using namespace std;
using namespace ShaderIDs;
using namespace XSDX;

const auto g_pNullSRV = static_cast<LPDXShaderResourceView>(nullptr);	// Helper to Clear SRVs
const auto g_pNullUAV = static_cast<LPDXUnorderedAccessView>(nullptr);	// Helper to Clear UAVs
const auto g_iNullUint = 0u;											// Helper to Clear Buffers

Fluid3D::Fluid3D(const CPDXDevice &pDXDevice, const spShader &pShader, const spState &pState) :
	m_pDXDevice(pDXDevice),
	m_pShader(pShader),
	m_pState(pState),
	m_uCBPerFrame(1ui8),
	m_uUASlot(0ui8),
	m_uSRField(0ui8),
	m_uSmpLinearClamp(1ui8)
{
	m_pDiffuse = make_unique<Poisson3D>(pDXDevice, pShader, pState);
	m_pPressure = make_unique<Poisson3D>(pDXDevice, pShader, pState);
	m_pDiffuse->SetShaders(g_uCSDiffuse);
	m_pPressure->SetShaders(g_uCSPressure, g_uCSTemporal, g_uCSDiv);
	m_pDXDevice->GetImmediateContext(&m_pDXContext);
}

void Fluid3D::Init(uint32_t uWidth, uint32_t uHeight, uint32_t uDepth)
{
	const auto fWidth = static_cast<float>(uWidth);
	const auto fHeight = static_cast<float>(uHeight);
	const auto fDepth = static_cast<float>(uDepth);
	m_vSimSize = XMFLOAT3A(fWidth, fHeight, fDepth);
	createConstBuffers();

	// Create 3D textures
	m_pTxDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pTxAdvDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pTxDensity->Create(true, false, uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);
	m_pTxAdvDensity->Create(true, false, uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);

	m_pDiffuse->Init(uWidth, uHeight, uDepth, sizeof(uint16_t[4]), DXGI_FORMAT_R16G16B16A16_FLOAT);
	m_pPressure->Init(uWidth, uHeight, uDepth, sizeof(float), DXGI_FORMAT_R32_FLOAT);
	m_pTxVelocity = m_pDiffuse->GetResult();
	m_pTxAdvVelocity = m_pDiffuse->GetKnown();

	m_vThreadGroupSize = XMUINT3(uWidth / THREAD_BLOCK_X, uHeight / THREAD_BLOCK_Y, uDepth / THREAD_BLOCK_Z);
}

void Fluid3D::Simulate(const float fDeltaTime, const DirectX::XMFLOAT4 vForceDens, const DirectX::XMFLOAT3 vImLoc, const uint8_t uItVisc)
{
	const auto cbPerFrame = array<XMFLOAT4, 2>
	{ { vForceDens, XMFLOAT4(vImLoc.x, vImLoc.y, vImLoc.z, fDeltaTime) } };
	m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0u, nullptr, cbPerFrame.data(), 0u, 0u);
	m_pDXContext->CSSetConstantBuffers(m_uCBPerFrame, 1u, m_pCBPerFrame.GetAddressOf());

	advect();			// Vel -> AdvVel, Den -> AdvDen
	// UAV: AdvVel, AdvDen, SRV: Vel, Den
	diffuse(uItVisc);	// AdvVel -> Vel
	// UAV: Vel, AdvDen, SRV: AdvVel, DffPp
	// UAV: null, AdvDen, SRV: null, null
	impulse();			// Vel -> AdvVel, AdvDen -> Den
	// UAV: AdvVel, Den, SRV: Vel, AdvDen
	project();			// AdvVel -> Vel, Vel -> AdvVel, AdvVel -> Vel
	// UAV: PrsKn, Den, SRV: AdvVel, AdvDen
	// UAV: null, Den, SRV: null, AdvDen
	// UAV: PrsUk, Den, SRV: PrsKn, AdvDen
	// UAV: null, Den, SRV: null, null
	// UAV: Vel, Den, SRV: AdvVel, null
	// UAV: AdvVel, Den, SRV: Vel, PrsUk
	// UAV: Vel, Den, SRV: AdvVel, PrsUk
	// UAV: PrsPp, Den, SRV: PrsUK, Vel		// temporal
	// UAV: null, Den, SRV: Vel, PrsUK		// temporal

	const auto pUAVs = array<LPDXUnorderedAccessView, 2>{ { nullptr, nullptr } };
	const auto pSRVs = array<LPDXShaderResourceView, 2>{ { nullptr, nullptr } };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 2u, pUAVs.data(), &g_iNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pSRVs.data());
}

void Fluid3D::Render()
{
	m_pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_pDXContext->IASetVertexBuffers(0u, 1u, &g_pNullBuffer, &g_iNullUint, &g_iNullUint);

	m_pDXContext->VSSetShader(m_pShader->GetVertexShader(g_uVSRayCast).Get(), nullptr, 0u);
	m_pDXContext->GSSetShader(nullptr, nullptr, 0u);
	m_pDXContext->PSSetShader(m_pShader->GetPixelShader(g_uPSRayCast).Get(), nullptr, 0u);
	
	m_pDXContext->PSSetShaderResources(m_uSRField, 1u, m_pTxDensity->GetSRV().GetAddressOf());
	m_pDXContext->PSSetSamplers(m_uSmpLinearClamp, 1u, m_pState->LinearClamp().GetAddressOf());

	m_pDXContext->OMSetBlendState(m_pState->NonPremultiplied().Get(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);

	m_pDXContext->Draw(36, 0);

	m_pDXContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

	m_pDXContext->PSSetShaderResources(m_uSRField, 1u, &g_pNullSRV);

	m_pDXContext->VSSetShader(nullptr, nullptr, 0);
	m_pDXContext->PSSetShader(nullptr, nullptr, 0);
	m_pDXContext->RSSetState(0);
}

void Fluid3D::createConstBuffers()
{
	// Setup constant buffers
	auto desc = CD3D11_BUFFER_DESC(sizeof(XMFLOAT4[2]), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, nullptr, &m_pCBPerFrame));

	const auto cbImmutable = XMFLOAT3A(1.0f / m_vSimSize.x, 1.0f / m_vSimSize.y, 1.0f / m_vSimSize.z);
	desc.ByteWidth = sizeof(XMFLOAT3A);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	const auto dsd = D3D11_SUBRESOURCE_DATA{ &cbImmutable };
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, &dsd, &m_pCBImmutable));

	m_pDXContext->CSSetConstantBuffers(0u, 1u, m_pCBImmutable.GetAddressOf());
}

void Fluid3D::advect()
{
	const auto initUAVCounts = 0u;

	// Setup
	const auto pUAVs = array<LPDXUnorderedAccessView, 2>
	{ { m_pTxAdvVelocity->GetUAV().Get(), m_pTxAdvDensity->GetUAV().Get() } };
	const auto pSRVs = array<LPDXShaderResourceView, 2>
	{ { m_pTxVelocity->GetSRV().Get(), m_pTxDensity->GetSRV().Get() } };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 2u, pUAVs.data(), &initUAVCounts);
	m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pSRVs.data());
	m_pDXContext->CSSetSamplers(m_uSmpLinearClamp, 1u, m_pState->LinearClamp().GetAddressOf());

	// Compute dye advection
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSAdvect).Get(), nullptr, 0u);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	const auto pNullSRVs = array<LPDXShaderResourceView, 2>{ { nullptr, nullptr } };
	m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pNullSRVs.data());
}

void Fluid3D::diffuse(const uint8_t uIteration)
{
	if (uIteration > 0u) m_pDiffuse->SolvePoisson(uIteration);
	else
	{
		m_pDiffuse->SwapBuffers();
		m_pTxAdvVelocity = m_pDiffuse->GetKnown();
	}
	m_pTxVelocity = m_pDiffuse->GetResult();
}

void Fluid3D::impulse()
{
	const auto initUAVCounts = 0u;

	// Setup
	const auto pUAVs = array<LPDXUnorderedAccessView, 2>
	{ { m_pTxAdvVelocity->GetUAV().Get(), m_pTxDensity->GetUAV().Get() } };
	const auto pSRVs = array<LPDXShaderResourceView, 2>
	{ { m_pTxVelocity->GetSRV().Get(), m_pTxAdvDensity->GetSRV().Get() } };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 2u, pUAVs.data(), &initUAVCounts);
	m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pSRVs.data());

	// Add force and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSImpulse).Get(), nullptr, 0);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);
}

void Fluid3D::project()
{
	m_pPressure->ComputeDivergence(m_pTxAdvVelocity->GetSRV());
	m_pPressure->SolvePoisson();

	bound();

	// Projection
	{
		const auto initUAVCounts = 0u;

		// Setup
		const auto pSRVs = array<LPDXShaderResourceView, 2>
		{ { m_pTxVelocity->GetSRV().Get(), m_pPressure->GetResult()->GetSRV().Get() } };
		m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1u, m_pTxAdvVelocity->GetUAV().GetAddressOf(), &initUAVCounts);
		m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pSRVs.data());

		// Compute projection
		m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSProject).Get(), nullptr, 0u);
		m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

		// Unset
		m_pDXContext->CSSetShaderResources(m_uSRField, 1u, &g_pNullSRV);
	}

	bound();

#ifdef _ADVECT_PRESSURE_
	// Temporal optimization
	m_pPressure->Advect(m_pTxVelocity->GetSRV());
#endif
}

void Fluid3D::bound()
{
	const auto initUAVCounts = 0u;

	// Setup
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1u, m_pTxVelocity->GetUAV().GetAddressOf(), &initUAVCounts);
	m_pDXContext->CSSetShaderResources(m_uSRField, 1u, m_pTxAdvVelocity->GetSRV().GetAddressOf());

	// Compute boundary conditions
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSBound).Get(), nullptr, 0u);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	m_pDXContext->CSSetShaderResources(m_uSRField, 1u, &g_pNullSRV);
}
