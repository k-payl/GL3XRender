/**
\author		Konstantin Pajl aka Consta
\date		11.06.2016 (c)Andrey Korotkov

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#pragma once

#include "DGLE.h"
using namespace DGLE;

#define PLUGIN_NAME				"GL3XRender"
#define PLUGIN_VERSION			"0.01 (" __DATE__ ")"
#define PLUGIN_VENDOR			"Consta"
#define PLUGIN_DESCRIPTION		"OpenGL 3.2 and above core render implementation for DGLE"
#define PLUGIN_INTERFACE_NAME	"ISubSystemPlugin"

class GL3XCoreRender;

class CPluginCore : public ISubSystemPlugin
{
	friend void LogToDGLE(uint uiInstIdx, const char *pcTxt, E_LOG_TYPE eType, const char *pcSrcFileName, int iSrcLineNumber);
	
	uint _uiInstIdx;
	IEngineCore *_pEngineCore;
	GL3XCoreRender *_pGL3XCoreRender;

	int _iDrawProfiler;

	void _Render();
	void _Update(uint uiDeltaTime);
	void _Init();
	void _Free();
	void _MsgProc(const TWindowMessage &stMsg);
	void _ProfilerDraw();

	static void DGLE_API _s_EventHandler(void *pParameter, IBaseEvent *pEvent);
	static void DGLE_API _s_Render(void *pParameter);
	static void DGLE_API _s_Update(void *pParameter);
	static void DGLE_API _s_Init(void *pParameter);
	static void DGLE_API _s_Free(void *pParameter);

public:

	CPluginCore(IEngineCore *pEngineCore);
	~CPluginCore();
	IEngineCore* GetCore() { return _pEngineCore; }

	DGLE_RESULT DGLE_API GetPluginInfo(TPluginInfo &stInfo);
	DGLE_RESULT DGLE_API GetPluginInterfaceName(char* pcName, uint &uiCharsCount);
	DGLE_RESULT DGLE_API GetSubSystemInterface(IEngineSubSystem *&prSubSystem);

	IDGLE_BASE_IMPLEMENTATION(IPlugin, INTERFACE_IMPL_END)
};