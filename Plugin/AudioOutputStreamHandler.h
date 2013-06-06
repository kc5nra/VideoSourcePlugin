#pragma once

#include "VideoAudioSource.h"
#include "vlc\vlc.h"

#define SAMP_RATE 44100
#define SAMP_FORMAT "S16N"

class AudioOutputStreamHandler
{

private:
    libvlc_media_player_t *mediaPlayer;
    VideoAudioSource *audioSource;

public:
    AudioOutputStreamHandler::AudioOutputStreamHandler(libvlc_media_player_t *mediaPlayer)
    {
        this->mediaPlayer = mediaPlayer;
        
        libvlc_audio_set_format_callbacks(mediaPlayer, audioSetupCallbackProxy, audioCleanupCallbackProxy);
        libvlc_audio_set_callbacks(mediaPlayer, audioPlayCallbackProxy, nullptr, nullptr, nullptr, nullptr, this); 
        
        audioSource = nullptr;
    }

    ~AudioOutputStreamHandler()
    {

        libvlc_audio_set_format_callbacks(mediaPlayer, nullptr, nullptr);
        libvlc_audio_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        
        if (audioSource) {
            delete audioSource;
            audioSource = nullptr;
        }
    }

public: 
    // vlc setup callbacks

    static int audioSetupCallbackProxy(void **opaque, char *format, unsigned *rate, unsigned *channels)
    {
        return reinterpret_cast<AudioOutputStreamHandler *>(*opaque)->AudioSetupCallback(format, rate, channels);
    }

    static void audioCleanupCallbackProxy(void *opaque)
    {
        reinterpret_cast<AudioOutputStreamHandler *>(opaque)->AudioCleanupCallback();
    };

    int AudioSetupCallback(char *format, unsigned *rate, unsigned *channels);
    void AudioCleanupCallback();

    // vlc play callbacks
    static void audioPlayCallbackProxy(void *opaque, const void *samples, unsigned count, int64_t pts)
    {
        reinterpret_cast<AudioOutputStreamHandler *>(opaque)->AudioPlayCallback(samples, count, pts);
    }

     void AudioPlayCallback(const void *samples, unsigned count, int64_t pts);
};