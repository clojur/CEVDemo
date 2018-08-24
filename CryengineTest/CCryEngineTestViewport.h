#pragma once
#include "IViewport.h"
#include "DisplayContext.h"
#include<CryRenderer/IRenderer.h>
#include<Cry3DEngine/I3DEngine.h>

struct ISystem;
struct IRenderer;
struct I3DEngine;
namespace YJGLSJ
{
	class CCryEngineTestViewport :public IViewport
	{
	public:
		CCryEngineTestViewport(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
		~CCryEngineTestViewport();

		void				SetHWND(HWND hWnd) { m_hWnd=hWnd; }

		HWND				GetHWND() { return  m_hWnd; };

		void				YJGLSJCreateWindow(int x, int y, int width, int height, const char* windowTitle);


		bool				StartProjectContext();

		void				SetISystem(ISystem* pSystem);

		ISystem*			GetISystem() { return m_pSystem; }

		void				SetIRenderer(IRenderer* pRenderer);

		IRenderer*			GetIRenderer() { return m_pRenderer; }

		void				SetIEngine(I3DEngine* pEngine);

		I3DEngine*			GetIEngine() { return m_pEngine; }

		bool				CreateRenderContext(uintptr_t displayContextHandle, IRenderer::EViewportType viewportType = IRenderer::EViewportType::eViewportType_Secondary);

		void				InitDisplayContext(uintptr_t displayContextHandle);

		void				OnRender();

		void				DrawAxis();

		void				Run();

	public:
		virtual	bool		InitializeWindow() override;

		virtual int			GetWidth()  override { return m_iWidth; }

		virtual int			GetHeight() override { return m_iHeight; }

		virtual size_t		GetSafeHwnd() { return (size_t)m_hWnd; }

		virtual	void		OnSize(int width, int height) override;

		virtual	void		OnMove(int x, int y) override;

		virtual void		DestoryWindow() override;

		virtual	LRESULT		WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static	LRESULT CALLBACK DefProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static	CCryEngineTestViewport*	GetInstance();

		static	bool					InitEngineSystem();

	private:
		static CCryEngineTestViewport* s_instance;

		HINSTANCE		m_hInstance;
		HINSTANCE		m_hPrevInstance; 
		LPSTR			m_lpCmdLine; 
		int				m_nCmdShow;
		HWND			m_hWnd;

		int				m_ix;
		int				m_iy;
		int				m_iWidth;
		int				m_iHeight;
		const char*		m_cszTitle;

		ISystem*		m_pSystem;
		IRenderer*		m_pRenderer;
		I3DEngine*		m_pEngine;

		CCamera			m_Camera;
		DisplayContext  m_displayContext;
		Matrix34		m_viewTM;

		bool			m_bRenderContextCreated;
		bool			m_bRenderStats;
		bool			m_bEmptyScene;

	};
};

