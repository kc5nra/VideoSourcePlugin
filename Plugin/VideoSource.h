/**
* John Bradley (jrb@turrettech.com)
*/
#pragma once

#include "OBSApi.h"
#include "VideoSourcePlugin.h"
#include "vlc\vlc.h"

#include <vector>

class DataSourceWithMimeType;
class JavascriptExtension;

struct BrowserSourceConfig;

class VideoSource : public ImageSource
{
    class BrowserSourceListener;

public:
    VideoSource(XElement *data);
    ~VideoSource();

private:
    Vect2 videoSize;
    Texture *texture;
    libvlc_instance_t *vlc;
    libvlc_media_player_t *mediaPlayer;

public:
    VideoSourceConfig *config;
    CRITICAL_SECTION textureLock;
    void *pixelData;

public:
    Texture *GetTexture() { return texture; }

public:
    // ImageSource
    void Tick(float fSeconds);
    void Render(const Vect2 &pos, const Vect2 &size);

    void ChangeScene();
    void UpdateSettings();
    Vect2 GetSize() const;
    
};