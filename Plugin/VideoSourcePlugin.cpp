/**
 * John Bradley (jrb@turrettech.com)
 */
#include "VideoSourcePlugin.h"
#include "VideoSource.h"
#include "VideoSourceConfigDialog.h"
#include "resource.h"

#include <windowsx.h>

#define BROWSER_SOURCE_CLASS TEXT("VideoSource")

HINSTANCE VideoSourcePlugin::hinstDLL = NULL;
VideoSourcePlugin *VideoSourcePlugin::instance = NULL;

ImageSource* STDCALL CreateVideoSource(XElement *data)
{
    return new VideoSource(data);
}

bool STDCALL ConfigureBrowserSource(XElement *element, bool bCreating)
{

    XElement *dataElement = element->GetElement(TEXT("data"));

    bool isMissingDataElement;
    if (isMissingDataElement = !dataElement) {
        dataElement = element->CreateElement(TEXT("data"));
    }

    VideoSourceConfig *config = new VideoSourceConfig(dataElement);
    if (isMissingDataElement) {
        config->Populate();
    }

    config->InitializeAudioOutputVectors(VideoSourcePlugin::instance->GetVlc());

    VideoSourceConfigDialog videoSourceConfigDialog(config);
        
    if (videoSourceConfigDialog.Show()) {
        config->Save();
        element->SetInt(TEXT("cx"), config->width);
        element->SetInt(TEXT("cy"), config->height);

        delete config;
        return true;
    }

    delete config;
    return false;
}

VideoSourcePlugin::VideoSourcePlugin() {
	isDynamicLocale = false;

	if (!locale->HasLookup(KEY("PluginName"))) {
		isDynamicLocale = true;
		int localizationStringCount = sizeof(localizationStrings) / sizeof(CTSTR);
		Log(TEXT("Video Source Plugin strings not found, dynamically loading %d strings"), sizeof(localizationStrings) / sizeof(CTSTR));
		for(int i = 0; i < localizationStringCount; i += 2) {
			locale->AddLookupString(localizationStrings[i], localizationStrings[i+1]);
		}
		if (!locale->HasLookup(KEY("PluginName"))) {
			AppWarning(TEXT("Uh oh..., unable to dynamically add our localization keys"));
		}
	}
	
    API->RegisterImageSourceClass(BROWSER_SOURCE_CLASS, STR("ClassName"), (OBSCREATEPROC)CreateVideoSource, (OBSCONFIGPROC)ConfigureBrowserSource);
    
    char *args[] = { "--no-video-title-show" };
    vlc = libvlc_new(1, args);

}



VideoSourcePlugin::~VideoSourcePlugin() {
	
	if (isDynamicLocale) {
		int localizationStringCount = sizeof(localizationStrings) / sizeof(CTSTR);
		Log(TEXT("Video Source Plugin instance deleted; removing dynamically loaded localization strings"));
		for(int i = 0; i < localizationStringCount; i += 2) {
			locale->RemoveLookupString(localizationStrings[i]);
		}
	}

	isDynamicLocale = false;

}

bool LoadPlugin()
{
    if(VideoSourcePlugin::instance != NULL)
        return false;
    VideoSourcePlugin::instance = new VideoSourcePlugin();
    return true;
}

void UnloadPlugin()
{
    if(VideoSourcePlugin::instance == NULL)
        return;
    delete VideoSourcePlugin::instance;
    VideoSourcePlugin::instance = NULL;
}

void OnStartStream()
{
}

void OnStopStream()
{
}

CTSTR GetPluginName()
{
    return STR("PluginName");
}

CTSTR GetPluginDescription()
{
    return STR("PluginDescription");
}

BOOL CALLBACK DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
        VideoSourcePlugin::hinstDLL = hinstDLL;
    return TRUE;
}
