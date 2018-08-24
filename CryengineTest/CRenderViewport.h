#pragma once

#include"DisplayContext.h"

#include<CryRenderer/IRenderer.h>
#include<Cry3DEngine/I3DEngine.h>

class CRenderViewport
{
public:

	struct SResolution
	{
		SResolution() :
			width(0), height(0)
		{
		}

		SResolution(int w, int h) :
			width(w), height(h)
		{
		}

		int width;
		int height;
	};
public:
	CRenderViewport();
	~CRenderViewport();

	void		SetRenderer(IRenderer* pRenderer) { m_pRenderer = pRenderer; }

	void		Set3DEngine(I3DEngine* p3DEngine) { m_p3DEngine = p3DEngine; }

	void		SetISystem(ISystem* pSystem) { m_pSystem = pSystem; }

	void*		GetSafeHwnd() { return m_hWnd; }

	void		SetSafeHwnd(HWND hWnd) { m_hWnd = hWnd; }

	void		SetResolution(int x, int y);

	IRenderer*	GetRenderer() const { return m_pRenderer; }

	I3DEngine*	Get3DEngine() const { return m_p3DEngine; }

	void		GetResolution(int& x,int& y);

	void		DrawAxis();


	bool		CreateRenderContext(uintptr_t displayContextHandle, IRenderer::EViewportType viewportType = IRenderer::EViewportType::eViewportType_Secondary);

	void		InitDisplayContext(uintptr_t displayContextHandle);

	void		Update();

	void		OnRender();

	void		OnResize();

	void		DrawBackground();

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	struct SScopedCurrentContext;
	struct SPreviousContext
	{

	};
	SPreviousContext	SetCurrentContext();
	void				RestorePreviousContext(const SPreviousContext& x) const;

protected:
	static void*              m_currentContextWnd;
protected:
	Matrix34 m_viewTM;
private:
	IRenderer*		m_pRenderer;
	I3DEngine*		m_p3DEngine;
	ISystem*		m_pSystem;
	HWND			m_hWnd;
	CCamera			m_Camera;
	DisplayContext	m_displayContext;
	

	bool			m_bRenderContextCreated;
	bool			m_bRenderStats;
	bool			m_bEmptyScene;
	SResolution		m_currentResolution;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
struct CRenderViewport::SScopedCurrentContext
{
	CRenderViewport*  viewport;
	CRenderViewport::SPreviousContext previousContext;

	SScopedCurrentContext(CRenderViewport* viewport)
		: viewport(viewport)
	{
		previousContext = viewport->SetCurrentContext();
	}

	~SScopedCurrentContext()
	{
		viewport->RestorePreviousContext(previousContext);
	}
}; 