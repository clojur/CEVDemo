#include	"stdafx.h"
#include	<CryMath/cry_geo.h>
#include	<CryRenderer/IRenderAuxGeom.h>

#include	"CCryEngineTestViewport.h"

#include	<CryCore/Platform/platform_impl.inl>
#include	<CryCore/Platform/CryLibrary.h>
#include	<CryString/StringUtils.h>

#include	<Cry3DEngine/I3DEngine.h>
#include	<CryMovie/IMovieSystem.h>
#include	<Cry3DEngine/ITimeOfDay.h>
#include	<CryGame/IGameFramework.h>

static void GameSystemAuthCheckFunction(void* data)
{
	// src and trg can be the same pointer (in place encryption)
	// len must be in bytes and must be multiple of 8 byts (64bits).
	// key is 128bit:  int key[4] = {n1,n2,n3,n4};
	// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
#define TEA_ENCODE(src, trg, len, key) {                                                                                      \
  unsigned int* v = (src), * w = (trg), * k = (key), nlen = (len) >> 3;                                                       \
  unsigned int delta = 0x9E3779B9, a = k[0], b = k[1], c = k[2], d = k[3];                                                    \
  while (nlen--) {                                                                                                            \
    unsigned int y = v[0], z = v[1], n = 32, sum = 0;                                                                         \
    while (n-- > 0) { sum += delta; y += (z << 4) + a ^ z + sum ^ (z >> 5) + b; z += (y << 4) + c ^ y + sum ^ (y >> 5) + d; } \
    w[0] = y; w[1] = z; v += 2, w += 2; }                                                                                     \
}

	// src and trg can be the same pointer (in place decryption)
	// len must be in bytes and must be multiple of 8 byts (64bits).
	// key is 128bit: int key[4] = {n1,n2,n3,n4};
	// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
#define TEA_DECODE(src, trg, len, key) {                                                                                      \
  unsigned int* v = (src), * w = (trg), * k = (key), nlen = (len) >> 3;                                                       \
  unsigned int delta = 0x9E3779B9, a = k[0], b = k[1], c = k[2], d = k[3];                                                    \
  while (nlen--) {                                                                                                            \
    unsigned int y = v[0], z = v[1], sum = 0xC6EF3720, n = 32;                                                                \
    while (n-- > 0) { z -= (y << 4) + c ^ y + sum ^ (y >> 5) + d; y -= (z << 4) + a ^ z + sum ^ (z >> 5) + b; sum -= delta; } \
    w[0] = y; w[1] = z; v += 2, w += 2; }                                                                                     \
}

	// Data assumed to be 32 bytes.
	int key1[4] = { 1178362782, 223786232, 371615531, 90884141 };
	TEA_DECODE((unsigned int*)data, (unsigned int*)data, 32, (unsigned int*)key1);
	int key2[4] = { 89158165, 1389745433, 971685123, 785741042 };
	TEA_ENCODE((unsigned int*)data, (unsigned int*)data, 32, (unsigned int*)key2);
}

using namespace YJGLSJ;

 CCryEngineTestViewport* CCryEngineTestViewport::s_instance =nullptr;
 void* CCryEngineTestViewport::m_currentContextWnd = 0;
