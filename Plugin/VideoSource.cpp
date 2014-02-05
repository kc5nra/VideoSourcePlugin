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
    isInScene = true;
    //globalSourceRefCount = 1;
    //isRenderingWithoutRef = false;

    config = new VideoSourceConfig(data);
    InitializeCriticalSection(&textureLock);
    UpdateSettings();
}

VideoSource::~VideoSource()
{ 

    // media list and media list player
    libvlc_media_list_player_stop(mediaListPlayer);
    libvlc_media_list_player_release(mediaListPlayer);
    libvlc_media_list_release(mediaList);

    // media player
    libvlc_video_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr);
    libvlc_media_player_stop(mediaPlayer);
        

    delete audioOutputStreamHandler;
    audioOutputStreamHandler = nullptr;


    libvlc_media_player_release(mediaPlayer);

    if (pixelData) {
        free(pixelData);
        pixelData = nullptr;
    }

    if (texture) {
        delete texture;
        texture = nullptr;
    }

    delete config;
    config = nullptr;

    DeleteCriticalSection(&textureLock);
}

void *lock(void *data, void **pixelData)
{
    VideoSource *_this = static_cast<VideoSource *>(data);

    *pixelData = _this->pixelData;

    EnterCriticalSection(&_this->textureLock);
    return NULL;
}

void unlock(void *data, void *id, void *const *pixelData)
{
    VideoSource *_this = static_cast<VideoSource *>(data);

    if (!_this->hasSetVolume)
    {
        _this->hasSetVolume |= libvlc_audio_set_volume(_this->mediaPlayer, _this->config->volume) == 0;
    }

    if (_this->isInScene) {
        _this->GetTexture()->SetImage(*pixelData, GS_IMAGEFORMAT_BGRA, _this->GetTexture()->Width() * 4);
    }

    LeaveCriticalSection(&_this->textureLock);
    
    assert(id == NULL); /* picture identifier, not needed here */
}

void display(void *data, void *id)
{
}

static void vlcEvent(const libvlc_event_t *e, void *data)
{
    VideoSource *_this = reinterpret_cast<VideoSource *>(data);
        
    if (e->type == libvlc_MediaPlayerEndReached) {
        // clear the texture and memory data 
        // because we can still renders from the video
        // and OBS that could fetch old data
        EnterCriticalSection(&_this->textureLock);
        
        if (!_this->config->isPlaylistLooping && !--_this->remainingVideos) {
            _this->isRendering = false;
            _this->ClearTexture();
        }

        LeaveCriticalSection(&_this->textureLock);
    } else if (e->type == libvlc_MediaPlayerPlaying) {
        EnterCriticalSection(&_this->textureLock);
        _this->isRendering = true;
        LeaveCriticalSection(&_this->textureLock);
    }

}

