#ifndef WINDOWS_AUDIO_H
#define WINDOWS_AUDIO_H


#include <DSound.h>
#include <Mfidl.h>
#include <Mfapi.h>
#include <Objbase.h>
#include <Mfreadwrite.h>

#include "util_audio.h"

struct audioMixer{
    int samplesPerSecond;
    int bytesPerSample;
    int bufferSize;
    int bytesPlayed;
    LPDIRECTSOUNDBUFFER talkbuffer;
    DWORD playCursor;
    DWORD writeCursor;
    bool played;
    int bytesLoaded;
    Audio * target;
    bool talkplaying;
};

audioMixer mixer;


enum AudioType{
    AudioType_PlayTalk,
    AudioType_StopTalk,
};

struct AudioItem{
    AudioType type;
    AudioItem * next;
    Audio * target;
};



#endif