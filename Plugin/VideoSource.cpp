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
    audioOutputStreamHandler = nullptr;
    
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
            audioOutputStreamHandler = nullptr;
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

void *lock(void *data, void **pixelData)
{
    VideoSource *context = static_cast<VideoSource *>(data);

    *pixelData = context->pixelData;

    EnterCriticalSection(&context->textureLock);
    return NULL;
}

void unlock(void *data, void *id, void *const *pixelData)
{
    VideoSource *context = static_cast<VideoSource *>(data);

    context->GetTexture()->SetImage(*pixelData, GS_IMAGEFORMAT_BGRA, context->GetTexture()->Width() * 4);

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
    if (!texture || texture->Width() != *width || texture->Height() != *height) {
        if (texture) {
            delete texture;
            texture = nullptr;
        }

        texture = CreateTexture(*width, *height, GS_BGRA, nullptr, FALSE, FALSE);
    }   
    
    memcpy(chroma, CHROMA, sizeof(CHROMA) - 1);
    *pitches = *width * 4;
    *lines = *height;
    
    if (pixelData) {
        free(pixelData);
        pixelData = nullptr;
    }
    pixelData = calloc((*width) * (*height) * 4, 1);

    mediaWidthOffset = 0;
    mediaHeightOffset = 0;
    
    mediaWidth = *width;
    mediaHeight = *height;
    
    if (!config->isStretching) {
        float srcAspect = (float)*width / (float)*height;
        float dstAspect = (float)config->width / (float)config->height;

        if (srcAspect > dstAspect) {
            if(config->width != (*width) ) { //don't scale if size equal
                mediaWidth  = config->width;
                mediaHeight = static_cast<unsigned>(mediaWidth / srcAspect + 0.5);
            }
            mediaHeightOffset = (config->height - mediaHeight) / 2;
        } else {
            if( config->height != (*height) ) { //don't scale if size equal
                mediaHeight = config->height;
                mediaWidth  = static_cast<unsigned>(mediaHeight * srcAspect + 0.5);
            }
            mediaWidthOffset = (config->width - mediaWidth) / 2;
        }
    } else {
        mediaWidth = config->width;
        mediaHeight = config->height;
    }

    previousRenderSize.x = previousRenderSize.y = 0;
    
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
    
    if (previousRenderSize != size) {
        mediaOffset.x = (float)mediaWidthOffset;
        mediaOffset.y = (float)mediaHeightOffset;

        mediaSize.x = (float)mediaWidth;
        mediaSize.y = (float)mediaHeight;
        
        mediaSize += mediaOffset;

        Vect2 scale = size / videoSize;
        mediaOffset *= scale;
        mediaSize *= scale;

        previousRenderSize.x = size.x;
        previousRenderSize.y = size.y;
    }

    if (texture) {
        DrawSprite(texture, 0xFFFFFFFF, pos.x + mediaOffset.x, pos.y + mediaOffset.y, pos.x + mediaSize.x, pos.y + mediaSize.y);
    }
    LeaveCriticalSection(&textureLock);
}

void VideoSource::UpdateSettings()
{


    if (mediaPlayer) {
        libvlc_video_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr);
        libvlc_media_player_stop(mediaPlayer);
    }
    
    config->Reload();

    videoSize.x = float(config->width);
    videoSize.y = float(config->height);

    if (mediaPlayer == nullptr) {
        mediaPlayer = libvlc_media_player_new(vlc);
    }

    char *utf8PathOrUrl = config->pathOrUrl.CreateUTF8String();
    if (utf8PathOrUrl) {
        libvlc_media_t *media = libvlc_media_new_path(vlc, utf8PathOrUrl);
        libvlc_media_player_set_media(mediaPlayer, media);
        libvlc_media_release(media);
        Free(utf8PathOrUrl);
    }

    libvlc_video_set_callbacks(mediaPlayer, lock, unlock, display, this);
    libvlc_video_set_format_callbacks(mediaPlayer, videoFormatProxy, videoCleanupProxy);
    libvlc_audio_set_volume(mediaPlayer, config->volume);

    if (!audioOutputStreamHandler) {
        audioOutputStreamHandler = new AudioOutputStreamHandler(vlc, mediaPlayer);
    }

    audioOutputStreamHandler->SetAudioOutputParameters(config->audioOutputType, config->audioOutputDevice, config->isAudioOutputToStream);

    libvlc_media_player_play(mediaPlayer);
}

Vect2 VideoSource::GetSize() const 
{
    return videoSize;
}
