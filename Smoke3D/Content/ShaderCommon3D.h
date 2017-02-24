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
	static auto			g_uCSDiffuse	(1ui8);
	static auto			g_uCSImpulse	(2ui8);
	static auto			g_uCSDiv		(3ui8);
	static auto			g_uCSPressure	(4ui8);
	static auto			g_uCSProject	(5ui8);
	static auto			g_uCSBound		(6ui8);
	static auto			g_uCSTemporal	(7ui8);
}
