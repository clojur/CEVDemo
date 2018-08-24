#include "stdafx.h"
#include "CryMath/cry_geo.h"
#include "CRenderViewport.h"

#include <CryRenderer/IRenderAuxGeom.h>

void* CRenderViewport::m_currentContextWnd = 0;
CRenderViewport::CRenderViewport()
	: m_bRenderContextCreated(false)
	, m_bRenderStats(true)
	, m_bEmptyScene(true)
	, m_pRenderer(nullptr)
	, m_p3DEngine(nullptr)
	, m_hWnd(nullptr)
{

}


CRenderViewport::~CRenderViewport()
{
}

void CRenderViewport::SetResolution(int x, int y)
{
}

void CRenderViewport::GetResolution(int & x, int & y)
{
}

void CRenderViewport::DrawAxis()
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

	dc.SetColor(colW);
	//dc.DrawTextLabel(posInWorld + x, textSize, "x");
	//dc.DrawTextLabel(posInWorld + y, textSize, "y");
	//dc.DrawTextLabel(posInWorld + z, textSize, "z");

	dc.DepthWriteOn();
	dc.DepthTestOn();
	dc.CullOn();
	dc.SetState(prevRState);
}

bool CRenderViewport::CreateRenderContext(uintptr_t displayContextHandle, IRenderer::EViewportType viewportType)
{
	if (displayContextHandle&&m_pRenderer && !m_bRenderContextCreated)
	{
		IRenderer::SDisplayContextDescription desc;

		desc.handle = displayContextHandle;
		desc.type = viewportType;
		desc.clearColor = ColorF(0.4f,0.4f,0.4f,1.0f);
		desc.renderFlags = FRT_CLEAR | FRT_OVERLAY_DEPTH;
		desc.superSamplingFactor.x = 1;
		desc.superSamplingFactor.y = 1;
		desc.screenResolution.x = m_currentResolution.width;
		desc.screenResolution.y = m_currentResolution.height;

		m_pRenderer->CreateContext(desc);
		m_bRenderContextCreated = true;

		SetCurrentContext();
		return true;
	}
	return false;
}

void CRenderViewport::InitDisplayContext(uintptr_t displayContextHandle)
{
	// Draw all objects.
	DisplayContext& dctx = m_displayContext;
	dctx.SetDisplayContext(displayContextHandle, IRenderer::eViewportType_Default);
	//dctx.SetView(this);
	dctx.SetCamera(&m_Camera);
	dctx.renderer = m_pRenderer;
	dctx.engine = m_p3DEngine;
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

void CRenderViewport::Update()
{
	if (!m_pRenderer || !m_p3DEngine)
		return;

	//视口口无效直接返回
	//if (!IsWindowVisible())
	//	return;

	//// 窗口获取到焦点后才可以更新
	//if (IsFocused())
	//	m_bUpdateViewport = true;

	//视口管理器有渲染视口才执行更新
	//if (!m_isOnPaint && GetIEditor()->GetViewportManager()->GetNumberOfGameViewports() > 1 && GetType() == ET_ViewportCamera)
	//{
	//	if (m_pPrimaryViewport != this)
	//	{
	//		if (IsFocused()) // If this is the focused window, set primary viewport.
	//			m_pPrimaryViewport = this;
	//		else if (!m_bUpdateViewport) // Skip this viewport.
	//			return;
	//	}
	//}

	//窗口获取焦点后才执行鼠标键盘操作
	//if (IsFocused())
	//{
	//	ProcessMouse();
	//	ProcessKeys();
	//}

	//如果是漫游模式
	//if (IsInRomanMode())
	//{
	//	....
	//	return;
	//}

	//正常渲染
	uintptr_t displayContextHandle = reinterpret_cast<uintptr_t>(GetSafeHwnd());
	m_viewTM = m_Camera.GetMatrix();
	if (!m_bRenderContextCreated)
	{
		if (!CreateRenderContext(displayContextHandle))
			return;
	}

	RECT  rcClient;
	::GetClientRect((HWND)GetSafeHwnd(), &rcClient);
	if (rcClient.right < 16 || rcClient.bottom < 16)
		return;
	m_currentContextWnd = 0;

	SScopedCurrentContext context(this);

	// Configures Aux to draw to the current display-context
	InitDisplayContext(displayContextHandle);

	// 3D engine stats
	m_pSystem->RenderBegin(displayContextHandle);

	bool bRenderStats = m_bRenderStats;

	if (m_bEmptyScene)
	{
		m_bEmptyScene = false;

		bRenderStats = false;
		DrawBackground();
		DrawAxis();
	}
	else
	{
		OnRender();
	}

	m_displayContext.Flush2D();

	CCamera CurCamera = m_pSystem->GetViewCamera();
	m_pSystem->SetViewCamera(m_Camera);

	m_pSystem->RenderEnd(bRenderStats);
	m_pSystem->SetViewCamera(CurCamera);

	m_pRenderer->EnableSwapBuffers(true);
	//Sleep(10);
}

void CRenderViewport::OnRender()
{
	

}

void CRenderViewport::OnResize()
{

	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);

	int width = rcClient.right- rcClient.left;
	int height = rcClient.bottom-rcClient.top;

	if (m_pRenderer)
	{
		m_pRenderer->ResizeContext(m_displayContext.GetDisplayContextHandle(), width, height);
	}

	m_currentResolution.width = width;
	m_currentResolution.height = height;

	m_pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_RESIZE, m_currentResolution.width, m_currentResolution.height);
}

