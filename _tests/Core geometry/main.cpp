//
// Loading geometry and creation it through core render
//
#include <DGLE.h>
#include <DGLE_CoreRenderer.h>

using namespace DGLE;

DGLE_DYNAMIC_FUNC

#define APP_CAPTION "Core geometry"
#define DLL_PATH "..\\..\\..\\DGLE\\bin\\windows\\DGLE.dll"
#define RESOURCE_PATH "..\\resources\\"
#define SCREEN_WIDTH 1000u
#define SCREEN_HEIGHT 700u

IEngineCore *pEngineCore = nullptr;
//ICoreRenderer *pCoreRender = nullptr;
IRender3D *pRender3D;
IRender *pRender;
IResourceManager *pResMan;
IMesh *mesh;
uint uiCounter = 0;

void DGLE_API Init(void *pParameter)
{
	pEngineCore->GetSubSystem(ESS_RENDER, reinterpret_cast<IEngineSubSystem *&>(pRender));
	//pEngineCore->GetSubSystem(ESS_CORE_RENDERER, reinterpret_cast<IEngineSubSystem *&>(pCoreRender));
	pRender->GetRender3D(pRender3D);

	pEngineCore->GetSubSystem(ESS_RESOURCE_MANAGER, reinterpret_cast<IEngineSubSystem *&>(pResMan));
	pResMan->Load(RESOURCE_PATH"cube.dmd", reinterpret_cast<IEngineBaseObject *&>(mesh), MMLF_FORCE_MODEL_TO_MESH);
}

void DGLE_API Update(void *pParameter)
{
	++uiCounter;
}

void DGLE_API Render(void *pParameter)
{
	pRender3D->SetPerspective(45.f, 0.1f, 100.0f);

	pRender3D->SetMatrix
	( 
		MatrixRotate(static_cast<float>(uiCounter), TVector3(0.f, 1.f, 0.f)) * 
		MatrixTranslate(TVector3(0.f, 0.f, -2.5f)) * 
		MatrixIdentity() // use identity matrix as zero point for all transformations
	);

	mesh->Draw();
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (GetEngine(DLL_PATH, pEngineCore))
	{
		if (SUCCEEDED(pEngineCore->InitializeEngine(NULL, APP_CAPTION, TEngineWindow(SCREEN_WIDTH, SCREEN_HEIGHT, false, false), 33u, static_cast<E_ENGINE_INIT_FLAGS>(EIF_LOAD_ALL_PLUGINS | EIF_NO_SPLASH))))
		{
			pEngineCore->AddProcedure(EPT_INIT, &Init);
			pEngineCore->AddProcedure(EPT_RENDER, &Render);
			pEngineCore->AddProcedure(EPT_UPDATE, &Update);

			pEngineCore->StartEngine();
		}

		FreeEngine();
	}
	else
		MessageBox(nullptr, "Couldn't load \"" DLL_PATH "\"!", APP_CAPTION, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);

	return 0;
}