#ifndef UTIL_AUDIO_H
#define UTIL_AUDIO_H


struct Audio{
    byte * samplesdata; //16 bit per channel, 2 channels, 44100 per second
    uint32 size;
    bool finished;
};




#endif