CCryEngineTestViewport::CCryEngineTestViewport(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
	: m_hInstance(hInstance)
	, m_hPrevInstance(hPrevInstance)
	, m_lpCmdLine(lpCmdLine)
	, m_nCmdShow(nCmdShow)
	, m_bRenderContextCreated(false)
	, m_bRenderStats(true)
	, m_bEmptyScene(true)
{
	m_currentResolution.width = m_currentResolution.height = 0;
}


CCryEngineTestViewport::~CCryEngineTestViewport()
{
}

void CCryEngineTestViewport::YJGLSJCreateWindow(int x, int y, int width, int height, const char * windowTitle)
{
	m_currentResolution.x = x;
	m_currentResolution.y = y;
	m_currentResolution.width= width;
	m_currentResolution.height =height;
	m_cszTitle = windowTitle;

	InitializeWindow();
	s_instance = this;

	m_pSystem = gEnv->pSystem;
	m_pRenderer = gEnv->pRenderer;
	m_pEngine = gEnv->p3DEngine;
}

bool YJGLSJ::CCryEngineTestViewport::InitEngineSystem()
{
	/*初始化CE系统*/
	SSystemInitParams startupParams;

	startupParams.bEditor = true;
	startupParams.bPreview = false;

	startupParams.hWnd = nullptr;
	startupParams.sLogFileName = "UI.log";
	startupParams.pUserCallback = nullptr;
	char BinDir[256] = {'\0'};
	sprintf(startupParams.szBinariesDir, "E:\\3DEngine\\CryEngine\\CRYENGINE\\bin\\win_x64");

	const char* cmdLine = GetCommandLineA();
	cry_strcpy(startupParams.szSystemCmdLine, cmdLine);

	startupParams.pCheckFunc = &GameSystemAuthCheckFunction;

	startupParams.bExecuteCommandLine = false;

	if (!CryInitializeEngine(startupParams, true))
		return false;

	ISystem*	pSystem = startupParams.pSystem;
	if (pSystem)
		gEnv = pSystem->GetGlobalEnvironment();

	IRenderer*	pRenderer = pSystem->GetIRenderer();
	I3DEngine*	pEngine = pSystem->GetI3DEngine();

	if (gEnv
		&& gEnv->p3DEngine
		&& gEnv->p3DEngine->GetTimeOfDay())
	{
		gEnv->p3DEngine->GetTimeOfDay()->BeginEditMode();
	}

	if (gEnv && gEnv->pMovieSystem)
	{
		gEnv->pMovieSystem->EnablePhysicsEvents(false);
	}

	return true;
}

bool YJGLSJ::CCryEngineTestViewport::StartProjectContext()
{
	gEnv->bServer = true;
	gEnv->bMultiplayer = false;
	gEnv->SetIsClient(true);

	if (gEnv->pGameFramework->StartedGameContext())
		return true;

	SGameContextParams ctx;

	SGameStartParams gameParams;
	gameParams.flags = eGSF_Server
		| eGSF_NoSpawnPlayer
		| eGSF_Client
		| eGSF_NoLevelLoading
		| eGSF_BlockingClientConnect
		| eGSF_NoGameRules
		| eGSF_NoQueries;

	gameParams.flags |= eGSF_LocalOnly;

	gameParams.connectionString = "";
	gameParams.hostname = "localhost";
	gameParams.port = 0xed17;
	gameParams.pContextParams = &ctx;
	gameParams.maxPlayers = 1;

	if (gEnv->pGameFramework->StartGameContext(&gameParams))
	{
		return true;
	}

	return false;
}

CCryEngineTestViewport::SPreviousContext YJGLSJ::CCryEngineTestViewport::SetCurrentContext()
{
	SPreviousContext x;
	m_currentContextWnd = GetSafeHwnd();

	GetISystem()->SetViewCamera(CCamera(m_Camera));

	return x;
}

void YJGLSJ::CCryEngineTestViewport::SetISystem(ISystem * pSystem)
{
	m_pSystem = pSystem;
}

void YJGLSJ::CCryEngineTestViewport::SetIRenderer(IRenderer * pRenderer)
{
	m_pRenderer = pRenderer;
}

void YJGLSJ::CCryEngineTestViewport::SetIEngine(I3DEngine * pEngine)
{
	m_pEngine = pEngine;
}

bool YJGLSJ::CCryEngineTestViewport::CreateRenderContext(HWND hWnd, IRenderer::EViewportType viewportType)
{
	// Create context.
	if (hWnd && m_pRenderer && !m_bRenderContextCreated)
	{
		IRenderer::SDisplayContextDescription desc;

		desc.handle = hWnd;
		desc.type = viewportType;
		desc.clearColor = ColorF(0.4f, 0.4f, 0.4f, 1.0f);
		desc.renderFlags = FRT_CLEAR | FRT_OVERLAY_DEPTH;
		desc.superSamplingFactor.x = 1;
		desc.superSamplingFactor.y = 1;
		desc.screenResolution.x = m_currentResolution.width;
		desc.screenResolution.y = m_currentResolution.height;

		m_displayContextKey = m_pRenderer->CreateSwapChainBackedContext(desc);
		m_bRenderContextCreated = true;

		// Make main context current.
		SetCurrentContext();
		return true;
	}

	return false;
}

void YJGLSJ::CCryEngineTestViewport::InitDisplayContext(SDisplayContextKey displayContextKey)
{
	CRY_PROFILE_FUNCTION(PROFILE_EDITOR);

	// Draw all objects.
	DisplayContext& dctx = m_displayContext;
	dctx.SetDisplayContext(displayContextKey, IRenderer::eViewportType_Default);
	dctx.SetView(this);
	dctx.SetCamera(&m_Camera);
	dctx.renderer = m_pRenderer;
	dctx.engine = m_pEngine;
	dctx.box.min = Vec3(-100000, -100000, -100000);
	dctx.box.max = Vec3(100000, 100000, 100000);
	dctx.flags = 0;

	dctx.flags |= DISPLAY_HIDENAMES;
	//dctx.flags |= DISPLAY_LINKS;
	//dctx.flags |= DISPLAY_DEGRADATED;
	//dctx.flags |= DISPLAY_BBOX;
	//dctx.flags |= DISPLAY_TRACKS;
	//dctx.flags |= DISPLAY_TRACKTICKS;
	//dctx.flags |= DISPLAY_WORLDSPACEAXIS;
}

void YJGLSJ::CCryEngineTestViewport::OnRender()
{
	if (!m_pRenderer || !m_pEngine)
		return;

	////正常渲染
	//uintptr_t displayContextHandle = reinterpret_cast<uintptr_t>(m_hWnd);
	m_viewTM = m_Camera.GetMatrix();
	if (!m_bRenderContextCreated)
	{
		if (!CreateRenderContext((HWND)GetSafeHwnd(), IRenderer::EViewportType::eViewportType_Secondary))
			return;

		// Configures Aux to draw to the current display-context
		InitDisplayContext(m_displayContextKey);

		//m_pModel= gEnv->p3DEngine->LoadStatObj("G:\\CryEngine\\CEProject\\test\\Assets\\objects\\sphere.cgf", false);
		//m_pMaterial = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("G:\\CryEngine\\CEProject\\test\\Assets\\Materials\\generic\\metal\\gold.mtl");
	}

	//RECT  rcClient;
	//::GetClientRect((HWND)m_hWnd, &rcClient);
	//if (rcClient.right < 16 || rcClient.bottom < 16)
	//	return;


	// 3D engine stats
	//m_pSystem->RenderBegin(m_displayContextKey);

	//bool bRenderStats = m_bRenderStats;

	//float fNearZ = 0.25;
	//float fFarZ = m_Camera.GetFarPlane();
	//float fov = 1.04712;// 60°
	//float viewportAspectRatio = float(m_currentResolution.width) / m_currentResolution.height;
	//float fMaxMaxViewDistance = m_pEngine->GetMaxViewDistance();
	//m_Camera.SetFrustum(m_currentResolution.width, m_currentResolution.height, fNearZ, fMaxMaxViewDistance);
	//m_pSystem->RenderEnd(bRenderStats);
	//const int32 renderFlags =0;
	//SRenderingPassInfo passInfo = SRenderingPassInfo::CreateGeneralPassRenderingInfo(m_Camera, SRenderingPassInfo::DEFAULT_FLAGS,false, m_displayContextKey);
	//m_pEngine->RenderWorld(renderFlags | SHDF_ALLOW_AO | SHDF_ALLOWPOSTPROCESS | SHDF_ALLOW_WATER | SHDF_ALLOWHDR | SHDF_ZPASS, passInfo, __FUNCTION__);

	//m_displayContext.SetState(e_Mode3D | e_AlphaBlended | e_FillModeSolid | e_CullModeBack | e_DepthWriteOn | e_DepthTestOn);
	//DrawAxis();

	////if (ITestSystem* pTestSystem = m_pSystem->GetITestSystem())
	////	pTestSystem->AfterRender();

	//m_displayContext.Flush2D();

	//// 3D engine stats
	//CCamera CurCamera = m_pSystem->GetViewCamera();
	//m_pSystem->SetViewCamera(m_Camera);

	//m_pSystem->RenderEnd(bRenderStats);
	//m_pSystem->SetViewCamera(CurCamera);

	//m_pRenderer->EndFrame();
	//m_pRenderer->EnableSwapBuffers(true);
	//gEnv->nMainFrameID++;


	//2D渲染
	gEnv->pSystem->GetIProfileSystem()->StartFrame();
	if (!gEnv->pRenderer)
		return;


	CScopedWireFrameMode scopedWireFrame(gEnv->pRenderer, R_SOLID_MODE);

	//////////////////////////////////////////////////////////////////////////
	// 2D Mode.
	//////////////////////////////////////////////////////////////////////////
	if (m_currentResolution.height&&m_currentResolution.width)
	{
		gEnv->pRenderer->GetIRenderAuxGeom()->SetOrthographicProjection(true, 0.0f, m_currentResolution.width, m_currentResolution.height, 0.0f);

		//////////////////////////////////////////////////////////////////////////
		// Draw viewport elements here.
		//////////////////////////////////////////////////////////////////////////
		// Calc world bounding box for objects rendering.
		Vec3 org;
		AABB box;
		box.Reset();
		box.Add(Vec3(0,0,0));
		box.Add(Vec3(1000,1000,1000));
		// Draw all objects.
		DisplayContext& dc = m_displayContext;

		dc.view = this;
		dc.renderer = gEnv->pRenderer;
		dc.engine = gEnv->p3DEngine;
		dc.flags = DISPLAY_2D;
		dc.box = box;
		m_Camera = gEnv->pSystem->GetViewCamera();
		// Should be setting orthogonal camera
		m_Camera.SetFrustum(m_currentResolution.width,m_currentResolution.height, m_Camera.GetFov(), m_Camera.GetNearPlane(), m_Camera.GetFarPlane());
		dc.SetCamera(&m_Camera);
		dc.SetDisplayContext(m_displayContextKey);
		dc.flags |= DISPLAY_HIDENAMES;

		// Render
		gEnv->pRenderer->BeginFrame(m_displayContextKey);

		// TODO: BeginFrame/EndFrame calls can be droped and replaced by RT_AuxRender
		auto oldCamera = gEnv->pRenderer->GetIRenderAuxGeom()->GetCamera();
		gEnv->pRenderer->UpdateAuxDefaultCamera(m_Camera);

		SRenderingPassInfo passInfo = SRenderingPassInfo::CreateGeneralPassRenderingInfo(gEnv->pSystem->GetViewCamera(), SRenderingPassInfo::DEFAULT_FLAGS, false, m_displayContextKey);

		gEnv->pRenderer->EF_StartEf(passInfo);
		dc.SetState(e_Mode3D | e_AlphaBlended | e_FillModeSolid | e_CullModeBack | e_DepthWriteOff | e_DepthTestOn);
		//Draw(CObjectRenderHelper{ dc, passInfo });
		gEnv->pRenderer->EF_EndEf3D(SHDF_ALLOWHDR | SHDF_SECONDARY_VIEWPORT, -1, -1, passInfo);

		// Return back from 2D mode.
		gEnv->pRenderer->GetIRenderAuxGeom()->SetOrthographicProjection(false);

		gEnv->pRenderer->RenderDebug(false);

		//ProcessRenderListeners(m_displayContext);

		gEnv->pRenderer->EndFrame();
	}

	gEnv->pSystem->GetIProfileSystem()->EndFrame();
}

void YJGLSJ::CCryEngineTestViewport::DrawAxis()
{

	Vec3 colX(1, 0, 0), colY(0, 1, 0), colZ(0, 0, 1), colW(1, 1, 1);
	Vec3 pos(50, 50, 0.1f); // Bottom-left corner

	Vec3 posInWorld(0, 0, 0);
	if (!m_Camera.Unproject(pos, posInWorld))
		return;

	auto GetScreenScaleFactor = [this](const Vec3& worldPoint)->float
	{
		Vec3 pointVec = worldPoint - m_Camera.GetPosition();
		// Only keep the projected part of the vector
		float dist = pointVec.Dot(m_Camera.GetViewdir());
		if (dist < m_Camera.GetNearPlane())
			dist = m_Camera.GetNearPlane();
		return dist;
	};

	float screenScale = GetScreenScaleFactor(posInWorld);
	float length = 0.03f * screenScale;
	float arrowSize = 0.02f * screenScale;
	float textSize = 1.1f;

	Vec3 x(length, 0, 0);
	Vec3 y(0, length, 0);
	Vec3 z(0, 0, length);

	DisplayContext& dc = m_displayContext;
	int prevRState = dc.GetState();

	dc.DepthWriteOff();
	dc.DepthTestOff();
	dc.CullOff();
	dc.SetLineWidth(1);

	dc.SetColor(colX);
	dc.DrawLine(posInWorld, posInWorld + x);
	dc.DrawArrow(posInWorld + x * 0.9f, posInWorld + x, arrowSize);
	dc.SetColor(colY);
	dc.DrawLine(posInWorld, posInWorld + y);
	dc.DrawArrow(posInWorld + y * 0.9f, posInWorld + y, arrowSize);
	dc.SetColor(colZ);
	dc.DrawLine(posInWorld, posInWorld + z);
	dc.DrawArrow(posInWorld + z * 0.9f, posInWorld + z, arrowSize);

	//dc.SetColor(colW);
	//dc.DrawTextLabel(posInWorld + x, textSize, "x");
	//dc.DrawTextLabel(posInWorld + y, textSize, "y");
	//dc.DrawTextLabel(posInWorld + z, textSize, "z");

	dc.DepthWriteOn();
	dc.DepthTestOn();
	dc.CullOn();
	dc.SetState(prevRState);
}

void YJGLSJ::CCryEngineTestViewport::DrawModel(const SRenderingPassInfo & passInfo)
{
	IRenderAuxGeom* pAuxGeom = m_pRenderer->GetIRenderAuxGeom();
	m_pRenderer->EF_StartEf(passInfo);

	//f32 fDistance =m_Camera.GetViewMatrix().GetTranslation().GetLength();
	//SRendParams rp;
	//rp.fDistance = fDistance;

	//Matrix34 tm;
	//tm.SetIdentity();
	//rp.pMatrix = &tm;
	//rp.pPrevMatrix = &tm;

	//Vec3 vAmbient;

	//rp.AmbientColor.r = 0.7;
	//rp.AmbientColor.g = 0.7;
	//rp.AmbientColor.b = 0.7;
	//rp.AmbientColor.a = 1;

	//rp.dwFObjFlags = 0;
	//rp.dwFObjFlags |= FOB_TRANS_MASK;
	//
	//rp.pMaterial = m_pMaterial;

	//m_pModel->Render(rp, passInfo);
	m_pRenderer->EF_EndEf3D(SHDF_ALLOWHDR | SHDF_SECONDARY_VIEWPORT, -1, -1, passInfo);
}

void YJGLSJ::CCryEngineTestViewport::Run()
{
	MSG msg;
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			OnRender();
			Sleep(10);
		}
	}
}