unsigned VideoSource::VideoFormatCallback(
    char *chroma,
    unsigned *width, 
    unsigned *height,
    unsigned *pitches, 
    unsigned *lines)
{
        
    memcpy(chroma, CHROMA, sizeof(CHROMA) - 1);
    *pitches = *width * 4;
    *lines = *height;
    
    EnterCriticalSection(&textureLock);

    if (!texture || texture->Width() != *width || texture->Height() != *height) {
        if (texture) {
            delete texture;
            texture = nullptr;
        }
        
        if (pixelData) {
            free(pixelData);
            pixelData = nullptr;
        }
    }
    
    if (!texture) {
        texture = CreateTexture(*width, *height, GS_BGRA, nullptr, FALSE, FALSE);
    }

    if (!pixelData) {
        pixelData = calloc((*width) * (*height) * 4, 1);
    }

    LeaveCriticalSection(&textureLock);

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

void VideoSource::GlobalSourceEnterScene()
{
    isInScene = true;
}

void VideoSource::GlobalSourceLeaveScene()
{
    isInScene = false;
}

// you must lock the texture before you call this
void VideoSource::ClearTexture()
{
    BYTE *lpData;
    UINT pitch;
    if (texture) {
        texture->Map(lpData, pitch);
        memset(lpData, 0, pitch * texture->Height());
        memset(pixelData, 0, (texture->Width() * 4) * texture->Height());
        texture->Unmap();
    }
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

    if (texture && isRendering) {
        DrawSprite(texture, 0xFFFFFFFF, pos.x + mediaOffset.x, pos.y + mediaOffset.y, pos.x + mediaSize.x, pos.y + mediaSize.y);
    }

    LeaveCriticalSection(&textureLock);
}

void VideoSource::UpdateSettings()
{
    
    EnterCriticalSection(&textureLock);
    isRendering = false;
    LeaveCriticalSection(&textureLock);

    if (mediaPlayer) {
        libvlc_video_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr);
        libvlc_media_player_stop(mediaPlayer);
    }
    
    config->Reload();

    hasSetVolume = false;

    videoSize.x = float(config->width);
    videoSize.y = float(config->height);

    if (mediaPlayer == nullptr) {
        mediaPlayer = libvlc_media_player_new(vlc);
        libvlc_event_manager_t *em = libvlc_media_player_event_manager(mediaPlayer);
        libvlc_event_attach(em, libvlc_MediaPlayerEndReached, vlcEvent, this);
        libvlc_event_attach(em, libvlc_MediaPlayerPlaying, vlcEvent, this);
    }

    if (mediaListPlayer == nullptr) {
        mediaListPlayer = libvlc_media_list_player_new(vlc);
        libvlc_media_list_player_set_media_player(mediaListPlayer, mediaPlayer);
    } else {
        libvlc_media_list_player_stop(mediaListPlayer);
    }

    if (mediaList) {
        libvlc_media_list_lock(mediaList);
        while(libvlc_media_list_count(mediaList)) {
            libvlc_media_list_remove_index(mediaList, 0);
        }
        libvlc_media_list_unlock(mediaList);
    } else {
        mediaList = libvlc_media_list_new(vlc);
        libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);
    }

    

    char *utf8PathOrUrl;
    for(unsigned int i = 0; i < config->playlist.Num(); i++) {
        String &mediaEntry = config->playlist[i];
        String token = mediaEntry.GetToken(1, L':');
     
        // .. Yup.
        bool isStream = token.Length() >= 2 && token[0] == L'/' && token[1] == L'/';
        utf8PathOrUrl = config->playlist[i].CreateUTF8String();
        if (utf8PathOrUrl) {
            libvlc_media_t *media;
            if (!isStream) {
                media = libvlc_media_new_path(vlc, utf8PathOrUrl);
            } else {
                media = libvlc_media_new_location(vlc, utf8PathOrUrl);
            }
            
            libvlc_media_list_lock(mediaList);
            libvlc_media_list_add_media(mediaList, media);
            libvlc_media_list_unlock(mediaList);

            libvlc_media_release(media);
            Free(utf8PathOrUrl);
        }
    }

    if (!config->isPlaylistLooping) {
        remainingVideos = config->playlist.Num();
    }

    libvlc_video_set_callbacks(mediaPlayer, lock, unlock, display, this);
    libvlc_video_set_format_callbacks(mediaPlayer, videoFormatProxy, videoCleanupProxy);

    libvlc_media_list_player_set_playback_mode(mediaListPlayer, config->isPlaylistLooping ? libvlc_playback_mode_loop : libvlc_playback_mode_default);
    
    if(config->deinterlacing == TEXT("none")) {
        libvlc_video_set_deinterlace(mediaPlayer,NULL);
    }
    else {
        LPSTR deinterlacingUTF8 = config->deinterlacing.CreateUTF8String();
        libvlc_video_set_deinterlace(mediaPlayer, deinterlacingUTF8);
        Free(deinterlacingUTF8);
    }

    if (!audioOutputStreamHandler) {
        audioOutputStreamHandler = new AudioOutputStreamHandler(vlc, mediaPlayer);
    }

    audioOutputStreamHandler->SetOutputParameters(
        config->audioOutputType, 
        config->audioOutputTypeDevice,
        config->audioOutputDevice, 
        config->isAudioOutputToStream);
    
    audioOutputStreamHandler->SetVolume(config->volume);
    
    // set (possibly in vane) the volume.  If it doesn't work it will try later until it works
    // vlc... que pasa amigo
    hasSetVolume = libvlc_audio_set_volume(mediaPlayer, config->volume) == 0;
    
    EnterCriticalSection(&textureLock);
    isRendering = true;
    LeaveCriticalSection(&textureLock);

    libvlc_media_list_player_play(mediaListPlayer);
    
}

Vect2 VideoSource::GetSize() const 
{
    return videoSize;
}
