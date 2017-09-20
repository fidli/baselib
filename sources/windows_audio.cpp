#ifndef WINDOWS_AUDIO_CPP
#define WINDOWS_AUDIO_CPP


#include "util_audio.h"
#include "windows_audio.h"

//intended only for loading audio by mff bollocks
static struct buffersample{
    buffersample * next;
    byte * data;
    DWORD size;
};

void loadAudio(const WCHAR * path, Audio * target){
    IMFSourceReader * reader;
    HRESULT r =  MFCreateSourceReaderFromURL(path, NULL, &reader);
    ASSERT(r == S_OK);
    
    //deselect all streams
    ASSERT(SUCCEEDED(reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE)));
    //select first audio stream
    ASSERT(SUCCEEDED(reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE)));
    
    //set the target type, #noidea
    IMFMediaType * targetType;
    ASSERT(SUCCEEDED(MFCreateMediaType(&targetType)));
    ASSERT(SUCCEEDED(targetType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)));
    ASSERT(SUCCEEDED(targetType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)));
    
    //convert
    reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, targetType);
    
    //obtain exact variables
    reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &targetType);
    
    WAVEFORMATEX * sourceFormat;
    UINT32 formatSize = 0;
    ASSERT(SUCCEEDED(MFCreateWaveFormatExFromMFMediaType(targetType, &sourceFormat, &formatSize)));
    
    ASSERT(sourceFormat->nChannels == 2);
    ASSERT(sourceFormat->nSamplesPerSec == mixer.samplesPerSecond);
    ASSERT(sourceFormat->wBitsPerSample == 16);
    
    //copy data into own memory
    //(is roughly 10 MB/sec)
    DWORD outflags;
    IMFSample * sample;
    IMFMediaBuffer *buffer;
    buffersample * queue = &PUSH(buffersample);
    buffersample * current = queue;
    uint32 totalSize = 0;
    while(true){
        //read samples till you die,
        ASSERT(SUCCEEDED(reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &outflags, NULL, &sample)));
        
        if((outflags & MF_SOURCE_READERF_ENDOFSTREAM) || sample == NULL){
            current->next = NULL;
            break;
        }
        
        ASSERT(SUCCEEDED(sample->ConvertToContiguousBuffer(&buffer)));
        
        
        byte * data;
        DWORD currentSize = 0;
        ASSERT(SUCCEEDED(buffer->Lock(&data, NULL, &currentSize)));
        
        totalSize += currentSize;
        current->size = currentSize;
        
        current->data = &PUSHS(byte, currentSize);
        
        for(int i = 0; i < currentSize; i++){
            current->data[i] = data[i];
        }
        
        ASSERT(SUCCEEDED(buffer->Unlock()));
        
        current->next = &PUSH(buffersample);
        current = current->next;
        buffer->Release();
        sample->Release();
    }
    
    //copy linked list audio chunks into one piece of memory
    target->size = totalSize;
    target->samplesdata = &PPUSHS(byte, target->size);
    
    byte * samplesbuffer = target->samplesdata;
    
    current = queue;
    
    while(current != NULL){
        for(int i = 0; i < current->size; i++){
            *samplesbuffer = current->data[i];
            samplesbuffer++;
        }
        current = current->next;
    }
    
    
    CoTaskMemFree(sourceFormat);
    targetType->Release();
    reader->Release();
    
    target->finished = false;
    
}

bool initAudio(){
    
    mixer.samplesPerSecond = 44100;
    mixer.bytesPerSample = sizeof(int16)*2;
    mixer.bufferSize = mixer.samplesPerSecond*mixer.bytesPerSample*2;
    
    
    LPDIRECTSOUND8 d8;
    if(SUCCEEDED(DirectSoundCreate8(0, &d8, 0))){
        if(SUCCEEDED(d8->SetCooperativeLevel(context.window, DSSCL_PRIORITY))){
            
            WAVEFORMATEX wf = {};
            wf.wFormatTag = WAVE_FORMAT_PCM;
            wf.nChannels = 2;
            wf.nSamplesPerSec = mixer.samplesPerSecond;
            wf.wBitsPerSample = 16;
            wf.nBlockAlign = (wf.nChannels*wf.wBitsPerSample) / 8;
            wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
            wf.cbSize = 0;
            
            DSBUFFERDESC pbuffdesc= {};
            pbuffdesc.dwSize = sizeof(DSBUFFERDESC);
            pbuffdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
            
            LPDIRECTSOUNDBUFFER primarybuffer;
            
            if(SUCCEEDED(d8->CreateSoundBuffer(&pbuffdesc, &primarybuffer, 0))){
                if(!SUCCEEDED(primarybuffer->SetFormat(&wf))){
                    return false;
                }
                
                
            }                      else{
                return false;
            }      
            
            
            
            
            
            
            DSBUFFERDESC sbuffdesc= {};
            sbuffdesc.dwSize = sizeof(DSBUFFERDESC);
            sbuffdesc.dwBufferBytes = mixer.bufferSize;
            sbuffdesc.lpwfxFormat = &wf;
            
            
            
            if(!SUCCEEDED(d8->CreateSoundBuffer(&sbuffdesc, &mixer.talkbuffer, 0))){
                return false;
            }
            
            
        }
    }
    
    
    //now for reading and loading files
    
    if(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK){
        if(SUCCEEDED(MFStartup(MF_VERSION, MFSTARTUP_LITE))){
            return true;
        }
    } 
    
    
    
    return false;
}

