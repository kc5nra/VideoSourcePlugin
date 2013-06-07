#pragma once

#include "VideoAudioSource.h"
#include "vlc.h"

#define SAMP_RATE 44100
#define SAMP_FORMAT "S16N"

class AudioOutputStreamHandler
{

private:
    libvlc_instance_t *vlc;
    libvlc_media_player_t *mediaPlayer;

    VideoAudioSource *audioSource;
    bool isAudioOutputToStream; 

public:

    AudioOutputStreamHandler::AudioOutputStreamHandler(libvlc_instance_t *vlc, libvlc_media_player_t *mediaPlayer);
    ~AudioOutputStreamHandler();

public:
    void SetAudioOutputParameters(String type, String device, bool isAudioOutputToStream);

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