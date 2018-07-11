//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "SharedMacros.h"
#include "Fluid3D.h"

using namespace DirectX;
using namespace DX;
using namespace std;
using namespace ShaderIDs;
using namespace XSDX;

const auto g_pNullSRV = static_cast<LPDXShaderResourceView>(nullptr);	// Helper to Clear SRVs
const auto g_pNullUAV = static_cast<LPDXUnorderedAccessView>(nullptr);	// Helper to Clear UAVs
const auto g_uNullUint = 0u;											// Helper to Clear Buffers

Fluid3D::Fluid3D(const CPDXDevice &pDXDevice, const spShader &pShader, const spState &pState) :
	m_pDXDevice(pDXDevice),
	m_pShader(pShader),
	m_pState(pState),
	m_uCBImmutable(0),
	m_uCBPerFrame(1),
	m_uUASlot(0),
	m_uSRField(0),
	m_uSmpLinearClamp(1)
{
	m_pDiffuse = make_unique<Poisson3D>(pDXDevice, pShader, pState);
	m_pPressure = make_unique<Poisson3D>(pDXDevice, pShader, pState);
	m_pDiffuse->SetShaders(g_uCSDiffuse);
	m_pPressure->SetShaders(g_uCSPressure, g_uCSTemporal, g_uCSDivergence);
	m_pDXDevice->GetImmediateContext(&m_pDXContext);
}

void Fluid3D::Init(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uDepth)
{
	const auto fWidth = static_cast<float>(uWidth);
	const auto fHeight = static_cast<float>(uHeight);
	const auto fDepth = static_cast<float>(uDepth);
	m_vSimSize = XMFLOAT3A(fWidth, fHeight, fDepth);
	createConstBuffers();

	// Create 3D textures
	m_pSrcDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pDstDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pSrcDensity->Create(uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);
	m_pDstDensity->Create(uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);

	// For macCormack advection
	m_pTmpDensity = make_unique<Texture3D>(m_pDXDevice);
	m_pTmpDensity->Create(uWidth, uHeight, uDepth, DXGI_FORMAT_R16_FLOAT);

	m_pDiffuse->Init(uWidth, uHeight, uDepth, sizeof(uint16_t[4]), DXGI_FORMAT_R16G16B16A16_FLOAT);
	m_pPressure->Init(uWidth, uHeight, uDepth, sizeof(float), DXGI_FORMAT_R32_FLOAT);
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();

	m_vThreadGroupSize = XMUINT3(uWidth / THREAD_BLOCK_X, uHeight / THREAD_BLOCK_Y, uDepth / THREAD_BLOCK_Z);
}

void Fluid3D::Simulate(const float fDeltaTime, const DirectX::XMFLOAT4 vForceDens,
	const DirectX::XMFLOAT3 vImLoc, const uint8_t uItVisc, const bool bMacCormack)
{
	m_vPerFrames[0] = vForceDens;
	m_vPerFrames[1] = XMFLOAT4(vImLoc.x, vImLoc.y, vImLoc.z, fDeltaTime);
	const auto pCBs = { m_pCBImmutables[bMacCormack].Get(), m_pCBPerFrame.Get() };
	m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0, nullptr, m_vPerFrames, 0, 0);
	m_pDXContext->CSSetConstantBuffers(m_uCBImmutable, static_cast<uint32_t>(pCBs.size()), pCBs.begin());

	advect(fDeltaTime, bMacCormack);
	diffuse(uItVisc);
	impulse();
	project();

	const auto vpUAVs = vLPDXUAV(2, nullptr);
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, static_cast<uint32_t>(vpUAVs.size()), vpUAVs.data(), &g_uNullUint);
}

void Fluid3D::Render(const XSDX::CPDXUnorderedAccessView &pUAVSwapChain)
{
	auto pSrc = CPDXResource();
	auto desc = D3D11_TEXTURE2D_DESC();
	pUAVSwapChain->GetResource(&pSrc);
	static_cast<LPDXTexture2D>(pSrc.Get())->GetDesc(&desc);

	// Setup
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1, pUAVSwapChain.GetAddressOf(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, 1, m_pSrcDensity->GetSRV().GetAddressOf());
	m_pDXContext->CSSetSamplers(m_uSmpLinearClamp, 1, m_pState->LinearClamp().GetAddressOf());

	// Advect velocity and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSRayCast).Get(), nullptr, 0);
	m_pDXContext->Dispatch(desc.Width / 32, desc.Height / 16, 1);

	// Unset
	m_pDXContext->CSSetShaderResources(m_uSRField, 1, &g_pNullSRV);
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1, &g_pNullUAV, &g_uNullUint);
}

void Fluid3D::createConstBuffers()
{
	// Setup constant buffers
	auto desc = CD3D11_BUFFER_DESC(sizeof(XMFLOAT4[2]), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, nullptr, &m_pCBPerFrame));

	auto cbImmutable = XMFLOAT4(1.0f / m_vSimSize.x, 1.0f / m_vSimSize.y, 1.0f / m_vSimSize.z, 0.0f);
	desc.ByteWidth = sizeof(XMFLOAT4);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	const auto dsd = D3D11_SUBRESOURCE_DATA{ &cbImmutable };
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, &dsd, &m_pCBImmutables[SEMI_LAGRANGE]));

	cbImmutable.w = 1.0f;
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, &dsd, &m_pCBImmutables[MAC_CORMACK]));
}

