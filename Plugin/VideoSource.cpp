/**
* John Bradley (jrb@turrettech.com)
*/

#include "VideoSource.h"
#include "AudioOutputStreamHandler.h"
#include "VideoSourcePlugin.h"
#include <process.h>

VideoSource::VideoSource(XElement *data)
{
	Log(TEXT("Using Video Source"));

    vlc = VideoSourcePlugin::instance->GetVlc();
    mediaPlayer = nullptr;
    pixelData = nullptr;
    
    config = new VideoSourceConfig(data);
    InitializeCriticalSection(&textureLock);
    UpdateSettings();	
}

VideoSource::~VideoSource()
{ 


    if (mediaPlayer) {
        libvlc_video_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr);
        libvlc_media_player_stop(mediaPlayer);
        
        if (audioOutputStreamHandler) {
            delete audioOutputStreamHandler;
        }

        libvlc_media_player_release(mediaPlayer);
    }


    if (pixelData) {
        free(pixelData);
        pixelData = nullptr;
    }

    if (texture) {
        delete texture;
        texture = nullptr;
    }

    delete config;
   
    DeleteCriticalSection(&textureLock);

    config = nullptr;
}

void *lock(void *data, void **p_pixels)
{
    VideoSource *context = static_cast<VideoSource *>(data);

    *p_pixels = context->pixelData;

    EnterCriticalSection(&context->textureLock);
    return NULL;
}

void unlock(void *data, void *id, void *const *p_pixels)
{
    VideoSource *context = static_cast<VideoSource *>(data);

    BYTE *textureData;
    unsigned int pitch;
    context->GetTexture()->Map(textureData, pitch);

    memset(textureData, 0, pitch * context->config->height);

    for(unsigned int y = 0; y < context->mediaHeight; y++)
    {
        LPBYTE curInput  = ((LPBYTE)*p_pixels) + ((context->mediaWidth * 4) * y);
        LPBYTE curOutput = ((LPBYTE)textureData) + (pitch * (y + context->mediaHeightOffset));

        mcpy(curOutput + (context->mediaWidthOffset * 4), curInput, context->mediaWidth * 4);
    }

    context->GetTexture()->Unmap();

    LeaveCriticalSection(&context->textureLock);
    
    assert(id == NULL); /* picture identifier, not needed here */
}

void display(void *data, void *id)
{
}


unsigned VideoSource::VideoFormatCallback(
    char *chroma,
    unsigned *width, 
    unsigned *height,
    unsigned *pitches, 
    unsigned *lines)
{


    mediaWidthOffset = 0;
    mediaWidthOffset = 0;

    if (!config->isStretching) {

        float srcAspect = (float)(*width) / (float)(*height);
        float dstAspect = (float)config->width / (float)config->height;
        if (srcAspect > dstAspect) {
            if(config->width != (*width) ) { //don't scale if size equal
                *width  = config->width;
                *height = static_cast<unsigned>( (*width) / srcAspect + 0.5);
            }
            mediaHeightOffset = (config->height - *height) / 2;
        } else {
            if( config->height != (*height) ) { //don't scale if size equal
                *height = config->height;
                *width  = static_cast<unsigned>( (*height) * srcAspect + 0.5); 
            }
            mediaWidthOffset = (config->width - *width) / 2;
        }
    } else {
        *width = config->width;
        *height = config->height;
    }
    
    mediaWidth = *width;
    mediaHeight = *height;

    memcpy(chroma, CHROMA, sizeof(CHROMA)-1);
    (*pitches) = mediaWidth * 4;
    (*lines)   = mediaHeight;

    if (pixelData) {
        free(pixelData);
        pixelData = nullptr;
    }
    pixelData = calloc(mediaWidth * mediaHeight * 4, 1);

    return 1;
}

void VideoSource::VideoFormatCleanup()
{
}


void VideoSource::Tick(float fSeconds)
{
}

void VideoSource::Render(const Vect2 &pos, const Vect2 &size)
{
    EnterCriticalSection(&textureLock);
    if (texture) {
        DrawSprite(texture, 0xFFFFFFFF, pos.x, pos.y, pos.x + size.x, pos.y + size.y);
    }
    LeaveCriticalSection(&textureLock);
}




void VideoSource::UpdateSettings()
{


    if (mediaPlayer) {
        libvlc_video_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr);
        libvlc_media_player_stop(mediaPlayer);
    }

    if (audioOutputStreamHandler) {
        delete audioOutputStreamHandler;
    }

    config->Reload();

    if (texture) {
        delete texture;
        texture = nullptr;
    }
    texture = CreateTexture(config->width, config->height, GS_BGRA, nullptr, FALSE, FALSE);

    BYTE *textureData;
    unsigned int pitch;
    
    texture->Map(textureData, pitch);
    memset(textureData, 0, pitch * config->height);
    texture->Unmap();

    videoSize.x = float(config->width);
    videoSize.y = float(config->height);

    if (mediaPlayer == nullptr) {
        mediaPlayer = libvlc_media_player_new(vlc);
        libvlc_video_set_callbacks(mediaPlayer, lock, unlock, display, this);
    }

    char *utf8PathOrUrl = config->pathOrUrl.CreateUTF8String();
    if (utf8PathOrUrl) {
        libvlc_media_t *media = libvlc_media_new_path(vlc, utf8PathOrUrl);
        libvlc_media_player_set_media(mediaPlayer, media);
        libvlc_media_release(media);
        Free(utf8PathOrUrl);
    }
    
    libvlc_video_set_format(mediaPlayer, "RV32", config->width, config->height, config->width * 4);
    libvlc_video_set_format_callbacks(mediaPlayer, videoFormatProxy, videoCleanupProxy);
    libvlc_audio_set_volume(mediaPlayer, config->volume);

     if (config->isAudioOutputToStream) {
        audioOutputStreamHandler = new AudioOutputStreamHandler(mediaPlayer);
    }
    

    libvlc_media_player_play(mediaPlayer);
}

Vect2 VideoSource::GetSize() const 
{
    return videoSize;
}
