//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Smoke3D.h"

using namespace std;
using namespace Concurrency;
using namespace DirectX;
using namespace DX;
using namespace ShaderIDs;
using namespace XSDX;

using upCDXUTTextHelper = unique_ptr<CDXUTTextHelper>;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager		g_DialogResourceManager;	// manager for shared resources of dialogs
CModelViewerCamera				g_Camera;					// A model viewing camera
//CD3DSettingsDlg				g_D3DSettingsDlg;			// Device settings dialog
//CDXUTDialog					g_HUD;						// manages the 3D   
CDXUTDialog						g_SampleUI;					// dialog for sample specific controls
bool							g_bShowHelp = false;		// If true, it renders the UI control text
bool							g_bShowFPS = false;			// If true, it shows the FPS
bool							g_bViscous = false;
bool							g_bLoadingComplete = false;

upCDXUTTextHelper				g_pTxtHelper;

upFluid3D						g_pFluid;

spShader						g_pShader;
spState							g_pState;

CPDXBuffer						g_pCBImmutable;
CPDXBuffer						g_pCBMatrices;
CPDXBuffer						g_pCBPerFrame;
CPDXBuffer						g_pCBPerObject;

XMFLOAT2						g_vViewport;
XMFLOAT3						g_vMouse;
XMFLOAT4						g_vForceDens;
XMFLOAT3						g_vImLoc = { 0.5f, 0.9f, 0.5f };
XMFLOAT4						g_vLightPt = { 10.0f, 45.0f, -75.0f, 0.0f };

const auto						g_fGravity = 0.0f;
const auto						g_mWorld = XMMatrixScaling(6.4f, 6.4f, 6.4f);

#define DELTA_TIME				0.03f

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLEWARP          4

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void *pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext);

void InitApp();
void RenderText();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w+t", stdout);
	freopen_s(&stream, "CONIN$", "r+t", stdin);
#endif

	// DXUT will create and use the best device
	// that is available on the system depending on which D3D callbacks are set below

	// Set DXUT callbacks
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackMouse(OnMouse, true);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);

	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

	InitApp();
	DXUTInit(true, true, nullptr); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(false, true); // Show the cursor and clip it when in full screen
	DXUTCreateWindow(L"Smoke 3D");
	DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, 1280, 960);
	DXUTMainLoop(); // Enter into the DXUT render loop

#if defined(DEBUG) | defined(_DEBUG)
	FreeConsole();
#endif

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_SampleUI.Init(&g_DialogResourceManager);
	g_SampleUI.SetCallback(OnGUIEvent);
}

//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	pDeviceSettings->d3d11.SyncInterval = 0;

	return true;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove(fElapsedTime);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
	UINT nBackBufferHeight = DXUTGetDXGIBackBufferSurfaceDesc()->Height;

	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(2, 0);
	g_pTxtHelper->SetForegroundColor(Colors::Yellow);
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

	// Draw help
	if (g_bShowHelp)
	{
		g_pTxtHelper->SetInsertionPos(2, nBackBufferHeight - 20 * 4);
		g_pTxtHelper->SetForegroundColor(Colors::Red);
		g_pTxtHelper->DrawTextLine(L"Controls:");

		g_pTxtHelper->SetInsertionPos(20, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Free impulese: Left mouse button\n"
			L"Vertical jit: J\n");

		g_pTxtHelper->SetInsertionPos(285, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Rotate camera: Right mouse button\n"
			L"Zoom camera: Mouse wheel scroll\n");

		g_pTxtHelper->SetInsertionPos(550, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Hide help: F1\n"
			L"Quit: ESC\n");
	}
	else
	{
		g_pTxtHelper->SetForegroundColor(Colors::White);
		g_pTxtHelper->DrawTextLine(L"Press F1 for help");
	}

	g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) return 0;

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) return 0;

	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

