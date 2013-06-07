#pragma once

#include "OBSApi.h"
#include "vlc.h"

#include <tuple>
#include <vector>

struct AudioOutputDevice 
{

private:
    String name;
    String longName;
    int index;

public:
    AudioOutputDevice(String name, String longName, int index)
    {
        this->name = name;
        this->longName = longName;
        this->index = index;
    }

public:
    String &GetName() { return name; }
    String &GetLongName() { return longName; }
    int GetIndex() { return index; }

};

class AudioOutputType 
{

private:
    String name;
    String description;
    std::vector<AudioOutputDevice> audioOutputDevices;
    bool isEnabled;

public:
    AudioOutputType(String name, String description) 
    {
        this->name = name;
        this->description = description;
    }
    
public:
    void AddAudioOutputDevice(AudioOutputDevice &audioOutputDevice) 
    {
        audioOutputDevices.push_back(audioOutputDevice); 
    }

public:
    String &GetName() { return name; }
    String &GetDescription() { return description; }

    std::vector<AudioOutputDevice> &GetAudioOutputDevices() { return audioOutputDevices; }
};

class VideoSourceConfig 
{

private:
    XElement *element;

    std::vector<AudioOutputType> audioOutputTypes;

public:
    String pathOrUrl;
    unsigned int width;
    unsigned int height;
    bool isStretching;
    unsigned int volume;
    bool isAudioOutputToStream;
    String audioOutputType;
    String audioOutputDevice;

public:
    VideoSourceConfig(XElement *element)
    {
        this->element = element;
        Reload();
    }

    ~VideoSourceConfig()
    {
    }

public:
    

    void Populate()
    {
        pathOrUrl = TEXT("");
        width = 640;
        height = 480;
        volume = 100;
        isStretching = false;
        isAudioOutputToStream = true;
        audioOutputType = TEXT("wavemapper");
        audioOutputDevice = TEXT("");
    }

    void Reload()
    {
        pathOrUrl = element->GetString(TEXT("pathOrUrl"));
        width = element->GetInt(TEXT("width"));
        height = element->GetInt(TEXT("height"));
        volume = element->GetInt(TEXT("volume"));
        isStretching = element->GetInt(TEXT("isStretching")) == 1;
        isAudioOutputToStream = element->GetInt(TEXT("isAudioOutputToStream")) == 1;
        audioOutputType = element->GetString(TEXT("audioOutputType"));
        audioOutputDevice = element->GetString(TEXT("audioOutputDevice"));
    }

    void Save()
    {
        element->SetString(TEXT("pathOrUrl"), pathOrUrl);
        element->SetInt(TEXT("width"), width);
        element->SetInt(TEXT("height"), height);
        element->SetInt(TEXT("volume"), volume);
        element->SetInt(TEXT("isStretching"), isStretching ? 1 : 0);
        element->SetInt(TEXT("isAudioOutputToStream"), isAudioOutputToStream ? 1 : 0);
        element->SetString(TEXT("audioOutputType"), audioOutputType);
        element->SetString(TEXT("audioOutputDevice"), audioOutputDevice);
    }

    void InitializeAudioOutputVectors(libvlc_instance_t *vlc) {
        audioOutputTypes.clear();
        
        libvlc_audio_output_t *list = libvlc_audio_output_list_get(vlc);
        libvlc_audio_output_t *listNode = list;
        while(listNode) {
            AudioOutputType audioOutputType(listNode->psz_name, listNode->psz_description);
            if (audioOutputType.GetName() == "waveout" || audioOutputType.GetName() == "aout_directx" || audioOutputType.GetName() == "adummy") {
                for(int i = 0; i < libvlc_audio_output_device_count(vlc, listNode->psz_name); i++) {
                    char *id = libvlc_audio_output_device_id(vlc, listNode->psz_name, i);
                    char *longName = libvlc_audio_output_device_longname(vlc, listNode->psz_name, i);
                    audioOutputType.AddAudioOutputDevice(AudioOutputDevice(id, longName, i));
                    libvlc_free(id);
                    libvlc_free(longName);
                }
                audioOutputTypes.push_back(audioOutputType);
            }
            listNode = listNode->p_next;
        }
        libvlc_audio_output_list_release(list);
                
    }

    std::vector<AudioOutputType> &GetAudioOutputTypes()
    {
        return audioOutputTypes;
    }

    AudioOutputType &GetAudioOutputType(String name)
    {
        for(auto i = audioOutputTypes.begin(); i < audioOutputTypes.end(); i++)
        {
            AudioOutputType &type = *i;
            if ((*i).GetName() == name) {
                return type;
            }
        }

        // device not found, return wavemapper
        // how could this happen? device disappeared/unplugged
        assert(audioOutputTypes.size());
        return audioOutputTypes[0];
    }
};