/**
 * John Bradley (jrb@turrettech.com)
 */
#pragma once

#include "OBSApi.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT bool LoadPlugin();
EXTERN_DLL_EXPORT void UnloadPlugin();
EXTERN_DLL_EXPORT void OnStartStream();
EXTERN_DLL_EXPORT void OnStopStream();
EXTERN_DLL_EXPORT CTSTR GetPluginName();
EXTERN_DLL_EXPORT CTSTR GetPluginDescription();

typedef bool (*LOADPLUGIN_PROC)();
typedef void (*UNLOADPLUGIN_PROC)();
typedef void (*ONSTARTSTREAM_PROC)();
typedef void (*ONSTOPSTREAM_PROC)();
typedef CTSTR (*GETPLUGINNAME_PROC)();
typedef CTSTR (*GETPLUGINDESCRIPTION_PROC)();