//--------------------------------------------------------------------------------------
// Handle mouse events
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void *pUserContext)
{
	static const float fForceScl = 200.0f;
	static bool bStart = true;
	if (bLeftButtonDown)
	{
		const auto mProj = g_Camera.GetProjMatrix();
		const auto mView = g_Camera.GetViewMatrix();
		const auto mViewProj = XMMatrixMultiply(mView, mProj);
		const auto mWorldViewProj = XMMatrixMultiply(g_mWorld, mViewProj);
		const auto mWorldViewProjI = XMMatrixInverse(nullptr, mWorldViewProj);

		const auto vMouse = XMFLOAT2(static_cast<float>(xPos), static_cast<float>(yPos));
		const auto vZero = XMVector3TransformCoord(XMVectorZero(), mViewProj);
		
		auto vPos = XMLoadFloat2(&vMouse) / XMLoadFloat2(&g_vViewport);
		vPos = vPos * 2.0f - XMVectorReplicate(1.0f);
		vPos = XMVectorSetY(vPos, -XMVectorGetY(vPos));
		vPos = XMVectorSetZ(vPos, XMVectorGetZ(vZero));
		vPos = XMVector3TransformCoord(vPos, mWorldViewProjI);
		vPos = XMVectorSetY(vPos, -XMVectorGetY(vPos));

		if (bStart) bStart = false;
		else
		{
			const auto vForce = (vPos - XMLoadFloat3(&g_vMouse)) * fForceScl;
			const auto fStrength = XMVectorGetX(XMVector3Length(vForce));
			if (fStrength > 0.0000001f)
			{
				const auto vForceDir = vForce / fStrength;
				XMStoreFloat4(&g_vForceDens, vForceDir * max(fStrength, 30.0f));
				g_vImLoc.x = min(max(g_vMouse.x * 0.5f + 0.5f, 0.1f), 0.9f);
				g_vImLoc.y = min(max(g_vMouse.y * 0.5f + 0.5f, 0.1f), 0.9f);
				g_vImLoc.z = min(max(g_vMouse.z * 0.5f + 0.5f, 0.1f), 0.9f);
				g_vForceDens.y += g_fGravity;
				g_vForceDens.w = 2.5f;
			}
		}
		XMStoreFloat3(&g_vMouse, vPos);
	}
	else
	{
		g_vForceDens = XMFLOAT4(0.0f, g_fGravity, 0.0f, 0.0f);
		g_vImLoc = XMFLOAT3(0.5f, 0.9f, 0.5f);
		bStart = true;
	}
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if (bKeyDown) {
		switch (nChar) {
		case VK_F1:
			g_bShowHelp = !g_bShowHelp; break;
		case VK_F2:
			g_bShowFPS = !g_bShowFPS; break;
		case 'V':
			g_bViscous = !g_bViscous; break;
		case 'J':
			g_vForceDens = XMFLOAT4(0.0f, g_fGravity - 30.0f, 0.0f, 2.5f);
			break;
		}
	}
	else {
		switch (nChar) {
		case 'J':
			g_vForceDens = XMFLOAT4(0.0f, g_fGravity, 0.0f, 0.0f);
			break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr;

	ID3D11DeviceContext *pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	g_pTxtHelper = make_unique<CDXUTTextHelper>(pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15);

	// Load shaders asynchronously.
	g_pShader = make_shared<Shader>(pd3dDevice);
	g_pState = make_shared<State>(pd3dDevice);

	g_pFluid = make_unique<Fluid3D>(pd3dDevice, g_pShader, g_pState);
	g_pFluid->Init(64u, 64u, 64u);

	auto loadVSTask = g_pShader->CreateVertexShader(L"VSRayCast.cso", g_uVSRayCast);
	auto loadPSTask = g_pShader->CreatePixelShader(L"PSRayCast.cso", g_uPSRayCast);
	auto loadCSTask = g_pShader->CreateComputeShader(L"CSAdvect3D.cso", g_uCSAdvect);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"CSDiffuse3D.cso", g_uCSDiffuse);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"CSImpulse3D.cso", g_uCSImpulse);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"CSDivergence3D.cso", g_uCSDiv);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"CSPressure3D.cso", g_uCSPressure);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"CSProject3D.cso", g_uCSProject);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"CSBound3D.cso", g_uCSBound);

	const auto createShaderTask = loadVSTask && loadPSTask && loadCSTask;

	const auto createConstTask = create_task([pd3dDevice, pd3dImmediateContext]() {
		// Setup constant buffers
		auto desc = CD3D11_BUFFER_DESC(sizeof(CBMatrices), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(pd3dDevice->CreateBuffer(&desc, nullptr, &g_pCBMatrices));

		desc.ByteWidth = sizeof(XMVECTOR[2]);
		ThrowIfFailed(pd3dDevice->CreateBuffer(&desc, nullptr, &g_pCBPerObject));

		desc.ByteWidth = sizeof(CBImmutable);
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		auto im = CBImmutable();
		im.m_vDirectional = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.6f);
		im.m_vAmbient = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.08f);
		const auto dsd = D3D11_SUBRESOURCE_DATA{ &im, 0, 0 };
		ThrowIfFailed(pd3dDevice->CreateBuffer(&desc, &dsd, &g_pCBImmutable));

		pd3dImmediateContext->PSSetConstantBuffers(0u, 1u, g_pCBImmutable.GetAddressOf());
	});

	// Once the cube is loaded, the object is ready to be rendered.
	(createShaderTask && createConstTask).then([]() {
		g_pShader->ReleaseShaderBuffers();

		// View
		// Setup the camera's view parameters
		const auto vAngV = XM_PI / 6.0f;
		const auto vAngH = XM_PI / 4.0f;
		const auto vLookAtPt = XMFLOAT4(0.0f, 0.0f, 0.0f, 32.0f);
		const auto fdy = vLookAtPt.w * sin(vAngV);
		const auto fr = vLookAtPt.w * cos(vAngV);
		const auto fdx = fr * cos(vAngH);
		const auto fdz = -fr * sin(vAngH);
		auto vEyePt = XMVectorSet(vLookAtPt.x + fdx, vLookAtPt.y + fdy, vLookAtPt.z + fdz, 0.0f);
		g_Camera.SetViewParams(vEyePt, XMLoadFloat4(&vLookAtPt));

		g_bLoadingComplete = true;
	});

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	// Setup the camera's projection parameters
	auto fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(XM_PIDIV4, fAspectRatio, 1.0f, 1000.0f);
	g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	g_Camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON);

	// Initialize window size dependent resources
	// Viewport clipping
	auto iVpNum = 1u;
	D3D11_VIEWPORT viewport;
	DXUTGetD3D11DeviceContext()->RSGetViewports(&iVpNum, &viewport);

	// Set window size dependent constants
	g_vViewport = XMFLOAT2(viewport.Width, viewport.Height);

	//g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	//g_HUD.SetSize(170, 170);
	g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_SampleUI.SetSize(170, 300);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!g_bLoadingComplete) return;

	// Set render targets to the screen.
	ID3D11RenderTargetView *pRTVs[] = { DXUTGetD3D11RenderTargetView() };
	ID3D11DepthStencilView *pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView(pRTVs[0], DirectX::Colors::CornflowerBlue);
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, pDSV);

	// Prepare the constant buffer to send it to the graphics device.
	// Get the projection & view matrix from the camera class
	const auto mWorldI = XMMatrixInverse(nullptr, g_mWorld);
	const auto mProj = g_Camera.GetProjMatrix();
	const auto mView = g_Camera.GetViewMatrix();
	const auto mViewProj = XMMatrixMultiply(mView, mProj);
	const auto mWorldViewProj = XMMatrixMultiply(g_mWorld, mViewProj);

	// Set VS constants
	const auto cbMatrices = CBMatrices{ XMMatrixTranspose(mWorldViewProj) };
	pd3dImmediateContext->UpdateSubresource(g_pCBMatrices.Get(), 0u, nullptr, &cbMatrices, 0u, 0u);
	pd3dImmediateContext->VSSetConstantBuffers(0u, 1u, g_pCBMatrices.GetAddressOf());

	// Set PS constants
	auto cbPerObject = array<XMVECTOR, 2>();
	cbPerObject[0] = XMVector3TransformCoord(XMLoadFloat4(&g_vLightPt), mWorldI);
	cbPerObject[1] = XMVector3TransformCoord(g_Camera.GetEyePt(), mWorldI);
	pd3dImmediateContext->UpdateSubresource(g_pCBPerObject.Get(), 0u, nullptr, cbPerObject.data(), 0u, 0u);
	pd3dImmediateContext->PSSetConstantBuffers(2u, 1u, g_pCBPerObject.GetAddressOf());

	// Render
	g_pFluid->Simulate(min(fElapsedTime, DELTA_TIME), g_vForceDens, g_vImLoc, g_bViscous ? 10ui8 : 0ui8);
	g_pFluid->Render();

	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	if (g_bShowFPS) {
		g_SampleUI.OnRender(fElapsedTime);
		RenderText();
	}
	DXUT_EndPerfEvent();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	g_bLoadingComplete = false;
	g_pCBPerObject.Reset();
	g_pCBPerFrame.Reset();
	g_pCBMatrices.Reset();
	g_pCBImmutable.Reset();
	g_pTxtHelper.reset();
	g_pFluid.reset();
	g_pState.reset();
	g_pShader.reset();
}
