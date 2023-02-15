#ifndef WINDOWS_AUDIO_CPP
#define WINDOWS_AUDIO_CPP

#include <DSound.h>
#include "util_audio.h"

static LPDIRECTSOUNDBUFFER primarybuffer;
static DWORD bufferSize;
static DWORD lastWriteCursor;
#define samplesPerSecond 44100

struct Audio{
    char * data;
    DWORD byteSize;
    DWORD commitedBytes;
    bool startedPlaying;
};

static Audio c;
static Audio * mixedSounds[256];
static i32 mixedSoundsCount;

bool initAudio(HWND target){
    
    LPDIRECTSOUND8 d8;
    if(SUCCEEDED(DirectSoundCreate8(0, &d8, 0))){
        if(SUCCEEDED(d8->SetCooperativeLevel(target, DSSCL_WRITEPRIMARY))){
            
            WAVEFORMATEX wf = {};
            wf.wFormatTag = WAVE_FORMAT_PCM;
            wf.nChannels = 2;
            wf.nSamplesPerSec = samplesPerSecond;
            wf.wBitsPerSample = 16;
            wf.nBlockAlign = (wf.nChannels*wf.wBitsPerSample) / 8;
            wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
            wf.cbSize = 0;
            
            DSBUFFERDESC pbuffdesc= {};
            pbuffdesc.dwSize = sizeof(DSBUFFERDESC);
            pbuffdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
            
            
            if(SUCCEEDED(d8->CreateSoundBuffer(&pbuffdesc, &primarybuffer, 0))){
                if(!SUCCEEDED(primarybuffer->SetFormat(&wf))){
                    return false;
                }
            }else{
                return false;
            }      
    

            DSBCAPS caps = {};
            caps.dwSize = sizeof(DSBCAPS);
            HRESULT r = primarybuffer->GetCaps(&caps);
            ASSERT(r == DS_OK);
            bufferSize = caps.dwBufferBytes;

            void * data1 = NULL;
            void * data2 = NULL;
            DWORD size1 = 0;
            DWORD size2 = 0;

            r = primarybuffer->Lock(0, bufferSize, &data1, &size1, &data2, &size2, 0);
            ASSERT(r == DS_OK);
            ASSERT(data1 != NULL);
            ASSERT(size1 == bufferSize);
            for (DWORD i = 0; i < size1; i++){
                CAST(char*, data1)[i] = 5;
            }
            r = primarybuffer->Unlock(data1, size1, data2, size2);
            ASSERT(r == DS_OK);

            r = primarybuffer->Play(0, 0, DSBPLAY_LOOPING);
            ASSERT(r == DS_OK);
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
    return true;
}

void loadAudio(){
    nint size = samplesPerSecond * sizeof(i16) * 2;
    char * secondOfC = &PPUSHA(char, size);
    for (DWORD sample = 0; sample < i32(samplesPerSecond); sample++){
        i16 value = CAST(i16, sin(fmod((CAST(f32, sample)/samplesPerSecond)*256*2*PI, 2*PI)) * 30000) + 3;
        CAST(i16*, secondOfC)[sample*2] = value;
        CAST(i16*, secondOfC)[sample*2+1] = value;
    }
    c.data = secondOfC;
    c.byteSize = CAST(DWORD, size);
} 

void playAudio(){
    ASSERT(mixedSoundsCount <= ARRAYSIZE(mixedSounds));
    mixedSounds[mixedSoundsCount++] = &c;
}

void mix(){
    DWORD playCursor;
    DWORD writeCursor;
    HRESULT r = primarybuffer->GetCurrentPosition(&playCursor, &writeCursor);
    ASSERT(r == DS_OK);
    // TODO some tolerance, dont need to write after 1 byte was played
    if(writeCursor == lastWriteCursor){
        return;
    }

    void * data1 = NULL;
    void * data2 = NULL;
    DWORD size1 = 0;
    DWORD size2 = 0;
    ASSERT(bufferSize != 0);
    // TODO dont need to write whole buffer just now, perhaps smaller part is ok
    DWORD toLock = 0;
    if (playCursor <= writeCursor)
    {
        ASSERT(bufferSize >= writeCursor);
        toLock = playCursor + (bufferSize - writeCursor);
    }
    else{
        toLock = playCursor - writeCursor;
    }

    DWORD bytesCommited = 0;
    {
        if (writeCursor >= lastWriteCursor){
            bytesCommited = writeCursor - lastWriteCursor;
        }
        else{
            ASSERT(bufferSize >= lastWriteCursor);
            bytesCommited = writeCursor + (bufferSize - lastWriteCursor);
        }
    }
    lastWriteCursor = writeCursor;

    r = primarybuffer->Lock(writeCursor, toLock, &data1, &size1, &data2, &size2, 0);
    ASSERT(r == DS_OK);
    ASSERT(data1 != NULL);
    ASSERT(toLock == size1 + size2);

    for(i32 i = 0; i < mixedSoundsCount; i++)
    {
        Audio * track = mixedSounds[i];
        if (track->startedPlaying){
            track->commitedBytes = MIN(track->byteSize, track->commitedBytes + bytesCommited);
        }
        ASSERT(track->commitedBytes <= track->byteSize);
    }

    ASSERT(size1%4 == 0);
    // TODO condense
    for (DWORD i = 0; i < size1; i += 4)
    {
        i32 sampleValueL = 0;
        i32 sampleValueR = 0;
        for(i32 s = 0; s < mixedSoundsCount; s++)
        {
            Audio * track = mixedSounds[s];
            ASSERT(track->commitedBytes%4 == 0);
            ASSERT(track->byteSize%4 == 0);
            if (track->commitedBytes + 2*i + 4 < track->byteSize){
                sampleValueL += *CAST(i16*,&track->data[track->commitedBytes + i]);
                sampleValueR += *CAST(i16*,&track->data[track->commitedBytes + i + 2]);
            }
        }
        ((i16*)data1)[i/2] = CAST(i16, sampleValueL/mixedSoundsCount);
        ((i16*)data1)[i/2 + 1] = CAST(i16, sampleValueR/mixedSoundsCount);
    }
    ASSERT(size2%4 == 0);
    for (DWORD i = 0; i < size2; i += 4)
    {
        i32 sampleValueL = 0;
        i32 sampleValueR = 0;
        for(i32 s = 0; s < mixedSoundsCount; s++)
        {
            Audio * track = mixedSounds[s];
            ASSERT(track->commitedBytes%4 == 0);
            ASSERT(track->byteSize%4 == 0);
            if (track->commitedBytes + 2*i + 4 + size1 < track->byteSize){
                sampleValueL += *CAST(i16*,&track->data[track->commitedBytes + size1 + i]);
                sampleValueR += *CAST(i16*,&track->data[track->commitedBytes + size1 + i + 2]);
            }
        }
        ((i16*)data2)[i/2] = CAST(i16, sampleValueL/mixedSoundsCount);
        ((i16*)data2)[i/2 + 1] = CAST(i16, sampleValueR/mixedSoundsCount);
    }
    for(i32 s = 0; s < mixedSoundsCount; s++)
    {
        Audio * track = mixedSounds[s];
        track->startedPlaying = true;
    }

    r = primarybuffer->Unlock(data1, size1, data2, size2);
    ASSERT(r == DS_OK);
}

#endif