bool YJGLSJ::CCryEngineTestViewport::InitializeWindow()
{
	/*创建窗口*/
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = DefProc;
	windowClass.hInstance = m_hInstance;
	windowClass.lpszClassName = "CCryEngineTestViewport";
	if (!::RegisterClassEx(&windowClass))
	{
		return false;
	}

	HWND windowHandle = CreateWindowEx(NULL, "CCryEngineTestViewport", m_cszTitle,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		m_currentResolution.x,
		m_currentResolution.y,
		m_currentResolution.width,    // some random values for now.  
		m_currentResolution.height,    // we will come back to these soon. 
		nullptr,
		nullptr,
		m_hInstance,
		this);

	m_hWnd = windowHandle;
	if (!windowHandle)
	{
		return false;
	}
	

	::ShowWindow(m_hWnd, SW_NORMAL);
	::UpdateWindow(m_hWnd);
	return TRUE;
}

void CCryEngineTestViewport::OnSize(int width, int height)
{
}

void CCryEngineTestViewport::OnMove(int x, int y)
{
}

void CCryEngineTestViewport::DestoryWindow()
{
}

LRESULT YJGLSJ::CCryEngineTestViewport::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE: {
		PostQuitMessage(0);
		break;
	}
	case WM_SIZE:
	{
		int a = 10;
		break;
	}
	default:
	{
		break;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT YJGLSJ::CCryEngineTestViewport::DefProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (s_instance)
		return s_instance->WindowProc(hWnd,uMsg,wParam,lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

CCryEngineTestViewport * YJGLSJ::CCryEngineTestViewport::GetInstance()
{
	return s_instance;
}
