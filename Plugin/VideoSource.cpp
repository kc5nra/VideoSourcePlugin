/**
* John Bradley (jrb@turrettech.com)
*/

#include "VideoSource.h"
#include "VideoSourcePlugin.h"

VideoSource::VideoSource(XElement *data)
{
	Log(TEXT("Using Browser Source"));

    vlc = VideoSourcePlugin::instance->GetVlc();

    config = new VideoSourceConfig(data);
    InitializeCriticalSection(&textureLock);
    UpdateSettings();	
}

VideoSource::~VideoSource()
{ 
    if (mediaPlayer) {
        libvlc_media_player_stop(mediaPlayer);
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

static void *lock(void *data, void **p_pixels)
{
    VideoSource *context = static_cast<VideoSource *>(data);

    *p_pixels = context->pixelData;

    EnterCriticalSection(&context->textureLock);
    return NULL;
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
    VideoSource *context = static_cast<VideoSource *>(data);

    context->GetTexture()->SetImage(*p_pixels, GS_IMAGEFORMAT_BGRA, context->config->width * 4);

    LeaveCriticalSection(&context->textureLock);
    
    assert(id == NULL); /* picture identifier, not needed here */
}

static void display(void *data, void *id)
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
    config->Reload();
   
    if (pixelData) {
        free(pixelData);
        pixelData = nullptr;
    }
    pixelData = malloc(config->width * config->height * 4);

    if (texture) {
        delete texture;
        texture = nullptr;
    }
    texture = CreateTexture(config->width, config->height, GS_BGRA, nullptr, FALSE, FALSE);

    videoSize.x = float(config->width);
    videoSize.y = float(config->height);

    char *url = "c:\\movie.m4v";

    if (mediaPlayer) {
        libvlc_media_player_stop(mediaPlayer);
    } else {
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
    
    libvlc_media_player_play(mediaPlayer);
}

Vect2 VideoSource::GetSize() const 
{
    return videoSize;
}
