//
// Loading geometry and creation it through core render
//
#include <DGLE.h>
#include <DGLE_CoreRenderer.h>

using namespace DGLE;

DGLE_DYNAMIC_FUNC

#define APP_CAPTION "Core geometry"
#define DLL_PATH "..\\..\\..\\DGLE\\bin\\windows\\DGLE.dll"
#define MODELS_PATH "..\\resources\\models\\"
#define SCREEN_WIDTH 1000u
#define SCREEN_HEIGHT 700u

IEngineCore *pEngineCore = nullptr;
ICoreRenderer *pCoreRender = nullptr;
IRender3D *pRender3D;
IRender *pRender;
IResourceManager *pResMan;
IInput* pInput;
IMesh *pMesh1;
uint uiCounter = 0;
uint prevWindowWidth, prevWindowHeight;

void DGLE_API Init(void *pParameter)
{
	pEngineCore->GetSubSystem(ESS_RENDER, reinterpret_cast<IEngineSubSystem *&>(pRender));
	pEngineCore->GetSubSystem(ESS_CORE_RENDERER, reinterpret_cast<IEngineSubSystem *&>(pCoreRender));
	
	pRender->GetRender3D(pRender3D);
	pEngineCore->GetSubSystem(ESS_RESOURCE_MANAGER, reinterpret_cast<IEngineSubSystem *&>(pResMan));
	pEngineCore->GetSubSystem(ESS_INPUT, reinterpret_cast<IEngineSubSystem *&>(pInput));
	pResMan->Load(MODELS_PATH"teapot.dmd", reinterpret_cast<IEngineBaseObject *&>(pMesh1), MMLF_FORCE_MODEL_TO_MESH);
	//pResMan->GetDefaultResource(EOT_MESH, reinterpret_cast<IEngineBaseObject *&>(pMesh));
}

void DGLE_API Update(void *pParameter)
{
	// exit by pressing "Esc" key
	bool is_pressed;
	pInput->GetKey(KEY_ESCAPE, is_pressed);
	if (is_pressed)
	{
		pEngineCore->QuitEngine();
	}

	++uiCounter;
}

void DGLE_API Render(void *pParameter)
{
	pRender3D->SetPerspective(45.f, 0.1f, 100.0f);

	pRender3D->SetMatrix
		(
			MatrixRotate(static_cast<float>(uiCounter), TVector3(0.f, 1.f, 0.f)) *
			MatrixRotate(static_cast<float>(25), TVector3(1.f, 0.f, 0.f)) *
			MatrixTranslate(TVector3(0.0f, -0.3f, -3.5f)) *
			MatrixIdentity() // zero point for all transformations
			);

	pCoreRender->PushStates();

	TRasterizerStateDesc rasterState;
	pCoreRender->GetRasterizerState(rasterState);
	rasterState.bWireframe = true;
	pCoreRender->SetRasterizerState(rasterState);

	pMesh1->Draw();

	pCoreRender->PopStates();

}

// callback on switching to fullscreen event
void DGLE_API OnFullScreenEvent(void *pParameter, IBaseEvent *pEvent)
{
	IEvGoFullScreen *p_event = (IEvGoFullScreen *)pEvent;

	uint res_width, res_height;
	bool go_fscreen;
	p_event->GetResolution(res_width, res_height, go_fscreen);

	if (go_fscreen)
	{
		prevWindowWidth = res_width;
		prevWindowHeight = res_height;

		pEngineCore->GetDesktopResolution(res_width, res_height);
		p_event->SetResolution(res_width, res_height);
	}
	else
		p_event->SetResolution(prevWindowWidth, prevWindowHeight);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (GetEngine(DLL_PATH, pEngineCore))
	{
		if (SUCCEEDED(pEngineCore->InitializeEngine(NULL, APP_CAPTION, TEngineWindow(SCREEN_WIDTH, SCREEN_HEIGHT, false, false, MM_4X, EWF_ALLOW_SIZEING), 33u, static_cast<E_ENGINE_INIT_FLAGS>(EIF_LOAD_ALL_PLUGINS | EIF_NO_SPLASH))))
		{
			pEngineCore->ConsoleExecute("rnd2d_profiler 2");
			pEngineCore->AddProcedure(EPT_INIT, &Init);
			pEngineCore->AddProcedure(EPT_RENDER, &Render);
			pEngineCore->AddProcedure(EPT_UPDATE, &Update);
			pEngineCore->AddEventListener(ET_ON_FULLSCREEN, &OnFullScreenEvent);
			pEngineCore->StartEngine();
		}

		FreeEngine();
	}
	else
		MessageBox(nullptr, "Couldn't load \"" DLL_PATH "\"!", APP_CAPTION, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);

	return 0;
}