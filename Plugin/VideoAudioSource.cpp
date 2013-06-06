
#include "VideoAudioSource.h"

#include "vlc\vlc.h"

VideoAudioSource::VideoAudioSource(unsigned int bitsPerSample, unsigned int blockSize, unsigned int channelMask, unsigned int rate, unsigned int channels)
{
    InitializeCriticalSection(&sampleBufferLock);

    API->AddAudioSource(this);

    this->bitsPerSample = bitsPerSample;
    this->blockSize = blockSize;
    this->channelMask = channelMask;
    this->rate = rate;
    this->channels = channels;

    sampleFrameCount   = this->rate / 100;
    sampleSegmentSize  = this->blockSize * sampleFrameCount;

    outputBuffer.SetSize(sampleSegmentSize);

    InitAudioData(false, channels, rate, bitsPerSample, blockSize, channelMask);
}

VideoAudioSource::~VideoAudioSource()
{
    API->RemoveAudioSource(this);
    DeleteCriticalSection(&sampleBufferLock);
}

bool VideoAudioSource::GetNextBuffer(void **buffer, UINT *numFrames, QWORD *timestamp)
{
    if(sampleBuffer.Num() >= sampleSegmentSize)
    {
        EnterCriticalSection(&sampleBufferLock);

        mcpy(outputBuffer.Array(), sampleBuffer.Array(), sampleSegmentSize);
        sampleBuffer.RemoveRange(0, sampleSegmentSize);

        LeaveCriticalSection(&sampleBufferLock);

        *buffer = outputBuffer.Array();
        *numFrames = sampleFrameCount;
        *timestamp = API->GetAudioTime();

        return true;
    }

    return false;
}

void VideoAudioSource::ReleaseBuffer()
{
}



CTSTR VideoAudioSource::GetDeviceName() const
{
    return NULL;
}


void VideoAudioSource::PushAudio(const void *lpData, unsigned int size)
{
    if(lpData)
    {
        EnterCriticalSection(&sampleBufferLock);
        sampleBuffer.AppendArray(static_cast<const BYTE *>(lpData), size);
        LeaveCriticalSection(&sampleBufferLock);
    }
}

//void VideoAudioSource::FlushSamples()
//{
//    EnterCriticalSection(&sampleBufferLock);
//    sampleBuffer.Clear();
//    LeaveCriticalSection(&sampleBufferLock);
//}

