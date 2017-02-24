//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "XSDXShader.h"

using namespace std;
using namespace Concurrency;
using namespace DX;
using namespace XSDX;

Shader::Shader(const CPDXDevice &pDXDevice) :
	m_pDXDevice(pDXDevice)
{
}

TaskCPDXBlob Shader::ReadShaderFile(const wstring &szFileName)
{
#ifdef WIN32_DESKTOP_DXUT
	return create_task([szFileName]()
	{
		auto pReturnBuffer = CPDXBlob();
		ThrowIfFailed(D3DReadFileToBlob(szFileName.c_str(), &pReturnBuffer));
		
		return pReturnBuffer;
	});
#else
	return ReadDataAsync(szFileName.c_str()).then([this, i](const vbyte& vFileData)
	{
		auto pReturnBuffer = CPDXBlob();
		const auto uSize = vFileData.size();
		ThrowIfFailed(D3DCreateBlob(uSize, &pReturnBuffer));
		memcpy(pReturnBuffer->GetBufferPointer(), vFileData.data(), uSize);

		return pReturnBuffer;
	});
#endif
}

TaskVoid Shader::CreateVertexShader(const wstring &szFileName, const uint8_t i)
{
	auto loadVSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadVSTask.then([this, i](const CPDXBlob &pFileData)
	{
		m_ppVSBuffers[i] = pFileData;
		ThrowIfFailed(
			m_pDXDevice->CreateVertexShader(
				pFileData->GetBufferPointer(),
				pFileData->GetBufferSize(),
				nullptr, &m_ppVertexShaders[i]
				)
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppVSReflectors[i]
				)
			);
	});
}

TaskVoid Shader::CreateHullShader(const wstring &szFileName, const uint8_t i)
{
	auto loadVSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadVSTask.then([this, i](const CPDXBlob &pFileData)
	{
		ThrowIfFailed(
			m_pDXDevice->CreateHullShader(
				pFileData->GetBufferPointer(),
				pFileData->GetBufferSize(),
				nullptr, &m_ppHullShaders[i]
				)
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppHSReflectors[i]
				)
			);
	});
}

TaskVoid Shader::CreateDomainShader(const wstring &szFileName, const uint8_t i)
{
	auto loadVSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadVSTask.then([this, i](const CPDXBlob &pFileData)
	{
		ThrowIfFailed(
			m_pDXDevice->CreateDomainShader(
				pFileData->GetBufferPointer(),
				pFileData->GetBufferSize(),
				nullptr, &m_ppDomainShaders[i]
				)
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppDSReflectors[i]
				)
			);
	});
}

TaskVoid Shader::CreateGeometryShader(const wstring &szFileName, const uint8_t i)
{
	auto loadGSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadGSTask.then([this, i](const CPDXBlob &pFileData)
	{
		ThrowIfFailed(
			m_pDXDevice->CreateGeometryShader(
				pFileData->GetBufferPointer(),
				pFileData->GetBufferSize(),
				nullptr, &m_ppGeometryShaders[i]
				)
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppGSReflectors[i]
				)
			);
	});
}

TaskVoid Shader::CreateGeometryShaderWithSO(const wstring &szFileName, const uint8_t i,
	const LPCD3D11_SO_DECLARATION_ENTRY pDecl, const uint8_t uNumEntry)
{
	auto loadGSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadGSTask.then([this, i, pDecl, uNumEntry](const CPDXBlob &pFileData)
	{
		auto uStride = 0u;
		for (auto j = 0ui8; j < uNumEntry; ++j)
			uStride += pDecl[j].ComponentCount * sizeof(float);
		ThrowIfFailed(
			m_pDXDevice->CreateGeometryShaderWithStreamOutput(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(), pDecl,
				uNumEntry, &uStride, 1u, 0u, nullptr, &m_ppGeometryShaders[i])
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppGSReflectors[i]
				)
			);
	});
}

TaskVoid Shader::CreatePixelShader(const wstring &szFileName, const uint8_t i)
{
	auto loadPSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadPSTask.then([this, i](const CPDXBlob &pFileData)
	{
		ThrowIfFailed(
			m_pDXDevice->CreatePixelShader(
				pFileData->GetBufferPointer(),
				pFileData->GetBufferSize(),
				nullptr, &m_ppPixelShaders[i]
				)
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppPSReflectors[i]
				)
			);
	});
}

TaskVoid Shader::CreateComputeShader(const wstring &szFileName, const uint8_t i)
{
	auto loadCSTask = ReadShaderFile(szFileName);

	// Create the shader
	return loadCSTask.then([this, i](const CPDXBlob &pFileData)
	{
		ThrowIfFailed(
			m_pDXDevice->CreateComputeShader(
				pFileData->GetBufferPointer(),
				pFileData->GetBufferSize(),
				nullptr, &m_ppComputeShaders[i]
				)
			);
		ThrowIfFailed(
			D3DReflect(
				pFileData->GetBufferPointer(), pFileData->GetBufferSize(),
				IID_ID3D11ShaderReflection, &m_ppCSReflectors[i]
				)
			);
	});
}

