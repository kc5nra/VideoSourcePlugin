/**
 * John Bradley (jrb@turrettech.com)
 */
#pragma once

#include "OBSApi.h"
#include "vlc.h"
#include "VideoSourceConfig.h"
#include "Localization.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

class VideoSourcePlugin
{

public:
    static HINSTANCE hinstDLL;
	static VideoSourcePlugin *instance;

public:
    VideoSourcePlugin();
    ~VideoSourcePlugin();

private:
	bool isDynamicLocale;
    libvlc_instance_t *vlc;

public:
    libvlc_instance_t *GetVlc() { return vlc; }
};

EXTERN_DLL_EXPORT bool LoadPlugin();
EXTERN_DLL_EXPORT void UnloadPlugin();
EXTERN_DLL_EXPORT void OnStartStream();
EXTERN_DLL_EXPORT void OnStopStream();
EXTERN_DLL_EXPORT CTSTR GetPluginName();
EXTERN_DLL_EXPORT CTSTR GetPluginDescription();
