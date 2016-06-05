/**
\author		Konstantin Pajl aka Consta
\date		21.05.2016 (c)Korotkov Andrey

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

/*
* Working with OpenGL context in windows through
* WGL interface.
*/

#include "DGLE.h"
#include <GL\glew.h>
#include <GL\wglew.h>
//#include <strsafe.h> // for StringCchPrintf
using namespace DGLE;

static IEngineCore *_core;
static HWND _hwnd;
static HDC _hdc;
static HGLRC _hRC;

//void LogWinAPILastError(LPTSTR lpszFunction)
//{
//	// Retrieve the system error message for the last-error code
//	LPVOID lpMsgBuf;
//	LPVOID lpDisplayBuf;
//	DWORD dw = GetLastError();
//
//	FormatMessage(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		nullptr,
//		dw,
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		reinterpret_cast<LPTSTR>(&lpMsgBuf),
//		0, nullptr);
//
//	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
//	StringCchPrintf((LPTSTR)lpDisplayBuf, 
//					LocalSize(lpDisplayBuf) / sizeof(TCHAR),
//					TEXT("%s failed with error %d: %s"),
//					lpszFunction, dw, lpMsgBuf);
//
//	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
//
//	LocalFree(lpMsgBuf);
//	LocalFree(lpDisplayBuf);
//}

static void LogToDGLE(const char *pcTxt, E_LOG_TYPE eType, const char *pcSrcFileName, int iSrcLineNumber)
{
	_core->WriteToLogEx(pcTxt, eType, pcSrcFileName, iSrcLineNumber);
}
#define LOG_FATAL(txt) LogToDGLE(std::string(txt).c_str(), LT_FATAL, __FILE__, __LINE__)


// Useful extensions:
// WGL_ARB_pixel_format_float: Allows for floating-point framebuffers.
// WGL_ARB_framebuffer_sRGB: Allows for color buffers to be in sRGB format.
// WGL_ARB_multisample: Allows for multisampled framebuffers.

bool CreateGL(TWindowHandle hwnd, IEngineCore* pCore, const TEngineWindow& stWin)
{
	const int major_version = 3;
	const int minor_version = 2;

	PIXELFORMATDESCRIPTOR pfd{};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	
	int closest_pixel_format = 0;

	_hdc = GetDC(hwnd);

	if (stWin.eMultisampling == MM_NONE)
	{
		closest_pixel_format = ChoosePixelFormat(_hdc, &pfd);

		if (closest_pixel_format == 0)
		{
			LOG_FATAL("Wrong ChoosePixelFormat() result");
			return false;
		}

		if (!SetPixelFormat(_hdc, closest_pixel_format, &pfd))
		{
			LOG_FATAL("Wrong SetPixelFormat() result");
			return false;
		}

		HGLRC hrc_fake = wglCreateContext(_hdc);
		wglMakeCurrent(_hdc, hrc_fake);

		if (glewInit() != GLEW_OK)
		{
			LOG_FATAL("Couldn't initialize GLEW");
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(hrc_fake);
			return false;
		}

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hrc_fake);

	}
	else
	{
		HWND hwnd_fake = CreateWindowEx(0, TEXT("STATIC"), NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
		HDC hdc_fake = GetDC(hwnd_fake);
		int closest_pixel_format_temp = ChoosePixelFormat(hdc_fake, &pfd);
		SetPixelFormat(hdc_fake, closest_pixel_format_temp, &pfd);
		HGLRC hrc_fake = wglCreateContext(hdc_fake);
		wglMakeCurrent(hdc_fake, hrc_fake);

		if (glewInit() != GLEW_OK)
		{
			LOG_FATAL("Couldn't initialize GLEW");
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(hrc_fake);
			ReleaseDC(hwnd_fake, hdc_fake);
			DestroyWindow(hwnd_fake);
			return false;
		}

		// New way create default framebuffer
		if (WGLEW_ARB_pixel_format)
		{
			int samples = 0;
			switch (stWin.eMultisampling)
			{
				case MM_2X: samples = 2; break;
				case MM_4X: samples = 4; break;
				case MM_8X: samples = 8; break;
				case MM_16X: samples = 16; break;
				default: break;
			}

			const int iPixelFormatAttribList[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				WGL_SAMPLE_BUFFERS_ARB, 1, //Number of buffers (must be 1 at time of writing)
				WGL_SAMPLES_ARB, samples,
				0
			};

			int numFormats = 0;
			int chosen = wglChoosePixelFormatARB(_hdc, iPixelFormatAttribList, NULL, 1, &closest_pixel_format, (UINT*)&numFormats);
			if  (!chosen || numFormats <= 0)
			{
				LOG_FATAL("Wrong wglChoosePixelFormatARB() result");
				return false;
			}		
		}
		else
		{
			LOG_FATAL("Extension WGLEW_ARB_pixel_format didn't found in driver");
			return false;
		}

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hrc_fake);
		ReleaseDC(hwnd_fake, hdc_fake);
		DestroyWindow(hwnd_fake);
	}

	if (!SetPixelFormat(_hdc, closest_pixel_format, &pfd))
	{
		LOG_FATAL("Wrong SetPixelFormat() result");
		return false;
	}

	if (WGLEW_ARB_create_context)
	{
		const int context_attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, major_version,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor_version,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0 // end
		};

		_hRC = wglCreateContextAttribsARB(_hdc, nullptr, context_attribs); // context

		if (_hRC)
		{
			if (!wglMakeCurrent(_hdc, _hRC))
			{
				wglDeleteContext(_hRC);
				ReleaseDC(_hwnd, _hdc);
				LOG_FATAL("Couldn't perform wglMakeCurrent(_hdc, _hRC);");
			}
		}
		else
		{
			LOG_FATAL("Couldn't create OpenGL context with wglCreateContextAttribsARB()");
			return false;
		}
	}
	else
	{
		LOG_FATAL("Extension WGLEW_ARB_create_context didn't found in driver");
		return false;
	}

	return true;
}

void MakeCurrent()
{
	wglMakeCurrent(_hdc, _hRC);
}

void FreeGL()
{	
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(_hRC);
	ReleaseDC(_hwnd, _hdc);
}

void SwapBuffer()
{	
	::SwapBuffers(_hdc);
}
