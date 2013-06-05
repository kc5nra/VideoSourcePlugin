/**
* John Bradley (jrb@turrettech.com)
*/

#include "VideoSourcePluginWrapper.h"

static HINSTANCE hinstDLL = 0;
static HMODULE hmodVspPlugin;
static HMODULE hmodLibVlc;
static HMODULE hmodLibVlcCore;

static LOADPLUGIN_PROC InternalLoadPlugin = 0;
static UNLOADPLUGIN_PROC InternalUnloadPlugin = 0;
static ONSTARTSTREAM_PROC InternalOnStartStream = 0;
static ONSTOPSTREAM_PROC InternalOnStopStream = 0;
static GETPLUGINNAME_PROC InternalGetPluginName = 0;
static GETPLUGINDESCRIPTION_PROC InternalGetPluginDescription = 0;

bool LoadPlugin()
{
    if (InternalLoadPlugin &&
        InternalUnloadPlugin &&
        InternalOnStartStream && 
        InternalOnStopStream &&
        InternalGetPluginName &&
        InternalGetPluginDescription) 
    {
        return InternalLoadPlugin();
    }

    return false;
}

void UnloadPlugin()
{
    InternalUnloadPlugin();
}

void OnStartStream()
{
    InternalOnStartStream();
}

void OnStopStream()
{
    InternalOnStopStream();
}

CTSTR GetPluginName()
{
    return InternalGetPluginName();
}

CTSTR GetPluginDescription()
{
    return InternalGetPluginDescription();
}

BOOL CALLBACK DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        {
            // order is important!
       
            hmodLibVlcCore = LoadLibrary(L".\\plugins\\VideoSourcePlugin\\libvlccore.dll");
            hmodLibVlc = LoadLibrary(L".\\plugins\\VideoSourcePlugin\\libvlc.dll");
        
            // main plugin dll
            hmodVspPlugin = LoadLibrary(L".\\plugins\\VideoSourcePlugin\\VideoSourcePlugin.dll"); 

            if (hmodVspPlugin != NULL) {
                InternalLoadPlugin = (LOADPLUGIN_PROC)GetProcAddress(hmodVspPlugin, "LoadPlugin");
                InternalUnloadPlugin = (UNLOADPLUGIN_PROC)GetProcAddress(hmodVspPlugin, "UnloadPlugin");
                InternalOnStartStream = (ONSTARTSTREAM_PROC)GetProcAddress(hmodVspPlugin, "OnStartStream");
                InternalOnStopStream = (ONSTOPSTREAM_PROC)GetProcAddress(hmodVspPlugin, "OnStopStream");
                InternalGetPluginName = (GETPLUGINNAME_PROC)GetProcAddress(hmodVspPlugin, "GetPluginName");
                InternalGetPluginDescription = (GETPLUGINDESCRIPTION_PROC)GetProcAddress(hmodVspPlugin, "GetPluginDescription");
            }
            break;
        }
    case DLL_PROCESS_DETACH:
        {
            if (hmodVspPlugin) FreeLibrary(hmodVspPlugin);
            if (hmodLibVlc) FreeLibrary(hmodLibVlc);
            if (hmodLibVlcCore) FreeLibrary(hmodLibVlcCore);
            break;
        }
    }
    return TRUE;
}
