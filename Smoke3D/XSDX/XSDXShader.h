//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXType.h"

#define MAX_SHADER_NUM			UINT8_MAX
#ifndef NULL_SHADER
#define NULL_SHADER				UINT8_MAX
#endif

namespace XSDX
{
	class Shader
	{
	public:
		using LPCD3D11_SO_DECLARATION_ENTRY = std::add_pointer_t<const D3D11_SO_DECLARATION_ENTRY>;

		Shader(const CPDXDevice &pDXDevice);
		virtual ~Shader(void);

		TaskCPDXBlob ReadShaderFile(const std::wstring &szFileName);
		TaskVoid CreateVertexShader(const std::wstring &szFileName, const uint8_t i);
		TaskVoid CreateHullShader(const std::wstring &szFileName, const uint8_t i);
		TaskVoid CreateDomainShader(const std::wstring &szFileName, const uint8_t i);
		TaskVoid CreateGeometryShader(const std::wstring &szFileName, const uint8_t i);
		TaskVoid CreateGeometryShaderWithSO(const std::wstring &szFileName, const uint8_t i,
			const LPCD3D11_SO_DECLARATION_ENTRY pDecl, const uint8_t uNumEntries);
		TaskVoid CreatePixelShader(const std::wstring &szFileName, const uint8_t i);
		TaskVoid CreateComputeShader(const std::wstring &szFileName, const uint8_t i);

		void CreateGeometryShaderWithSO(const uint8_t i,
			const LPCD3D11_SO_DECLARATION_ENTRY pDecl, const uint8_t uNumEntries);

		void SetVertexShaderBuffer(const uint8_t i, const CPDXBlob &pFileData);
		void SetVertexShader(const uint8_t i, const CPDXVertexShader &pShader);
		void SetHullShader(const uint8_t i, const CPDXHullShader &pShader);
		void SetDomainShader(const uint8_t i, const CPDXDomainShader &pShader);
		void SetGeometryShader(const uint8_t i, const CPDXGeometryShader &pShader);
		void SetPixelShader(const uint8_t i, const CPDXPixelShader &pShader);
		void SetComputeShader(const uint8_t i, const CPDXComputeShader &pShader);

		void SetVSReflector(const uint8_t i, const CPDXReflector &pReflector);
		void SetHSReflector(const uint8_t i, const CPDXReflector &pReflector);
		void SetDSReflector(const uint8_t i, const CPDXReflector &pReflector);
		void SetGSReflector(const uint8_t i, const CPDXReflector &pReflector);
		void SetPSReflector(const uint8_t i, const CPDXReflector &pReflector);
		void SetCSReflector(const uint8_t i, const CPDXReflector &pReflector);
		void ReleaseShaderBuffers();

		const CPDXVertexShader		&GetVertexShader(const uint8_t i) const;
		const CPDXHullShader		&GetHullShader(const uint8_t i) const;
		const CPDXDomainShader		&GetDomainShader(const uint8_t i) const;
		const CPDXGeometryShader	&GetGeometryShader(const uint8_t i) const;
		const CPDXPixelShader		&GetPixelShader(const uint8_t i) const;
		const CPDXComputeShader		&GetComputeShader(const uint8_t i) const;

		const CPDXReflector			&GetVSReflector(const uint8_t i) const;
		const CPDXReflector			&GetHSReflector(const uint8_t i) const;
		const CPDXReflector			&GetDSReflector(const uint8_t i) const;
		const CPDXReflector			&GetGSReflector(const uint8_t i) const;
		const CPDXReflector			&GetPSReflector(const uint8_t i) const;
		const CPDXReflector			&GetCSReflector(const uint8_t i) const;

		const CPDXBlob				&GetVertexShaderBuffer(const uint8_t i) const;
	protected:
		CPDXBlob			m_ppVSBuffers[MAX_SHADER_NUM];

		CPDXVertexShader	m_ppVertexShaders[MAX_SHADER_NUM];
		CPDXHullShader		m_ppHullShaders[MAX_SHADER_NUM];
		CPDXDomainShader	m_ppDomainShaders[MAX_SHADER_NUM];
		CPDXGeometryShader	m_ppGeometryShaders[MAX_SHADER_NUM];
		CPDXPixelShader		m_ppPixelShaders[MAX_SHADER_NUM];
		CPDXComputeShader	m_ppComputeShaders[MAX_SHADER_NUM];

		CPDXReflector		m_ppVSReflectors[MAX_SHADER_NUM];
		CPDXReflector		m_ppHSReflectors[MAX_SHADER_NUM];
		CPDXReflector		m_ppDSReflectors[MAX_SHADER_NUM];
		CPDXReflector		m_ppGSReflectors[MAX_SHADER_NUM];
		CPDXReflector		m_ppPSReflectors[MAX_SHADER_NUM];
		CPDXReflector		m_ppCSReflectors[MAX_SHADER_NUM];

		CPDXDevice			m_pDXDevice;
	};

	using upShader = std::unique_ptr<Shader>;
	using spShader = std::shared_ptr<Shader>;
	using vuShader = std::vector<upShader>;
	using vpShader = std::vector<spShader>;
}