void closeAudio(){
    //io files
    MFShutdown();
    CoUninitialize();
}


static void updatePlayCursorPosition(){
    DWORD writeCursor;
    DWORD playCursor;
    mixer.played = false;
    if(SUCCEEDED(mixer.talkbuffer->GetCurrentPosition(&playCursor, &writeCursor))){
        if(mixer.playCursor != playCursor){
            //something was played
            
            if(mixer.bytesLoaded != 0){
                //talk audio was played
                if(playCursor > mixer.playCursor){    
                    mixer.bytesPlayed += playCursor - mixer.playCursor;
                }else{
                    mixer.bytesPlayed += mixer.bufferSize -  mixer.playCursor + playCursor;
                }
            }
            
            mixer.playCursor = playCursor;
            mixer.writeCursor = writeCursor;
            mixer.played = true;
        }
    }
    
}

static void loadBuffer(){
    if(mixer.bytesLoaded < mixer.target->size){
        DWORD writeCursor = mixer.writeCursor;
        DWORD playCursor = mixer.playCursor;
        if(mixer.played){
            
            DWORD byteToLock = writeCursor;
            DWORD bytesToWrite;
            DWORD bytesGap;
            
            if(byteToLock >= playCursor){
                
                bytesToWrite = mixer.bufferSize - byteToLock; 
                bytesToWrite += playCursor;
                
                bytesGap = byteToLock - playCursor;
                
            } else{
                bytesToWrite = playCursor - byteToLock;
                bytesGap = byteToLock + (mixer.bufferSize - playCursor);
            }
            
            
            
            
            
            //not preloaded enough
            DWORD bytesUnplayed = mixer.bytesLoaded - mixer.bytesPlayed;
            if(mixer.bytesLoaded != 0){
                bytesUnplayed -= bytesGap;
            }
            
            if(bytesToWrite > bytesUnplayed){
                
                byteToLock = (playCursor + bytesUnplayed) % mixer.bufferSize;
                bytesToWrite -= bytesUnplayed + bytesGap;   
                
                
                
                //lock and load
                
                void * region1;
                DWORD region1size;
                void * region2;
                DWORD region2size;
                
                if(SUCCEEDED(mixer.talkbuffer->Lock(byteToLock, bytesToWrite, &region1, &region1size, &region2, &region2size,0))){
                    
                    
                    for(DWORD index = 0; index < region1size && index + mixer.bytesLoaded < mixer.target->size; index++){
                        
                        if(index + mixer.bytesLoaded < mixer.target->size){
                            
                            *((byte *) region1 + index) = mixer.target->samplesdata[index+mixer.bytesLoaded];
                            
                        }
                        else{
                            
                            *((byte *) region1 + index) = 0;
                        }
                        
                        
                    }
                    
                    mixer.bytesLoaded += region1size;
                    
                    for(DWORD index = 0; index < region2size && index + mixer.bytesLoaded < mixer.target->size; index++){
                        
                        if(index + mixer.bytesLoaded < mixer.target->size){
                            
                            *((byte *) region2 + index) = mixer.target->samplesdata[index+mixer.bytesLoaded];
                        }
                        else{
                            
                            *((byte *) region2 + index) = 0;
                        }
                        
                        
                    }
                    
                    
                    
                    
                    
                    mixer.bytesLoaded += region2size;
                    
                    if(!SUCCEEDED(mixer.talkbuffer->Unlock(region1, region1size, region2, region2size))){
                        ASSERT(!"Failed to unlock buffer");
                    }
                    
                }
            }
            
            
        }
        
    }
    
    
    
    
    
    
    
}


void mix(AudioItem * queue){
    AudioItem * current = queue;
    while(current != NULL){
        switch(current->type){
            case AudioType_PlayTalk:
            mixer.talkplaying = true;
            mixer.talkbuffer->Play(0,0,DSBPLAY_LOOPING);
            mixer.bytesLoaded = 0;
            mixer.bytesPlayed = 0;
            mixer.target = current->target;
            mixer.target->finished = false;
            break;
            case AudioType_StopTalk:
            mixer.talkplaying = false;
            mixer.talkbuffer->Stop();
            break;
            default:
            ASSERT(!"FUCK");
            break;
        }
        current = current->next;
    }
    if(mixer.talkplaying){
        updatePlayCursorPosition();
        loadBuffer();
        if(mixer.bytesPlayed >= mixer.target->size){
            mixer.target->finished = true;
        }
    }
    
}




#endif