/**
 * John Bradley (jrb@turrettech.com)
 */
#include "VideoSourcePlugin.h"
#include "VideoSource.h"
#include "resource.h"

#include "Localization.h"


#define BROWSER_SOURCE_CLASS TEXT("VideoSource")

HINSTANCE VideoSourcePlugin::hinstDLL = NULL;
VideoSourcePlugin *VideoSourcePlugin::instance = NULL;

ImageSource* STDCALL CreateVideoSource(XElement *data)
{
    return new VideoSource(data);
}

INT_PTR CALLBACK ConfigureDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
            VideoSourceConfig *config = reinterpret_cast<VideoSourceConfig *>(lParam);

            LocalizeWindow(hwnd);

            HWND hwndPathOrUrl = GetDlgItem(hwnd, IDC_PATH_OR_URL);
            HWND hwndWidth = GetDlgItem(hwnd, IDC_WIDTH);
            HWND hwndHeight = GetDlgItem(hwnd, IDC_HEIGHT);
            HWND hwndVolume = GetDlgItem(hwnd, IDC_VOLUME);
            HWND hwndStretch = GetDlgItem(hwnd, IDC_STRETCH);
            HWND hwndIsAudioOutputToStream = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_STREAM);
            HWND hwndIsAudioOutputToDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_DEVICE);

            SendMessage(hwndPathOrUrl, WM_SETTEXT, 0, (LPARAM)config->pathOrUrl.Array());
            SendMessage(hwndWidth, WM_SETTEXT, 0, (LPARAM)IntString(config->width).Array());
            SendMessage(hwndHeight, WM_SETTEXT, 0, (LPARAM)IntString(config->height).Array());
         
            SendMessage(hwndStretch, BM_SETCHECK, config->isStretching, 0);

            SendMessage(hwndVolume, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0,100));
            SendMessage(hwndVolume, TBM_SETPOS, (WPARAM)1, config->volume);
           
            SendMessage(hwndIsAudioOutputToStream, BM_SETCHECK, config->isAudioOutputToStream, 0);
            SendMessage(hwndIsAudioOutputToDevice, BM_SETCHECK, !config->isAudioOutputToStream, 0);


            return TRUE;
        }
    case WM_COMMAND:
        {
            switch(LOWORD(wParam)) 
            {
            case IDOK:
                {
                    VideoSourceConfig *config = (VideoSourceConfig *)GetWindowLongPtr(hwnd, DWLP_USER);

                    HWND hwndPathOrUrl = GetDlgItem(hwnd, IDC_PATH_OR_URL);
                    HWND hwndWidth = GetDlgItem(hwnd, IDC_WIDTH);
                    HWND hwndHeight = GetDlgItem(hwnd, IDC_HEIGHT);
                    HWND hwndVolume = GetDlgItem(hwnd, IDC_VOLUME);
                    HWND hwndStretch = GetDlgItem(hwnd, IDC_STRETCH);
                    HWND hwndIsAudioOutputToStream = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_STREAM);
                    HWND hwndIsAudioOutputToDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_DEVICE);

                    String str;
                    str.SetLength((UINT)SendMessage(hwndPathOrUrl, WM_GETTEXTLENGTH, 0, 0));
                    if(!str.Length()) return TRUE;
                    SendMessage(hwndPathOrUrl, WM_GETTEXT, str.Length()+1, (LPARAM)str.Array());
                    config->pathOrUrl = str;

                    str.SetLength((UINT)SendMessage(hwndWidth, WM_GETTEXTLENGTH, 0, 0));
                    if(!str.Length()) return TRUE;
                    SendMessage(hwndWidth, WM_GETTEXT, str.Length()+1, (LPARAM)str.Array());
                    config->width = str.ToInt();

                    str.SetLength((UINT)SendMessage(hwndHeight, WM_GETTEXTLENGTH, 0, 0));
                    if(!str.Length()) return TRUE;
                    SendMessage(hwndHeight, WM_GETTEXT, str.Length()+1, (LPARAM)str.Array());
                    config->height = str.ToInt();
                    
                    config->volume = (unsigned int)SendMessage(hwndVolume, TBM_GETPOS, (WPARAM)1, config->volume);

                    config->isStretching = (SendMessage(hwndStretch, BM_GETCHECK, 0, 0) == 1);

                    config->isAudioOutputToStream = (SendMessage(hwndIsAudioOutputToStream, BM_GETCHECK, 0, 0) == 1);

                    EndDialog(hwnd, IDOK);
                    return TRUE;
                }
            case IDCANCEL:
                {
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;
                }
            }
            break;
        }
    case WM_CLOSE:
        {
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }

    return FALSE;
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

    if(DialogBoxParam(VideoSourcePlugin::hinstDLL, MAKEINTRESOURCE(IDD_VIDEOCONFIG), API->GetMainWindow(), ConfigureDialogProc, (LPARAM)config) == IDOK)
    {
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
