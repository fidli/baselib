#pragma once
struct AudioTrack{
    char * data;
    i32 byteSize;
};

AudioTrack loadAudio();
void playAudio(AudioTrack * track);

bool decodeWAV(const FileContents * source, AudioTrack * target){
    if (source->size > 44){
        ReadHead head;
        head.offset = CAST(u8*, source->contents) + source->head;
        ASSERT(strncmp(CAST(char*, head.offset), "RIFF", 4) == 0);
        head.offset += 4;
        u32 fileSize = scanDword(&head, ByteOrder_LittleEndian);
        ASSERT(fileSize <= source->size);
        ASSERT(strncmp(CAST(char*, head.offset), "WAVE", 4) == 0);
        head.offset += 4;
        ASSERT(strncmp(CAST(char*, head.offset), "fmt ", 4) == 0);
        head.offset += 4;
        u32 lengthOfFormatData = scanDword(&head, ByteOrder_LittleEndian);
        ASSERT(lengthOfFormatData == 16);
        u16 format = scanWord(&head, ByteOrder_LittleEndian);
        ASSERT(format == 1); // PCM
        u16 bytesPerSample = scanWord(&head, ByteOrder_LittleEndian);
        ASSERT(bytesPerSample == 2);
        u32 samplesPerSecond = scanDword(&head, ByteOrder_LittleEndian);
        ASSERT(samplesPerSecond == 44100);
        u32 bytesPerSecond = scanDword(&head, ByteOrder_LittleEndian);
        ASSERT(bytesPerSecond == 176400);
        scanWord(&head, ByteOrder_LittleEndian); // no idea
        u16 bitsPerSample = scanWord(&head, ByteOrder_LittleEndian);
        ASSERT(bitsPerSample == 16);
        ASSERT(strncmp(CAST(char*, head.offset), "data", 4) == 0);
        head.offset += 4;
        u32 dataSize = scanDword(&head, ByteOrder_LittleEndian);
        ASSERT(44 + dataSize <= source->size);
        ASSERT(CAST(char*, head.offset) - source->contents - source->head == 44);
        target->byteSize = dataSize;
        target->data = CAST(char*, head.offset);
        return true;
    }
    return false;
}
