#include	"stdafx.h"

#include	"CCryEngineTestViewport.h"

#define CRY_PLATFORM_DESKTOP

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	YJGLSJ::CCryEngineTestViewport::InitEngineSystem();

	YJGLSJ::CCryEngineTestViewport* pCeTest = new YJGLSJ::CCryEngineTestViewport(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	pCeTest->YJGLSJCreateWindow(300, 300, 800, 600, "CryEngineTest");
	pCeTest->StartProjectContext();

	pCeTest->GetIEngine()->UnloadLevel();
	//bool bOpen= GetISystem()->GetIPak()->OpenPacks("G:/CryEngine/CEProject/test/Assets/levels/example/*.pak");
	//bOpen= pCeTest->GetIEngine()->LoadLevel("G:/CryEngine/CEProject/test/Assets/levels/example", "Mission0");
	pCeTest->Run();
	return 0;
}