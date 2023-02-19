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
        head.offset = source->contents + source->head;
        ASSERT(strncmp(head.offset, "RIFF", 4) == 0);
        head.offset += 4;
        u32 fileSize = scanDword(&head);
        ASSERT(fileSize <= source->size);
        ASSERT(strncmp(head.offset, "WAVE", 4) == 0);
        head.offset += 4;
        ASSERT(strncmp(head.offset, "fmt ", 4) == 0);
        head.offset += 4;
        u32 lengthOfFormatData = scanDword(&head);
        ASSERT(lengthOfFormatData == 16);
        u16 format = scanWord(&head);
        ASSERT(format == 1); // PCM
        u16 bytesPerSample = scanWord(&head);
        ASSERT(bytesPerSample == 2);
        u32 samplesPerSecond = scanDword(&head);
        ASSERT(samplesPerSecond == 44100);
        u32 bytesPerSecond = scanDword(&head);
        ASSERT(bytesPerSecond == 176400);
        scanWord(&head); // no idea
        u16 bitsPerSample = scanWord(&head);
        ASSERT(bitsPerSample == 16);
        ASSERT(strncmp(head.offset, "data", 4) == 0);
        head.offset += 4;
        u32 dataSize = scanDword(&head);
        ASSERT(44 + dataSize <= source->size);
        ASSERT(head.offset - source->contents - source->head == 44);
        target->byteSize = dataSize;
        target->data = head.offset;
        return true;
    }
    return false;
}
