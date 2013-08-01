/**
* John Bradley (jrb@turrettech.com)
*/
#include "AudioOutputStreamHandler.h"
#include "vlc.h"
#include "OBSApi.h"

AudioOutputStreamHandler::AudioOutputStreamHandler(
    libvlc_instance_t *vlc, 
    libvlc_media_player_t *mediaPlayer)
{
    this->vlc = vlc;
    this->mediaPlayer = mediaPlayer;
    this->isAudioOutputToStream = isAudioOutputToStream;
        
    libvlc_audio_set_format_callbacks(mediaPlayer, audioSetupCallbackProxy, audioCleanupCallbackProxy);        
    audioSource = nullptr;
}

AudioOutputStreamHandler::~AudioOutputStreamHandler()
{
    if (isAudioOutputToStream) {
        libvlc_audio_set_format_callbacks(mediaPlayer, nullptr, nullptr);
        libvlc_audio_set_callbacks(mediaPlayer, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
       
    if (audioSource) {
        delete audioSource;
        audioSource = nullptr;
    }
}

void AudioOutputStreamHandler::SetAudioOutputParameters(String type, String device, bool isAudioOutputToStream)
{
    this->isAudioOutputToStream = isAudioOutputToStream;

    if (isAudioOutputToStream) {
        libvlc_audio_set_callbacks(mediaPlayer, audioPlayCallbackProxy, nullptr, nullptr, nullptr, nullptr, this); 
    } else {
        auto *node = libvlc_audio_output_list_get(vlc);
        
        char *_type = type.CreateUTF8String();
        char *_device = device.CreateUTF8String();

        if (_type) {
            int i = libvlc_audio_output_set(mediaPlayer, _type);
            if (_device) {
                libvlc_audio_output_device_set(mediaPlayer, _type, _device);
                Free(_device);
            }
            Free(_type);    
        }
    }
}

int AudioOutputStreamHandler::AudioSetupCallback(char *format, unsigned int *rate, unsigned int *channels)
{
    if (audioSource) {
        delete audioSource;
        audioSource = nullptr;
    }

    if (isAudioOutputToStream) {
        memcpy(format, SAMP_FORMAT, sizeof(SAMP_FORMAT));

        unsigned int bitsPerSample = 16;
        unsigned int blockSize = 4;
        unsigned int channelMask = 0;

        *channels = 2;
        *rate = 44100;

        audioSource = new VideoAudioSource(bitsPerSample, blockSize, channelMask, *rate, *channels);

        return 0;
    } else {
        return 1;
    }
}

void AudioOutputStreamHandler::AudioCleanupCallback()
{

}

void AudioOutputStreamHandler::AudioPlayCallback(const void *samples, unsigned int count, int64_t pts)
{
    audioSource->PushAudio(samples, count, pts);
}
