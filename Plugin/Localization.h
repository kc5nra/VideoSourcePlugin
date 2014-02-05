/**
 * John Bradley (jrb@turrettech.com)
 */
#pragma once

#include "OBSApi.h"

#define STR_PREFIX L"Plugins.VideoSource."
#define KEY(k) (STR_PREFIX L ## k)
#define STR(text) locale->LookupString(KEY(text))

#ifndef VSP_VERSION
#define VSP_VERSION L" !INTERNAL VERSION!"
#endif

static CTSTR localizationStrings[] = {
    KEY("PluginName"),          L"Video Source",
    KEY("PluginDescription"),   L"Uses the libvlc library to play audio, local videos and local/remote streams\n\n"
                                L"Plugin Version: " VSP_VERSION,
    KEY("ClassName"),           L"Video",
    KEY("Settings"),			L"Video Source Settings",
    KEY("PathOrUrl"),			L"Enter the path of the video file you would like to load:",
    KEY("Video"),               L"Video Settings",
    KEY("VideoWidth"),          L"Video Width:",
    KEY("VideoHeight"),         L"Video Height:",
    KEY("StretchImage"),        L"Stretch image",
    KEY("Audio"),               L"Audio Settings",
    KEY("Volume"),              L"Video Volume (0-100)",
    KEY("AudioOutputToStream"), L"Output audio to stream",
    KEY("AudioOutputToDevice"), L"Output audio to device",
    KEY("AudioOutputType"),     L"Audio Module",
    KEY("AudioOutputDevice"),   L"Audio Device",
    KEY("MediaFileOrUrl"),      L"Enter a file path or a URL pointing to a media object",
    KEY("Playlist"),            L"Playlist Settings",
    KEY("PlaylistLoop"),        L"Playlist looping",
    KEY("PlaylistEditor"),        L"Playlist Editor",
    KEY("Deinterlacing"),       L"Deinterlacing:",
};

