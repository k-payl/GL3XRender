//
// Test texturing
//
#include "GL3XCoreRender.h"
#include <DGLE.h>
#include <DGLE_CoreRenderer.h>

#include <assert.h>

using namespace DGLE;

DGLE_DYNAMIC_FUNC

#define APP_CAPTION "Textured"
#define DLL_PATH "..\\..\\..\\DGLE\\bin\\windows\\DGLE.dll"
#define MODELS_PATH "..\\resources\\models\\"
#define TEXTURES_PATH "..\\resources\\textures\\"
#define SCREEN_WIDTH 1000u
#define SCREEN_HEIGHT 600u

IEngineCore *pEngineCore = nullptr;
ICoreRenderer *pCoreRender = nullptr;
IRender3D *pRender3D;
IRender2D *pRender2D;
IRender *pRender;
IResourceManager *pResMan;
IInput* pInput;
IMesh *pMesh1, *pMesh2;
ITexture *pTex1, *pTex2, *pTex3;
uint uiCounter = 0;
uint prevWindowWidth, prevWindowHeight;
ITexture *pRenderTarget;
ICoreTexture *pRenderTargetCore;

void E_GUARDS()
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::string error;

		switch (err) {
		case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		assert(err == GL_NO_ERROR);
	}
}

void DGLE_API Init(void *pParameter)
{
	pEngineCore->GetSubSystem(ESS_RENDER, reinterpret_cast<IEngineSubSystem *&>(pRender));
	pEngineCore->GetSubSystem(ESS_CORE_RENDERER, reinterpret_cast<IEngineSubSystem *&>(pCoreRender));
	pRender->GetRender3D(pRender3D);
	pRender->GetRender2D(pRender2D);
	pEngineCore->GetSubSystem(ESS_RESOURCE_MANAGER, reinterpret_cast<IEngineSubSystem *&>(pResMan));
	pEngineCore->GetSubSystem(ESS_INPUT, reinterpret_cast<IEngineSubSystem *&>(pInput));
	pResMan->Load(MODELS_PATH"plane.dmd", reinterpret_cast<IEngineBaseObject *&>(pMesh2), MMLF_FORCE_MODEL_TO_MESH);
	pResMan->Load(TEXTURES_PATH"ss_color3.jpg", reinterpret_cast<IEngineBaseObject *&>(pTex2), TLF_GENERATE_MIPMAPS | TLF_FILTERING_ANISOTROPIC);
	pResMan->Load(MODELS_PATH"h60.dmd", reinterpret_cast<IEngineBaseObject *&>(pMesh1), MMLF_FORCE_MODEL_TO_MESH);
	pResMan->Load(TEXTURES_PATH"u60_1.jpg", reinterpret_cast<IEngineBaseObject *&>(pTex1), TLF_GENERATE_MIPMAPS | TLF_FILTERING_ANISOTROPIC);

	pResMan->CreateTexture(pRenderTarget, NULL, 600, 600, TDF_RGB8, TCF_DEFAULT, (E_TEXTURE_LOAD_FLAGS)(TLF_GENERATE_MIPMAPS | TLF_FILTERING_ANISOTROPIC));
	pRenderTarget->GetCoreTexture(pRenderTargetCore);

	pRender3D->SetPerspective(45.f, 0.1f, 100.0f);
	E_GUARDS();
	glewInit();
	E_GUARDS();

	pCoreRender->SetClearColor(ColorGray());

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

	// 1 //
	/*
	pCoreRender->SetRenderTarget(pRenderTargetCore);
	pCoreRender->Clear(true, true, false);

	pRender3D->SetMatrix
	(
		MatrixRotate(static_cast<float>(uiCounter * 0.25), TVector3(0.f, 1.f, 0.f)) *
		MatrixRotate(static_cast<float>(25 - 180), TVector3(1.f, 0.f, 0.f)) *
		MatrixTranslate(TVector3(.0f, -0.3f, -3.5f)) *
		MatrixIdentity() // zero point for all transformations
	);

	pRender3D->BindTexture(pTex1, 0);
	pRender3D->PushMatrix();
	pRender3D->MultMatrix(MatrixScale(TVector3(0.15f, 0.15f, 0.15f)));
	pMesh1->Draw();
	pRender3D->PopMatrix();
	
	GLTexture* tex = static_cast<GLTexture*>(pRenderTargetCore);
	GLuint tex_id = tex->Texture_ID();
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glGenerateMipmap(GL_TEXTURE_2D);
	*/


	// 2 //
	pCoreRender->SetRenderTarget(nullptr);
	
	pRender3D->SetMatrix
	(
		MatrixRotate(static_cast<float>( 0.25), TVector3(0.f, 1.f, 0.f)) *
		MatrixRotate(static_cast<float>(25), TVector3(1.f, 0.f, 0.f)) *
		MatrixTranslate(TVector3(.0f, -0.3f, -3.5f)) *
		MatrixIdentity() // zero point for all transformations
	);
	
	pRender3D->BindTexture(pTex2, 0);
	pMesh2->Draw();	

	uint x, y;
	pTex2->GetDimensions(x, y);
	pRender2D->DrawTexture(pTex2, TPoint2(0, 0), TVec2(x/2, y/2));
	
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
		if (SUCCEEDED(pEngineCore->InitializeEngine(NULL, APP_CAPTION, TEngineWindow(SCREEN_WIDTH, SCREEN_HEIGHT, false, false, MM_NONE, EWF_ALLOW_SIZEING), 33u, static_cast<E_ENGINE_INIT_FLAGS>(EIF_LOAD_ALL_PLUGINS | EIF_NO_SPLASH))))
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
		MessageBox(nullptr, TEXT("Couldn't load \"") DLL_PATH TEXT("\"!"), TEXT(APP_CAPTION), MB_OK | MB_ICONERROR | MB_SETFOREGROUND);

	return 0;
}