void Fluid3D::advect(const float fDeltaTime, const bool bMacCormack)
{
	auto pSrcVelocity = m_pSrcVelocity->GetSRV();
	advect(fDeltaTime, pSrcVelocity);
	
	if (bMacCormack)
	{
		m_pDiffuse->SwapTextures(true);
		m_pDstVelocity = m_pDiffuse->GetDst();
		pSrcVelocity = m_pDiffuse->GetTmp()->GetSRV();
		m_pTmpDensity.swap(m_pDstDensity);
		advect(-fDeltaTime, pSrcVelocity);

		macCormack(fDeltaTime, pSrcVelocity);
	}
}

void Fluid3D::advect(const float fDeltaTime, const CPDXShaderResourceView& pSRVVelocity)
{
	m_vPerFrames[1].w = fDeltaTime;
	m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0, nullptr, m_vPerFrames, 0, 0);

	// Setup
	const auto pUAVs = { m_pDstVelocity->GetUAV().Get(), m_pDstDensity->GetUAV().Get() };
	const auto pSRVs = { m_pSrcVelocity->GetSRV().Get(), m_pSrcDensity->GetSRV().Get(), pSRVVelocity.Get() };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, static_cast<uint32_t>(pUAVs.size()), pUAVs.begin(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(pSRVs.size()), pSRVs.begin());
	m_pDXContext->CSSetSamplers(m_uSmpLinearClamp, 1, m_pState->LinearClamp().GetAddressOf());

	// Advect velocity and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSAdvect).Get(), nullptr, 0);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	const auto vpNullSRVs = vLPDXSRV(pSRVs.size(), nullptr);
	m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(vpNullSRVs.size()), vpNullSRVs.data());

	// Swap buffers
	m_pDiffuse->SwapTextures();
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();
	m_pSrcDensity.swap(m_pDstDensity);
}

void Fluid3D::macCormack(const float fDeltaTime, const CPDXShaderResourceView& pSRVVelocity)
{
	m_vPerFrames[1].w = fDeltaTime;
	m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0, nullptr, m_vPerFrames, 0, 0);

	// Setup
	const auto pUAVs = { m_pDstVelocity->GetUAV().Get(), m_pDstDensity->GetUAV().Get() };
	const auto pSamplers = { m_pState->LinearClamp().Get(), m_pState->PointClamp().Get() };
	const auto pSRVs =
	{
		pSRVVelocity.Get(), m_pTmpDensity->GetSRV().Get(),
		m_pSrcVelocity->GetSRV().Get(), m_pSrcDensity->GetSRV().Get()
	};
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, static_cast<uint32_t>(pUAVs.size()), pUAVs.begin(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(pSRVs.size()), pSRVs.begin());
	m_pDXContext->CSSetSamplers(m_uSmpLinearClamp, static_cast<uint32_t>(pSamplers.size()), pSamplers.begin());

	// Advect velocity and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSMacCormack).Get(), nullptr, 0);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	const auto vpNullSRVs = vLPDXSRV(pSRVs.size(), nullptr);
	m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(vpNullSRVs.size()), vpNullSRVs.data());

	// Swap buffers
	m_pDiffuse->SwapTextures();
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();
	m_pSrcDensity.swap(m_pDstDensity);
}

void Fluid3D::diffuse(const uint8_t uIteration)
{
	if (uIteration > 0)
	{
		m_pDiffuse->SolvePoisson(uIteration);
		m_pSrcVelocity = m_pDiffuse->GetSrc();
		m_pDstVelocity = m_pDiffuse->GetDst();
	}
}

void Fluid3D::impulse()
{
	// Setup
	const auto pUAVs = { m_pDstVelocity->GetUAV().Get(), m_pDstDensity->GetUAV().Get() };
	const auto pSRVs = { m_pSrcVelocity->GetSRV().Get(), m_pSrcDensity->GetSRV().Get() };
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, static_cast<uint32_t>(pUAVs.size()), pUAVs.begin(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(pSRVs.size()), pSRVs.begin());

	// Add force and density
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSImpulse).Get(), nullptr, 0);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	const auto vpNullSRVs = vLPDXSRV(pSRVs.size(), nullptr);
	m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(vpNullSRVs.size()), vpNullSRVs.data());

	// Swap buffers
	m_pDiffuse->SwapTextures();
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
		const auto pSRVs = { m_pSrcVelocity->GetSRV().Get(), m_pPressure->GetSrc()->GetSRV().Get() };
		m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1, m_pDstVelocity->GetUAV().GetAddressOf(), &g_uNullUint);
		m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(pSRVs.size()), pSRVs.begin());

		// Compute projection
		m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSProject).Get(), nullptr, 0);
		m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

		// Unset
		const auto vpNullSRVs = vLPDXSRV(pSRVs.size(), nullptr);
		m_pDXContext->CSSetShaderResources(m_uSRField, static_cast<uint32_t>(vpNullSRVs.size()), vpNullSRVs.data());

		// Swap buffers
		m_pDiffuse->SwapTextures();
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
	m_pDXContext->CSSetUnorderedAccessViews(m_uUASlot, 1, m_pDstVelocity->GetUAV().GetAddressOf(), &g_uNullUint);
	m_pDXContext->CSSetShaderResources(m_uSRField, 1, m_pSrcVelocity->GetSRV().GetAddressOf());

	// Compute boundary conditions
	m_pDXContext->CSSetShader(m_pShader->GetComputeShader(g_uCSBound).Get(), nullptr, 0);
	m_pDXContext->Dispatch(m_vThreadGroupSize.x, m_vThreadGroupSize.y, m_vThreadGroupSize.z);

	// Unset
	m_pDXContext->CSSetShaderResources(m_uSRField, 1, &g_pNullSRV);

	// Swap buffers
	m_pDiffuse->SwapTextures();
	m_pSrcVelocity = m_pDiffuse->GetSrc();
	m_pDstVelocity = m_pDiffuse->GetDst();
}
