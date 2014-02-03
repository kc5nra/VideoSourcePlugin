/**
* John Bradley (jrb@turrettech.com)
*/
#pragma once

#include "OBSApi.h"
#include "vlc.h"

#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <propsys.h>
#include <Functiondiscoverykeys_devpkey.h>

#include <tuple>
#include <vector>

struct AudioOutputDevice 
{

private:
    String name;
    String longName;
    int index;

public:
    AudioOutputDevice(String name, String longName)
    {
        this->name = name;
        this->longName = longName;
    }

public:
    String &GetName() { return name; }
    String &GetLongName() { return longName; }
};

class AudioOutputType 
{

private:
    String name;
    String deviceSetName;
    String description;
    std::vector<AudioOutputDevice> audioOutputDevices;
    bool isEnabled;

public:
    AudioOutputType(String name, String deviceSetName, String description) 
    {
        this->name = name;
        this->deviceSetName = deviceSetName;
        this->description = description;
    }
    
public:
    void AddAudioOutputDevice(AudioOutputDevice &audioOutputDevice) 
    {
        audioOutputDevices.push_back(audioOutputDevice); 
    }

public:
    void SetName(String &name) { this->name = name; }
    String &GetName() { return name; }
    void SetDeviceSetName(String &deviceSetName) { this->deviceSetName = deviceSetName; }
    String &GetDeviceSetName() { return deviceSetName; }
    String &GetDescription() { return description; }

    std::vector<AudioOutputDevice> &GetAudioOutputDevices() { return audioOutputDevices; }
};

class VideoSourceConfig 
{

private:
    XElement *element;

    std::vector<AudioOutputType> audioOutputTypes;

public:
    unsigned int width;
    unsigned int height;
    bool isStretching;
    unsigned int volume;
    bool isAudioOutputToStream;
    String audioOutputType;
    String audioOutputTypeDevice;
    String audioOutputDevice;
    bool isPlaylistLooping;
    StringList playlist;
	String deinterlacing;

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
        width = 640;
        height = 480;
        volume = 100;
        isStretching = false;
        isAudioOutputToStream = true;
        audioOutputType = TEXT("wavemapper");
        audioOutputTypeDevice = TEXT("wavemapper");
        audioOutputDevice = TEXT("");
        isPlaylistLooping = false;
		deinterlacing = TEXT("none");
    }

    void Reload()
    {
        width = element->GetInt(TEXT("width"));
        height = element->GetInt(TEXT("height"));
        volume = element->GetInt(TEXT("volume"));
        isStretching = element->GetInt(TEXT("isStretching")) == 1;
        isAudioOutputToStream = element->GetInt(TEXT("isAudioOutputToStream")) == 1;
        audioOutputType = element->GetString(TEXT("audioOutputType"));
        audioOutputTypeDevice = element->GetString(TEXT("audioOutputTypeDevice"));
        audioOutputDevice = element->GetString(TEXT("audioOutputDevice"));
        isPlaylistLooping = element->GetInt(TEXT("isPlaylistLooping")) == 1;
        playlist.Clear();
        element->GetStringList(TEXT("playlist"), playlist);
		deinterlacing = element->GetString(TEXT("deinterlacing"));
    }

    void Save()
    {
        element->SetInt(TEXT("width"), width);
        element->SetInt(TEXT("height"), height);
        element->SetInt(TEXT("volume"), volume);
        element->SetInt(TEXT("isStretching"), isStretching ? 1 : 0);
        element->SetInt(TEXT("isAudioOutputToStream"), isAudioOutputToStream ? 1 : 0);
        element->SetString(TEXT("audioOutputType"), audioOutputType);
        element->SetString(TEXT("audioOutputTypeDevice"), audioOutputTypeDevice);
        element->SetString(TEXT("audioOutputDevice"), audioOutputDevice);
        element->SetInt(TEXT("isPlaylistLooping"), isPlaylistLooping ? 1 : 0);
        element->SetStringList(TEXT("playlist"), playlist);
		element->SetString(TEXT("deinterlacing"), deinterlacing);
    }

    void InitializeAudioOutputVectors(libvlc_instance_t *vlc) {
        audioOutputTypes.clear();
        
        
        libvlc_audio_output_t *typeList = libvlc_audio_output_list_get(vlc);
        libvlc_audio_output_t *typeListNode = typeList;
        while(typeListNode) {
            AudioOutputType audioOutputType(typeListNode->psz_name, typeListNode->psz_name, typeListNode->psz_description);
#ifdef VLC2X
            if (audioOutputType.GetName() == "waveout" || audioOutputType.GetName() == "directsound" || audioOutputType.GetName() == "adummy") {
                char *device = typeListNode->psz_name;
                // fix directx reporting wrong device name
                // wtf? bad vlc, bad
                // is there a step I'm missing from resolving aout_directx -> directx?
                if (audioOutputType.GetName() == "directsound") {
                    audioOutputType.SetDeviceSetName(String("directx"));
                    device = "directx";
                }

                // Why does this not work?
                libvlc_audio_output_device_t *deviceList = libvlc_audio_output_device_list_get(vlc, device);
                libvlc_audio_output_device_t *deviceListNode = deviceList;
                while(deviceListNode) {
                    audioOutputType.AddAudioOutputDevice(AudioOutputDevice(deviceListNode->psz_description, deviceListNode->psz_device));
                    deviceListNode = deviceListNode->p_next;
                }
                libvlc_audio_output_device_list_release(deviceList);
                audioOutputTypes.push_back(audioOutputType);
            }
#else
            if (audioOutputType.GetName() == "waveout" || audioOutputType.GetName() == "adummy" || audioOutputType.GetName() == "aout_directx") {
                char *device = typeListNode->psz_name;

                // fix directx reporting wrong device name
                // wtf? bad vlc, bad
                // is there a step I'm missing from resolving aout_directx -> directx?
                if (audioOutputType.GetName() == "aout_directx") {
                    audioOutputType.SetName(String("directx"));
                    device = "directx";
                }

                for(int i = 0; i < libvlc_audio_output_device_count(vlc, device); i++) {
                    char *longName = libvlc_audio_output_device_longname(vlc, device, i);
                    audioOutputType.AddAudioOutputDevice(AudioOutputDevice(longName, longName));
                    libvlc_free(longName);
                }
                audioOutputTypes.push_back(audioOutputType);
            }
#endif
            typeListNode = typeListNode->p_next;
        }
        libvlc_audio_output_list_release(typeList);
                
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