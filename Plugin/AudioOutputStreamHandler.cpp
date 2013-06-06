#include "AudioOutputStreamHandler.h"
#include "vlc\vlc.h"
#include "OBSApi.h"

int AudioOutputStreamHandler::AudioSetupCallback(char *format, unsigned int *rate, unsigned int *channels)
{
    if (audioSource) {
        delete audioSource;
    }

    memcpy(format, SAMP_FORMAT, sizeof(SAMP_FORMAT));

    unsigned int bitsPerSample = 16;
    unsigned int blockSize = 4;
    unsigned int channelMask = 0;
    
    audioSource = new VideoAudioSource(bitsPerSample, blockSize, channelMask, *rate, *channels);

    return 0;
}

void AudioOutputStreamHandler::AudioCleanupCallback()
{

}

void AudioOutputStreamHandler::AudioPlayCallback(const void *samples, unsigned int count, int64_t pts)
{  
    audioSource->PushAudio(samples, count);
}

