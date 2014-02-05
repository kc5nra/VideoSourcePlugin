/**
 * John Bradley (jrb@turrettech.com)
 */
#include "VideoSourcePlugin.h"
#include "VideoSource.h"
#include "VideoSourceConfigDialog.h"
#include "resource.h"

#include <windowsx.h>
#include "vlc.h"

#include <cstdio>

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

#ifdef VLC2X
void log_callback(
    void *data, 
    int level, 
    const libvlc_log_t *ctx, 
    const char *format, 
    va_list args)
{
    wchar_t *levelString;
    switch (level) 
    {
    case LIBVLC_DEBUG:  levelString = TEXT("DEBUG  "); break;
    case LIBVLC_NOTICE: levelString = TEXT("NOTICE "); break;
    case LIBVLC_WARNING:levelString = TEXT("WARNING"); break;
    case LIBVLC_ERROR:  levelString = TEXT("ERROR  "); break;
    }
    
    char message[1024 * 16];
    memset(message, 0, sizeof(char) * 1024 * 16);

    vsnprintf(message, 1024 * 16, format, args);
    int messageLength = strlen(message);

    int messageBufferLength = utf8_to_wchar_len(message, messageLength, 0);
    wchar_t *messageBuffer = static_cast<wchar_t *>(calloc(messageBufferLength + 1, sizeof(wchar_t)));
    utf8_to_wchar(message, messageLength, messageBuffer, messageBufferLength, 0);

    Log(TEXT("VideoSourcePlugin::%s | %s"), levelString, messageBuffer);

    free(messageBuffer);
}
#endif


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
    
    OBSRegisterImageSourceClass(BROWSER_SOURCE_CLASS, STR("ClassName"), (OBSCREATEPROC)CreateVideoSource, (OBSCONFIGPROC)ConfigureBrowserSource);
    
    char *args[] = { 
        "--no-osd",
        "--disable-screensaver",
        "--ffmpeg-hw",
        "--no-video-title-show",
    };
    
    vlc = libvlc_new(4, args);

#ifdef VLC2X
    libvlc_log_set(vlc, log_callback, nullptr);
#endif
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
