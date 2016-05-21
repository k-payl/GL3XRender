/**
\author		Korotkov Andrey aka DRON
\date		23.03.2016 (c)Korotkov Andrey

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#include "PluginCore.h"
#include <memory>
#include <vector>
#include <algorithm>
using namespace std;


static vector<unique_ptr<CPluginCore>> pluginCores;

void CALLBACK InitPlugin(IEngineCore *engineCore, ISubSystemPlugin *&plugin)
{
	pluginCores.push_back(make_unique<CPluginCore>(engineCore));
	plugin = pluginCores.back().get();
}

void CALLBACK FreePlugin(IPlugin *plugin)
{
	typedef decltype(pluginCores) TPluginCores;
	pluginCores.erase(find_if(pluginCores.begin(), pluginCores.end(), [plugin](TPluginCores::const_reference curPlugin)
	{
		return curPlugin.get() == plugin;
	}));
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}