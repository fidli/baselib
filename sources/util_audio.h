#ifndef UTIL_AUDIO_H
#define UTIL_AUDIO_H

struct AudioTrack{
    char * data;
    i32 byteSize;
};

AudioTrack loadAudio();
void playAudio(AudioTrack * track);

#endif
