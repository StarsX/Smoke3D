//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXSharedConst.h"

namespace XSDX
{
#ifndef	BASIC_SHADER_STRUCTURES
#define	BASIC_SHADER_STRUCTURES

	// Constant buffer used to send MVP matrices to the vertex shader.
#ifndef	CB_MATRICES
#define CB_MATRICES
	struct alignas(16) CBMatrices
	{
		DirectX::XMMATRIX	m_mWorldViewProj;
		DirectX::XMMATRIX	m_mWorld;
		DirectX::XMMATRIX	m_mNormal;
		DirectX::XMMATRIX	m_mShadow;
#if	TEMPORAL
		DirectX::XMMATRIX	m_mWVPPrev;
#endif
	};
	using LPCBMatrices = std::add_pointer_t<CBMatrices>;
#endif

#ifndef	CB_IMMUTABLE
#define	CB_IMMUTABLE
	struct CBImmutable
	{
		DirectX::XMFLOAT4	m_vViewport;
		DirectX::XMFLOAT4	m_vDirectional;
		DirectX::XMFLOAT4	m_vAmbient;
	};
#endif

#ifndef	CB_GLOBAL
#define	CB_GLOBAL
	struct CBGlobal
	{
		DirectX::XMFLOAT4	m_vLightPt;
		DirectX::XMFLOAT4	m_vEyePtTime;
		DirectX::XMFLOAT4X4	m_mViewProjI;
		DirectX::XMFLOAT4X4	m_mViewProj;
		DirectX::XMFLOAT4X4	m_mView;
		DirectX::XMFLOAT4	m_vLightPtETime;
	};
	using LPCBGlobal = std::add_pointer_t<CBGlobal>;
#endif

#endif

#ifndef	BASIC_SHADER_IDS
#define	BASIC_SHADER_IDS
	// Vertex shaders
	enum CommonVertexShader	: uint32_t
	{
		VS_SCREEN_QUAD
	};

	enum ModelVertexShader	: uint32_t
	{
		VS_BASE_PASS,
		VS_DEPTH,
		VS_SKINNING,
		VS_REFLECT,
		VS_BOUND
	};

	// Pixel shaders
	enum CommonPixelShader	: uint32_t
	{
		PS_DEFERRED_SHADE,
		PS_DEPTH,
		PS_OCCLUSION
	};

	enum ModelPixelShader	: uint32_t
	{
		PS_BASE_PASS,
		PS_DEPTH_MODEL,
		PS_OCCLUSION_MODEL,
		PS_REFLECT,
		PS_BOUND
	};

	enum EnvPixelShader		: uint32_t
	{
		PS_SS_REFLECT,
		PS_SKY,
		PS_WATER,
		PS_RESAMPLE
	};

	enum PostPixelShader	: uint32_t
	{
		PS_POST_PROC,
		PS_TONE_MAP,
		PS_TEMPORAL_AA,
		PS_UNSHARP,
		PS_FXAA
	};

	// Compute shaders
	enum ModelComputeShader : uint32_t
	{
		CS_SKINNING
	};

	enum PostComputeShader	: uint32_t
	{
		CS_LUM_ADAPT
	};

	// Vertex shaders
	static uint8_t			g_uVSScreenQuad	(VS_SCREEN_QUAD);
	static uint8_t			g_uVSBasePass	(VS_BASE_PASS);
	static uint8_t			g_uVSPostproc	(VS_SCREEN_QUAD);
	static uint8_t			g_uVSDepth		(VS_DEPTH);
	static uint8_t			g_uVSSkinning	(VS_SKINNING);
	static uint8_t			g_uVSBound		(VS_BOUND);

	// Pixel shaders
	static uint8_t			g_uPSShade		(PS_DEFERRED_SHADE);
	static uint8_t			g_uPSBasePass	(PS_BASE_PASS);
	static uint8_t			g_uPSSSReflect	(PS_SS_REFLECT);
	static uint8_t			g_uPSPostproc	(PS_POST_PROC);
	static uint8_t			g_uPSDepth		(PS_DEPTH);
	static uint8_t			g_uPSToneMap	(PS_TONE_MAP);
	
	static uint8_t			g_uPSTemporalAA	(PS_TEMPORAL_AA);
	static uint8_t			g_uPSOcclusion	(PS_OCCLUSION);
	static uint8_t			g_uPSResample	(PS_RESAMPLE);
	static uint8_t			g_uPSUnsharp	(PS_UNSHARP);
	static uint8_t			g_uPSFXAA		(PS_FXAA);
	static uint8_t			g_uPSReflect	(PS_REFLECT);
	static uint8_t			g_uPSBound		(PS_BOUND);

	// Compute shaders
	static uint8_t			g_uCSSkinning	(CS_SKINNING);
	static uint8_t			g_uCSLumAdapt	(CS_LUM_ADAPT);
#endif

#ifndef	BASIC_SHADER_SLOTS
#define	BASIC_SHADER_SLOTS
	// Other constant slots
	static const uint8_t	g_uCBBound		(3);
#endif
}
