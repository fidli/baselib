#ifndef WINDOWS_AUDIO_CPP
#define WINDOWS_AUDIO_CPP

#include <DSound.h>
#include "util_audio.h"

struct AudioReplay{
    char * data;
    DWORD byteSize;
    DWORD commitedBytes;
    bool startedPlaying;
};

static struct {
    LPDIRECTSOUNDBUFFER buffer;
    DWORD bufferSize;
    DWORD samplesPerSecond;

    DWORD lastWriteCursor;
    AudioReplay mixedSounds[256];
    i32 mixedSoundsCount;
} mixer;

bool initAudio(HWND target){
    mixer.samplesPerSecond = 44100;
    LPDIRECTSOUND8 d8;
    if(SUCCEEDED(DirectSoundCreate8(0, &d8, 0))){
        if(SUCCEEDED(d8->SetCooperativeLevel(target, DSSCL_WRITEPRIMARY))){
            
            WAVEFORMATEX wf = {};
            wf.wFormatTag = WAVE_FORMAT_PCM;
            wf.nChannels = 2;
            wf.nSamplesPerSec = mixer.samplesPerSecond;
            wf.wBitsPerSample = 16;
            wf.nBlockAlign = (wf.nChannels*wf.wBitsPerSample) / 8;
            wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
            wf.cbSize = 0;
            
            DSBUFFERDESC pbuffdesc = {};
            pbuffdesc.dwSize = sizeof(DSBUFFERDESC);
            pbuffdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
            
            if(SUCCEEDED(d8->CreateSoundBuffer(&pbuffdesc, &mixer.buffer, 0))){
                if(!SUCCEEDED(mixer.buffer->SetFormat(&wf))){
                    return false;
                }
            }else{
                return false;
            }      
    
            DSBCAPS caps = {};
            caps.dwSize = sizeof(DSBCAPS);
            HRESULT r = mixer.buffer->GetCaps(&caps);
            ASSERT(r == DS_OK);
            mixer.bufferSize = caps.dwBufferBytes;

            void * data1 = NULL;
            void * data2 = NULL;
            DWORD size1 = 0;
            DWORD size2 = 0;

            r = mixer.buffer->Lock(0, mixer.bufferSize, &data1, &size1, &data2, &size2, 0);
            ASSERT(r == DS_OK);
            ASSERT(data1 != NULL);
            ASSERT(size1 == mixer.bufferSize);
            for (DWORD i = 0; i < size1; i++){
                CAST(char*, data1)[i] = 0;
            }
            r = mixer.buffer->Unlock(data1, size1, data2, size2);
            ASSERT(r == DS_OK);

            r = mixer.buffer->Play(0, 0, DSBPLAY_LOOPING);
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

AudioTrack loadAudio(){
    AudioTrack result = {};
    nint size = CAST(nint, 0.5f*mixer.samplesPerSecond * sizeof(i16) * 2);
    char * secondOfC = &PPUSHA(char, size);
    DWORD samplesFade = 1000;
    for (DWORD sample = 0; sample < i32(size/4); sample++){
        f32 fade = 1.0f;
        if (sample < samplesFade){
            fade = CAST(f32, sample) / CAST(f32, samplesFade);
        }
        else if (size/4 - sample < samplesFade){
            fade = CAST(f32, size/4 - sample) / CAST(f32, samplesFade);
        }
        i16 value = CAST(i16, fade * sin(fmod((CAST(f32, sample)/mixer.samplesPerSecond)*256*2*PI, 2*PI)) * 10000);
        CAST(i16*, secondOfC)[sample*2] = value;
        CAST(i16*, secondOfC)[sample*2+1] = value;
    }
    result.data = secondOfC;
    result.byteSize = CAST(DWORD, size);
    return result;
} 

void playAudio(AudioTrack * track){
    ASSERT(mixer.mixedSoundsCount <= ARRAYSIZE(mixer.mixedSounds));
    AudioReplay * replay = &mixer.mixedSounds[mixer.mixedSoundsCount++];
    replay->data = track->data;
    replay->byteSize = track->byteSize;
    replay->commitedBytes = 0;
    replay->startedPlaying = false;
}

void mix(){
    DWORD playCursor;
    DWORD writeCursor;
    HRESULT r = mixer.buffer->GetCurrentPosition(&playCursor, &writeCursor);
    ASSERT(r == DS_OK);
    // TODO some tolerance, dont need to write after 1 byte was played
    if(writeCursor == mixer.lastWriteCursor){
        return;
    }

    void * data1 = NULL;
    void * data2 = NULL;
    DWORD size1 = 0;
    DWORD size2 = 0;
    ASSERT(mixer.bufferSize != 0);
    // TODO dont need to write whole buffer just now, perhaps smaller part is ok
    DWORD toLock = 0;
    if (playCursor <= writeCursor)
    {
        ASSERT(mixer.bufferSize >= writeCursor);
        toLock = playCursor + (mixer.bufferSize - writeCursor);
    }
    else{
        toLock = playCursor - writeCursor;
    }

    DWORD bytesCommited = 0;
    {
        if (writeCursor >= mixer.lastWriteCursor){
            bytesCommited = writeCursor - mixer.lastWriteCursor;
        }
        else{
            ASSERT(mixer.bufferSize >= mixer.lastWriteCursor);
            bytesCommited = writeCursor + (mixer.bufferSize - mixer.lastWriteCursor);
        }
    }
    mixer.lastWriteCursor = writeCursor;

    r = mixer.buffer->Lock(writeCursor, toLock, &data1, &size1, &data2, &size2, 0);
    ASSERT(r == DS_OK);
    ASSERT(data1 != NULL);
    ASSERT(toLock == size1 + size2);

    for(i32 i = 0; i < mixer.mixedSoundsCount; i++)
    {
        AudioReplay * track = &mixer.mixedSounds[i];
        if (track->startedPlaying){
            track->commitedBytes = MIN(track->byteSize, track->commitedBytes + bytesCommited);
        }
        ASSERT(track->commitedBytes <= track->byteSize);
    }
    ASSERT(bytesCommited%4 == 0);

    void * data[2] = {data1, data2};
    DWORD sizes[2] = {size1, size2};
    DWORD offsets[2] = {0, size1};
    i32 parts = data2 != NULL ? 2 : 1;
    for (i32 p = 0; p < parts; p++)
    {
        ASSERT(sizes[p]%4 == 0);
        for (DWORD i = 0; i < sizes[p]; i += 4)
        {
            i32 mixerValueL = 0;
            i32 mixerValueR = 0;
            for(i32 s = 0; s < mixer.mixedSoundsCount; s++)
            {
                AudioReplay * track = &mixer.mixedSounds[s];
                ASSERT(track->commitedBytes%4 == 0);
                ASSERT(track->byteSize%4 == 0);
                if ((track->commitedBytes + i + 4 + offsets[p]) <= track->byteSize){
                    i32 sampleValueL = *CAST(i16*,&track->data[track->commitedBytes + offsets[p] + i]);
                    i32 sampleValueR = *CAST(i16*,&track->data[track->commitedBytes + offsets[p] + i + 2]);
                    ASSERT((track->commitedBytes + offsets[p] + i + 2) <= track->byteSize);
                    mixerValueL += sampleValueL;
                    mixerValueR += sampleValueR;
                }
            }
            mixerValueL = clamp(CAST(i32, mixerValueL), -32768, 32767); 
            mixerValueR = clamp(CAST(i32, mixerValueR), -32768, 32767);
            ((i16*)data[p])[i/2] = CAST(i16, mixerValueL);
            ((i16*)data[p])[i/2 + 1] = CAST(i16, mixerValueR);
        }
    }
    for(i32 s = 0; s < mixer.mixedSoundsCount; s++)
    {
        AudioReplay * track = &mixer.mixedSounds[s];
        track->startedPlaying = true;
        if (track->commitedBytes >= track->byteSize){
            mixer.mixedSounds[s] = mixer.mixedSounds[mixer.mixedSoundsCount-1];
            mixer.mixedSoundsCount--;
            s--;
        }
    }

    r = mixer.buffer->Unlock(data1, size1, data2, size2);
    ASSERT(r == DS_OK);
}

#endif
