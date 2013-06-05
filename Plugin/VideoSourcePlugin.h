/**
 * John Bradley (jrb@turrettech.com)
 */
#pragma once

#include "OBSApi.h"
#include "vlc\vlc.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

struct VideoSourceConfig 
{
private:
    XElement *element;
public:
    String pathOrUrl;
    unsigned int width;
    unsigned int height;
    bool isStretching;
    unsigned int volume;


    VideoSourceConfig(XElement *element)
    {
        this->element = element;
        Reload();
    }

    void Populate()
    {
        pathOrUrl = TEXT("");
        width = 640;
        height = 480;
        volume = 100;
        isStretching = false;
    }

    void Reload()
    {
        pathOrUrl = element->GetString(TEXT("pathOrUrl"));
        width = element->GetInt(TEXT("width"));
        height = element->GetInt(TEXT("height"));
        volume = element->GetInt(TEXT("volume"));
        isStretching = element->GetInt(TEXT("isStretching")) == 1;
    }

    void Save()
    {
        element->SetString(TEXT("pathOrUrl"), pathOrUrl);
        element->SetInt(TEXT("width"), width);
        element->SetInt(TEXT("height"), height);
        element->SetInt(TEXT("volume"), volume);
        element->SetInt(TEXT("isStretching"), isStretching ? 1 : 0);
    }
};

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
