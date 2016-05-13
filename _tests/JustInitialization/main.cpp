//
// Test creation OpenGL 3.2 context in DGLE
//
#include <DGLE.h>

using namespace DGLE;

DGLE_DYNAMIC_FUNC

#define APP_CAPTION "Just initialization"
#define DLL_PATH "..\\..\\..\\DGLE\\bin\\windows\\DGLE.dll"
#define SCREEN_WIDTH 200u
#define SCREEN_HEIGHT 200u

IEngineCore *pEngineCore = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (GetEngine(DLL_PATH, pEngineCore))
	{
		if (SUCCEEDED(pEngineCore->InitializeEngine(NULL, APP_CAPTION, TEngineWindow(SCREEN_WIDTH, SCREEN_HEIGHT, false, false), 33u, EIF_LOAD_ALL_PLUGINS)))
		{
			pEngineCore->StartEngine();
		}

		FreeEngine();
	}
	else
		MessageBox(nullptr, "Couldn't load \"" DLL_PATH "\"!", APP_CAPTION, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);

	return 0;
}