void CRenderViewport::DrawBackground()
{
	DisplayContext& dc = m_displayContext;

	//if (!GetIEditor()->IsHelpersDisplayed())      // show gradient bg only if draw helpers are activated
	//	return;

	const float startX = 0.0f;
	const float endX = m_Camera.GetViewSurfaceX();
	const float startY = 0.0f;
	const float endY = m_Camera.GetViewSurfaceZ();

	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);

	m_Camera.SetFrustum(endX, endY, m_Camera.GetFov(), 0.02f, 10000, m_Camera.GetPixelAspectRatio());
	int w = rcClient.right - rcClient.left;
	int h = rcClient.bottom - rcClient.top;
	m_Camera.SetFrustum(w, h, DEG2RAD(90), 0.0101f, 10000.0f);

	Vec3 bottomLeft(startX, startY, 0);
	Vec3 bottomRight(endX, startY, 0);
	Vec3 topLeft(startX, endY, 0);
	Vec3 topRight(endX, endY, 0);

	COLORREF topColor = ::GetSysColor(COLOR_MENU);
	COLORREF bottomColor = ::GetSysColor(COLOR_GRAYTEXT);

	//ColorB secondC(GetRValue(topColor), GetGValue(topColor), GetBValue(topColor), 255.0f);
	//ColorB firstC(GetRValue(bottomColor), GetGValue(bottomColor), GetBValue(bottomColor), 255.0f);

	ColorB secondC(25.0f,25.0f,0.0f, 255.0f);
	ColorB firstC(255.0, 105.0, 0.0f, 255.0f);
	dc.SetState(e_Mode2D | e_AlphaBlended | e_FillModeSolid | e_CullModeBack | e_DepthWriteOn | e_DepthTestOn);
	dc.DrawQuadGradient(bottomRight, bottomLeft, topLeft, topRight, secondC, firstC);
}

LRESULT CRenderViewport::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		break;
	}
	case WM_SIZE:
	{
		OnResize();
		break;
	}
	default:
	{
		break;
	}
	}

	//a pass-through for now. We will return to this callback 
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CRenderViewport::SPreviousContext CRenderViewport::SetCurrentContext()
{
	SPreviousContext x;
	m_currentContextWnd = GetSafeHwnd();
	GetISystem()->SetViewCamera(CCamera(m_Camera));
	return x;
}

void CRenderViewport::RestorePreviousContext(const SPreviousContext & x) const
{
}
