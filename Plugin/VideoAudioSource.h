/**
* John Bradley (jrb@turrettech.com)
*/
#pragma once

#include "OBSApi.h"
#include <stdint.h>
#include <vector>
#include <tuple>

struct libvlc_media_player_t;

struct AudioTimestamp {
    unsigned int count;
    int64_t pts;
};

class VideoAudioSource : public AudioSource
{
private:

    libvlc_media_player_t *mediaPlayer;

    unsigned int sampleSegmentSize;
    unsigned int sampleFrameCount;

    unsigned int bitsPerSample;
    unsigned int blockSize;
    unsigned int channelMask;
    unsigned int rate;
    unsigned int channels;

    CRITICAL_SECTION sampleBufferLock;

    std::vector<AudioTimestamp> sampleBufferPts;
    List<BYTE> sampleBuffer;
    List<BYTE> outputBuffer;

    int offset;

public:
    VideoAudioSource(unsigned int bitsPerSample, unsigned int blockSize, unsigned int channelMask, unsigned int rate, unsigned int channels);
    ~VideoAudioSource();


protected:
    virtual bool GetNextBuffer(void **buffer, UINT *numFrames, QWORD *timestamp);
    virtual void ReleaseBuffer();

    virtual CTSTR GetDeviceName() const;

public:
    void VideoAudioSource::PushAudio(const void *lpData, unsigned int size, int64_t pts);
};