//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

namespace ShaderIDs
{
	// Vertex shaders
	static auto			g_uVSRayCast	(0ui8);

	// Pixel shaders
	static auto			g_uPSRayCast	(0ui8);

	// Compute shaders
	static auto			g_uCSAdvect		(0ui8);
	static auto			g_uCSMacCormack	(1ui8);
	static auto			g_uCSDiffuse	(2ui8);
	static auto			g_uCSImpulse	(3ui8);
	static auto			g_uCSDiv		(4ui8);
	static auto			g_uCSPressure	(5ui8);
	static auto			g_uCSProject	(6ui8);
	static auto			g_uCSBound		(7ui8);
	static auto			g_uCSTemporal	(8ui8);
}
