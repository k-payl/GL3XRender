#include "Common.h"
#include <GL\glew.h>
#include <GL\wglew.h>
//#include <strsafe.h> // for StringCchPrintf

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

bool CreateGL(TWindowHandle hwnd)
{
	_hdc = GetDC(hwnd);

	// 1
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;     // ��� ��������
	pfd.cColorBits = 32;                // ��� �����
	pfd.cDepthBits = 32;                // ��� z-������
	pfd.iLayerType = PFD_MAIN_PLANE;

	int closest_pixel_format = ChoosePixelFormat(_hdc, &pfd);

	if (closest_pixel_format == 0)
	{
		LogWrite(-1, "wrong ChoosePixelFormat() result", LT_FATAL, __FILE__, __LINE__);
		//TODO: safe delete all temporary stuff
		return false;
	}

	if (!SetPixelFormat(_hdc, closest_pixel_format, &pfd))
	{
		LogWrite(-1, "wrong SetPixelFormat() result", LT_FATAL, __FILE__, __LINE__);
		//TODO: safe delete all temporary stuff
		return false;
	}

	// Create fake context to get all new functions
	// in order to initialize OpenGL >=3.2
	HGLRC _hRC_fake = wglCreateContext(_hdc);
	wglMakeCurrent(_hdc, _hRC_fake);

	if (glewInit() != GLEW_OK)
	{
		LogWrite(-1, "Couldn't initialize GLEW", LT_FATAL, __FILE__, __LINE__);
		//TODO: safe delete all temporary stuff
		return false;
	}

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(_hRC_fake);



	// 2
	const int major_version = 3;
	const int minor_version = 3;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;     // ��� ��������
	pfd.cColorBits = 32;                // ��� �����
	pfd.cDepthBits = 32;                // ��� z-������
	pfd.iLayerType = PFD_MAIN_PLANE;

	// ������������� ��������� ����������� ���������
	if (WGLEW_ARB_create_context && WGLEW_ARB_pixel_format)
	{
		//const int iPixelFormatAttribList[] =
		//{
		//	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		//	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		//	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		//	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // ��� ��������
		//	WGL_COLOR_BITS_ARB, 32,                // ��� �����
		//	WGL_DEPTH_BITS_ARB, 32,                // ��� z-������
		//	WGL_STENCIL_BITS_ARB, 8,
		//	0 // end
		//};

		//int iPixelFormat, iNumFormats;
		//wglChoosePixelFormatARB(_hdc, iPixelFormatAttribList, NULL, 1, &iPixelFormat, (UINT*)&iNumFormats);

		int closest_pixel_format = ChoosePixelFormat(_hdc, &pfd);

		if (closest_pixel_format == 0)
		{
			LogWrite(-1, "wrong ChoosePixelFormat() result", LT_FATAL, __FILE__, __LINE__);
			//TODO: safe delete all temporary stuff
			return false;
		}

		if (!SetPixelFormat(_hdc, closest_pixel_format, &pfd))
		{
			LogWrite(-1, "wrong SetPixelFormat() result", LT_FATAL, __FILE__, __LINE__);
			//TODO: safe delete all temporary stuff
			return false;
		}

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
				LogWrite(-1, "Couldn't perform wglMakeCurrent(_hdc, _hRC);", LT_FATAL, __FILE__, __LINE__);
			}
		}
		else
		{
			LogWrite(-1, "Couldn't create OpenGL context with wglCreateContextAttribsARB()", LT_FATAL, __FILE__, __LINE__);
			return false;
		}
	}
	else
	{
		LogWrite(-1, "Extension WGLEW_ARB_create_context or WGLEW_ARB_pixel_format didn't found in driver", LT_FATAL, __FILE__, __LINE__);
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
