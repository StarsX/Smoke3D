//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

namespace ShaderIDs
{
	// Compute shaders
	enum FluidComputeShader : uint32_t
	{
		CS_ADVECT,
		CS_MAC_CORMACK,
		CS_DIFFUSE,
		CS_IMPULSE,
		CS_DIVERGENCE,
		CS_PRESSURE,
		CS_PROJECT,
		CS_BOUND,
		CS_TEMPORAL,
		CS_RAYCAST
	};

	// Vertex shaders
	static auto			g_uVSRayCast	(0ui8);

	// Pixel shaders
	static auto			g_uPSRayCast	(0ui8);

	// Compute shaders
	static auto			g_uCSAdvect		(CS_ADVECT);
	static auto			g_uCSMacCormack	(CS_MAC_CORMACK);
	static auto			g_uCSDiffuse	(CS_DIFFUSE);
	static auto			g_uCSImpulse	(CS_IMPULSE);
	static auto			g_uCSDivergence	(CS_DIVERGENCE);
	static auto			g_uCSPressure	(CS_PRESSURE);
	static auto			g_uCSProject	(CS_PROJECT);
	static auto			g_uCSBound		(CS_BOUND);
	static auto			g_uCSTemporal	(CS_TEMPORAL);
	static auto			g_uCSRayCast	(CS_RAYCAST);
}