void Shader::CreateGeometryShaderWithSO(const uint8_t i, const LPCD3D11_SO_DECLARATION_ENTRY pDecl,
	const uint8_t uNumEntry)
{
	// Create the shader
	auto uStride = 0u;
	for (auto j = 0ui8; j < uNumEntry; ++j)
		uStride += pDecl[j].ComponentCount * sizeof(float);

	ThrowIfFailed(
		m_pDXDevice->CreateGeometryShaderWithStreamOutput(
			m_ppVSBuffers[i]->GetBufferPointer(), m_ppVSBuffers[i]->GetBufferSize(),
			pDecl, uNumEntry, &uStride, 1u, 0u, nullptr, &m_ppGeometryShaders[i])
		);

	ThrowIfFailed(
		D3DReflect(
			m_ppVSBuffers[i]->GetBufferPointer(), m_ppVSBuffers[i]->GetBufferSize(),
			IID_ID3D11ShaderReflection, &m_ppGSReflectors[i]
			)
		);
}

void Shader::SetVertexShaderBuffer(const uint8_t i, const CPDXBlob &pFileData)
{
	m_ppVSBuffers[i] = pFileData;
}

void Shader::SetVertexShader(const uint8_t i, const CPDXVertexShader &pShader)
{
	m_ppVertexShaders[i] = pShader;
}

void Shader::SetHullShader(const uint8_t i, const CPDXHullShader &pShader)
{
	m_ppHullShaders[i] = pShader;
}

void Shader::SetDomainShader(const uint8_t i, const CPDXDomainShader &pShader)
{
	m_ppDomainShaders[i] = pShader;
}

void Shader::SetGeometryShader(const uint8_t i, const CPDXGeometryShader &pShader)
{
	m_ppGeometryShaders[i] = pShader;
}

void Shader::SetPixelShader(const uint8_t i, const CPDXPixelShader &pShader)
{
	m_ppPixelShaders[i] = pShader;
}

void Shader::SetComputeShader(const uint8_t i, const CPDXComputeShader & pShader)
{
	m_ppComputeShaders[i] = pShader;
}

void Shader::SetVSReflector(const uint8_t i, const CPDXReflector &pReflector)
{
	m_ppVSReflectors[i] = pReflector;
}

void Shader::SetHSReflector(const uint8_t i, const CPDXReflector &pReflector)
{
	m_ppHSReflectors[i] = pReflector;
}

void Shader::SetDSReflector(const uint8_t i, const CPDXReflector &pReflector)
{
	m_ppDSReflectors[i] = pReflector;
}

void Shader::SetGSReflector(const uint8_t i, const CPDXReflector &pReflector)
{
	m_ppGSReflectors[i] = pReflector;
}

void Shader::SetPSReflector(const uint8_t i, const CPDXReflector &pReflector)
{
	m_ppPSReflectors[i] = pReflector;
}

void Shader::SetCSReflector(const uint8_t i, const CPDXReflector &pReflector)
{
	m_ppCSReflectors[i] = pReflector;
}

void Shader::ReleaseShaderBuffers()
{
	for (auto &pBuffer : m_ppVSBuffers) pBuffer.Reset();
	for (auto &pReflector : m_ppVSReflectors) pReflector.Reset();
	for (auto &pReflector : m_ppHSReflectors) pReflector.Reset();
	for (auto &pReflector : m_ppDSReflectors) pReflector.Reset();
	for (auto &pReflector : m_ppGSReflectors) pReflector.Reset();
	for (auto &pReflector : m_ppPSReflectors) pReflector.Reset();
	for (auto &pReflector : m_ppCSReflectors) pReflector.Reset();
}

const CPDXVertexShader &Shader::GetVertexShader(const uint8_t i) const
{
	return m_ppVertexShaders[i];
}

const CPDXHullShader &Shader::GetHullShader(const uint8_t i) const
{
	return m_ppHullShaders[i];
}

const CPDXDomainShader &Shader::GetDomainShader(const uint8_t i) const
{
	return m_ppDomainShaders[i];
}

const CPDXGeometryShader &Shader::GetGeometryShader(const uint8_t i) const
{
	return m_ppGeometryShaders[i];
}

const CPDXPixelShader &Shader::GetPixelShader(const uint8_t i) const
{
	return m_ppPixelShaders[i];
}

const CPDXComputeShader &Shader::GetComputeShader(const uint8_t i) const
{
	return m_ppComputeShaders[i];
}

const CPDXReflector &Shader::GetVSReflector(const uint8_t i) const
{
	return m_ppVSReflectors[i];
}

const CPDXReflector &Shader::GetHSReflector(const uint8_t i) const
{
	return m_ppHSReflectors[i];
}

const CPDXReflector &Shader::GetDSReflector(const uint8_t i) const
{
	return m_ppDSReflectors[i];
}

const CPDXReflector &Shader::GetGSReflector(const uint8_t i) const
{
	return m_ppGSReflectors[i];
}

const CPDXReflector &Shader::GetPSReflector(const uint8_t i) const
{
	return m_ppPSReflectors[i];
}

const CPDXReflector &Shader::GetCSReflector(const uint8_t i) const
{
	return m_ppCSReflectors[i];
}

const CPDXBlob &Shader::GetVertexShaderBuffer(const uint8_t i) const
{
	return m_ppVSBuffers[i];
}
