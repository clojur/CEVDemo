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
		struct SResolution
		{
			SResolution() :
				width(0), height(0),x(0),y(0)
			{
			}

			SResolution(int x,int y,int w, int h) :
				 x(x),y(y),width(w), height(h)
			{
			}
			int x;
			int y;
			int width;
			int height;
		};
	protected:
		struct SPreviousContext
		{

		};
	public:
		CCryEngineTestViewport(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
		~CCryEngineTestViewport();

		void				SetHWND(HWND hWnd) { m_hWnd=hWnd; }

		HWND				GetHWND() { return  m_hWnd; };

		void				YJGLSJCreateWindow(int x, int y, int width, int height, const char* windowTitle);


		bool				StartProjectContext();

		SPreviousContext	SetCurrentContext();

		void				SetISystem(ISystem* pSystem);

		ISystem*			GetISystem() { return m_pSystem; }

		void				SetIRenderer(IRenderer* pRenderer);

		IRenderer*			GetIRenderer() { return m_pRenderer; }

		void				SetIEngine(I3DEngine* pEngine);

		I3DEngine*			GetIEngine() { return m_pEngine; }

		bool				CreateRenderContext(HWND hWnd, IRenderer::EViewportType viewportType);

		void				InitDisplayContext(SDisplayContextKey displayContextKey);

		void				OnRender();

		void				DrawAxis();

		void				DrawModel(const SRenderingPassInfo& passInfo);

		void				Run();

	public:
		virtual	bool		InitializeWindow() override;

		virtual int			GetWidth()  override { return m_currentResolution.width; }

		virtual int			GetHeight() override { return m_currentResolution.height; }

		virtual void*		GetSafeHwnd()override { return m_hWnd; }

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

		SResolution   m_currentResolution;
		SDisplayContextKey            m_displayContextKey;
		IStatObj*						m_pModel;
		IMaterial*					m_pMaterial;
		static void*              m_currentContextWnd;

	};
};

