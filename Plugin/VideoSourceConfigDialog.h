#pragma once

#include "OBSApi.h"

class VideoSourceConfig;

class VideoSourceConfigDialog
{
private:
    VideoSourceConfig *config;

public:
    HWND hwndPathOrUrl;
    HWND hwndWidth;
    HWND hwndHeight;
    HWND hwndVolume;
    HWND hwndStretch;
    HWND hwndIsAudioOutputToStream;
    HWND hwndIsAudioOutputToDevice;
    HWND hwndAudioOutputType;
    HWND hwndAudioOutputDevice;

public:
    VideoSourceConfigDialog(VideoSourceConfig *config);
    ~VideoSourceConfigDialog();

public:
    bool Show();

public:
    VideoSourceConfig *GetConfig() { return config; }
};