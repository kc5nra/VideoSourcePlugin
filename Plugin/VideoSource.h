/**
* John Bradley (jrb@turrettech.com)
*/
#pragma once

#include "OBSApi.h"
#include "VideoSourcePlugin.h"
#include "vlc.h"

#include <vector>

#define CHROMA "RV32"

class AudioOutputStreamHandler;

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
    libvlc_media_list_player_t *mediaListPlayer;

    libvlc_media_list_t *mediaList;

    AudioOutputStreamHandler *audioOutputStreamHandler;

    Vect2 previousRenderSize;
    Vect2 mediaOffset;
    Vect2 mediaSize;



public:
    VideoSourceConfig *config;
    CRITICAL_SECTION textureLock;

    void *pixelData;

    unsigned int mediaWidth;
    unsigned int mediaHeight;
    unsigned int mediaWidthOffset;
    unsigned int mediaHeightOffset;

    // should only be set inside texture lock
    bool isRendering;

        
public:
    Texture *GetTexture() { return texture; }

public:
    // ImageSource
    void Tick(float fSeconds);
    void Render(const Vect2 &pos, const Vect2 &size);

    void ChangeScene();
    void UpdateSettings();
    Vect2 GetSize() const;

public:
    // Vlc

    static unsigned videoFormatProxy(
        void **opaque, 
        char *chroma,
        unsigned *width, 
        unsigned *height,
        unsigned *pitches, 
        unsigned *lines)
    { 
        return reinterpret_cast<VideoSource *>(*opaque)->VideoFormatCallback(chroma, width, height, pitches, lines); 
    }

    static void videoCleanupProxy(void *opaque)
    { 
        reinterpret_cast<VideoSource *>(opaque)->VideoFormatCleanup(); 
    };

    unsigned int VideoFormatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    void VideoFormatCleanup();

    
};