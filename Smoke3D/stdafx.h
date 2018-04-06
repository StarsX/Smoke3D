// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// C RunTime Header Files
#include <wrl/client.h>
#include <fstream>

//#define DXUT_AUTOLIB
#include <DXUT.h>
#include <DXUTCamera.h>
#include <SDKmisc.h>
#include <SDKmesh.h>

#include <DXUTSettingsDlg.h>
//#include <DXUTmisc.h>

#pragma comment(lib, "DXUT.lib")
#pragma comment(lib, "DXUTOpt.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "usp10.lib")

#define WIN32_DESKTOP_DXUT
#if defined(DEBUG) | defined(_DEBUG)
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

// TODO: reference additional headers your program requires here
