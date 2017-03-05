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
const auto g_uNullUint = 0u;											// Helper to Clear Buffers

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
	m_pSrcDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pDstDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pSrcDensity->Create(true, false, uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);
	m_pDstDensity->Create(true, false, uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);

	m_pDiffuse->Init(uWidth, uHeight, uDepth, sizeof(uint16_t[4]), DXGI_FORMAT_R16G16B16A16_FLOAT);
	m_pPressure->Init(uWidth, uHeight, uDepth, sizeof(float), DXGI_FORMAT_R32_FLOAT);
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();

	m_vThreadGroupSize = XMUINT3(uWidth / THREAD_BLOCK_X, uHeight / THREAD_BLOCK_Y, uDepth / THREAD_BLOCK_Z);
}

void Fluid3D::Simulate(const float fDeltaTime, const DirectX::XMFLOAT4 vForceDens, const DirectX::XMFLOAT3 vImLoc, const uint8_t uItVisc)
{
	m_vPerFrames[0] = vForceDens;
	m_vPerFrames[1] = XMFLOAT4(vImLoc.x, vImLoc.y, vImLoc.z, fDeltaTime);
	m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0u, nullptr, m_vPerFrames, 0u, 0u);
	m_pDXContext->CSSetConstantBuffers(m_uCBPerFrame, 1u, m_pCBPerFrame.GetAddressOf());

	advect(fDeltaTime);
	diffuse(uItVisc);
	impulse();
	project();

	const auto pUAVs = array<LPDXUnorderedAccessView, 2>{ { nullptr, nullptr } };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 2u, pUAVs.data(), &g_uNullUint);
}

void Fluid3D::Render()
{
	m_pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_pDXContext->IASetVertexBuffers(0u, 1u, &g_pNullBuffer, &g_iNullUint, &g_iNullUint);

	m_pDXContext->VSSetShader(m_pShader->GetVertexShader(g_uVSRayCast).Get(), nullptr, 0u);
	m_pDXContext->GSSetShader(nullptr, nullptr, 0u);
	m_pDXContext->PSSetShader(m_pShader->GetPixelShader(g_uPSRayCast).Get(), nullptr, 0u);
	
	m_pDXContext->PSSetShaderResources(m_uSRField, 1u, m_pSrcDensity->GetSRV().GetAddressOf());
	m_pDXContext->PSSetSamplers(m_uSmpLinearClamp, 1u, m_pState->LinearClamp().GetAddressOf());

	m_pDXContext->OMSetBlendState(m_pState->NonPremultiplied().Get(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);

	m_pDXContext->Draw(36u, 0u);

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

void Fluid3D::advect(const float fDeltaTime)
{
	auto pSrcVelocity = m_pSrcVelocity->GetSRV();
	advect(fDeltaTime, pSrcVelocity);
	
#if 0
	m_pDiffuse->SwapBuffers(true);
	pSrcVelocity = m_pDiffuse->GetTmp()->GetSRV();
	advect(-fDeltaTime, pSrcVelocity);

	advect(fDeltaTime, pSrcVelocity, g_uCSAdvectMC);
#endif
}

void Fluid3D::advect(const float fDeltaTime, const XSDX::CPDXShaderResourceView& pSRVVelocity, const uint8_t uCS)
{
	m_vPerFrames[1].w = fDeltaTime;
	m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0u, nullptr, m_vPerFrames, 0u, 0u);

	// Setup
	const auto pUAVs = array<LPDXUnorderedAccessView, 2>
	{ { m_pDstVelocity->GetUAV().Get(), m_pDstDensity->GetUAV().Get() } };
	const auto pSRVs = array<LPDXShaderResourceView, 3>
	{ { m_pSrcVelocity->GetSRV().Get(), m_pSrcDensity->GetSRV().Get(), pSRVVelocity.Get() } };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 2u, pUAVs.data(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, 3u, pSRVs.data());
	m_pDXContext->CSSetSamplers(m_uSmpLinearClamp, 1u, m_pState->LinearClamp().GetAddressOf());

	// Advect velocity and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSAdvect).Get(), nullptr, 0u);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	const auto pNullSRVs = array<LPDXShaderResourceView, 3>{ { nullptr, nullptr, nullptr } };
	m_pDXContext->CSSetShaderResources(m_uSRField, 3u, pNullSRVs.data());

	// Swap buffers
	m_pDiffuse->SwapBuffers();
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();
	m_pSrcDensity.swap(m_pDstDensity);
}

void Fluid3D::diffuse(const uint8_t uIteration)
{
	if (uIteration > 0u)
	{
		m_pDiffuse->SolvePoisson(uIteration);
		m_pSrcVelocity = m_pDiffuse->GetSrc();
		m_pDstVelocity = m_pDiffuse->GetDst();
	}
}

void Fluid3D::impulse()
{
	// Setup
	const auto pUAVs = array<LPDXUnorderedAccessView, 2>
	{ { m_pDstVelocity->GetUAV().Get(), m_pDstDensity->GetUAV().Get() } };
	const auto pSRVs = array<LPDXShaderResourceView, 2>
	{ { m_pSrcVelocity->GetSRV().Get(), m_pSrcDensity->GetSRV().Get() } };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 2u, pUAVs.data(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pSRVs.data());

	// Add force and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSImpulse).Get(), nullptr, 0u);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	const auto pNullSRVs = array<LPDXShaderResourceView, 2>{ { nullptr, nullptr } };
	m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pNullSRVs.data());

	// Swap buffers
	m_pDiffuse->SwapBuffers();
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();
	m_pSrcDensity.swap(m_pDstDensity);
}

void Fluid3D::project()
{
	m_pPressure->ComputeDivergence(m_pSrcVelocity->GetSRV());
	m_pPressure->SolvePoisson();

	bound();

	// Projection
	{
		// Setup
		const auto pSRVs = array<LPDXShaderResourceView, 2>
		{ { m_pSrcVelocity->GetSRV().Get(), m_pPressure->GetSrc()->GetSRV().Get() } };
		m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1u, m_pDstVelocity->GetUAV().GetAddressOf(), &g_uNullUint);
		m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pSRVs.data());

		// Compute projection
		m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSProject).Get(), nullptr, 0u);
		m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

		// Unset
		const auto pNullSRVs = array<LPDXShaderResourceView, 2>{ { nullptr, nullptr } };
		m_pDXContext->CSSetShaderResources(m_uSRField, 2u, pNullSRVs.data());

		// Swap buffers
		m_pDiffuse->SwapBuffers();
		m_pSrcVelocity = m_pDiffuse->GetSrc();
		m_pDstVelocity = m_pDiffuse->GetDst();
	}

	bound();

#ifdef _ADVECT_PRESSURE_
	// Temporal optimization
	m_pPressure->Advect(m_pSrcVelocity->GetSRV());
#endif
}

void Fluid3D::bound()
{
	// Setup
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1u, m_pDstVelocity->GetUAV().GetAddressOf(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, 1u, m_pSrcVelocity->GetSRV().GetAddressOf());

	// Compute boundary conditions
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSBound).Get(), nullptr, 0u);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	m_pDXContext->CSSetShaderResources(m_uSRField, 1u, &g_pNullSRV);

	// Swap buffers
	m_pDiffuse->SwapBuffers();
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();
}
