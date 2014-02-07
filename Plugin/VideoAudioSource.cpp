/**
* John Bradley (jrb@turrettech.com)
*/
#include "VideoAudioSource.h"

#include "vlc.h"

VideoAudioSource::VideoAudioSource(unsigned int bitsPerSample, unsigned int blockSize, unsigned int channelMask, unsigned int rate, unsigned int channels)
{
    InitializeCriticalSection(&sampleBufferLock);

    this->bitsPerSample = bitsPerSample;
    this->blockSize = blockSize;
    this->channelMask = channelMask;
    this->rate = rate;
    this->channels = channels;
    this->lastTimestamp = 0;

    sampleFrameCount   = this->rate / 100;
    sampleSegmentSize  = this->blockSize * sampleFrameCount;

    outputBuffer.SetSize(sampleSegmentSize);

    InitAudioData(false, channels, rate, bitsPerSample, blockSize, channelMask);
    
    OBSAddAudioSource(this);
}

VideoAudioSource::~VideoAudioSource()
{
    OBSRemoveAudioSource(this);
    DeleteCriticalSection(&sampleBufferLock);
}

bool VideoAudioSource::GetNextBuffer(void **buffer, UINT *numFrames, QWORD *timestamp)
{
    if(sampleBuffer.Num() >= sampleSegmentSize)
    {
        EnterCriticalSection(&sampleBufferLock);

        int64_t pts;
        int samplesProcessed = 0;
        while(samplesProcessed != sampleFrameCount) {
            int remaining = sampleFrameCount - samplesProcessed;
            AudioTimestamp &ts = sampleBufferPts[0];
            ts.count -= remaining;
            samplesProcessed += remaining;
            if (ts.count < 0) {
                samplesProcessed += ts.count;
                sampleBufferPts.erase(sampleBufferPts.begin());
            }
            pts = ts.pts / 1000;
        }

        mcpy(outputBuffer.Array(), sampleBuffer.Array(), sampleSegmentSize);
        sampleBuffer.RemoveRange(0, sampleSegmentSize);

        LeaveCriticalSection(&sampleBufferLock);

        *buffer = outputBuffer.Array();
        *numFrames = sampleFrameCount;
        if(!lastTimestamp || OBSGetAudioTime() > lastTimestamp) {
             *timestamp = OBSGetAudioTime();
        }
        else {
            *timestamp = lastTimestamp + 10;
        }

        lastTimestamp = *timestamp;
        
        // get the difference between vlc internal clock and audio clock
        int64_t vlcClockDiff = libvlc_delay(OBSGetAudioTime() * 1000);
        pts += (vlcClockDiff / 1000);

        //Log(TEXT("timestamp: %lld, pts: %llu, GetAudioTime(): %llu, orig pts: %llu"), *timestamp, pts, OBSGetAudioTime(), pts - (vlcClockDiff / 1000));
        
        // only set if in the future
        if (pts >= *timestamp) {
            *timestamp = pts;
        }
        
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


void VideoAudioSource::PushAudio(const void *lpData, unsigned int size, int64_t pts)
{
    if(lpData)
    {
        EnterCriticalSection(&sampleBufferLock);
        sampleBuffer.AppendArray(static_cast<const BYTE *>(lpData), size * blockSize);
        
        AudioTimestamp audioTimestamp;
        audioTimestamp.count = size * blockSize;
        audioTimestamp.pts = pts;

        sampleBufferPts.push_back(audioTimestamp);

        

        LeaveCriticalSection(&sampleBufferLock);
    }
}
