#ifndef UTIL_ARCHIVE
#define UTIL_ARCHIVE

#include "util_filesystem.cpp"
#include "util_compress.cpp"

//zip
//https://www.loc.gov/preservation/digital/formats/digformatspecs/APPNOTE%2820120901%29_Version_6.3.3.txt

#pragma pack(push)
#pragma pack(1)
struct ZipCentralDirectoryHeader{
    u32 signature;
    u16 compressVersion;
    u16 minExtractVersion;
    u16 gpFlags;
    u16 compressionMethod;
    u16 mtime;
    u16 mdate;
    u32 crc32;
    u32 compressedSize;
    u32 uncompressedSize;
    u16 fileNameLength;
    u16 extraFieldLength;
    u16 fileCommentLength;
    u16 diskNumberOfFileStart;
    u16 internalFileAttributes;
    u32 externalFileAttributes;
    u32 localFileHeaderOffset;
    char fileName[1];
    
};


struct ZipEndOfCentralDirectory{
    u32 signature;
    u16 diskIndex;
    u16 centralDirectoryStartDiskIndex;
    u16 centralDirectoryRecordOnThisDiskCount;
    u16 centralDirectoryRecordCount;
    u32 centralDirectoryByteSize;
    u32 centralDirectoryOffset;
    u16 commentLength;
    char comment[1];
    
    
};

struct ZipLocalFileHeader{
    u32 signature;
    u16 minExtractVersion;
    u16 gpFlags;
    u16 compressionMethod;
    u16 mtime;
    u16 mdate;
    u32 crc32;
    u32 compressedSize;
    u32 uncompressedSize;
    u16 fileNameLength;
    u16 extraFieldLength;
    char fileName[1];
};

#pragma pack(pop)

struct ZipDirectory{
    char ** fileNames;
    FileContents * files;
    u16 count;
};

ZipDirectory * parseZipArchive(const FileContents * zipArchive){
    ZipEndOfCentralDirectory * endStruct = (ZipEndOfCentralDirectory *)(zipArchive->contents + zipArchive->size - 22);
    
    while(endStruct->signature != 0x06054b50){
        if((char *) endStruct == zipArchive->contents && endStruct->signature != 0x06054b50){
            return NULL;
        }
        endStruct = (ZipEndOfCentralDirectory *)(((char *) endStruct) - 1);
    }
    
    //support split zip archives later
    ASSERT(endStruct->centralDirectoryRecordOnThisDiskCount == endStruct->centralDirectoryRecordCount);
    
    ZipDirectory * result = &PPUSH(ZipDirectory);
    result->count = endStruct->centralDirectoryRecordCount;
    result->fileNames = &PPUSHA(char *, result->count);
    result->files = &PPUSHA(FileContents, result->count);
    u32 customOffset = 0;
    char * firstDirectory = zipArchive->contents + endStruct->centralDirectoryOffset;
    for(u16 ei = 0; ei < endStruct->centralDirectoryRecordCount; ei++){
        ZipCentralDirectoryHeader * centralHeader = (ZipCentralDirectoryHeader *) (firstDirectory + customOffset + ei*(sizeof(ZipCentralDirectoryHeader)-1));
        ASSERT(centralHeader->signature == 0x02014b50);
        
        result->fileNames[ei] = &PPUSHA(char, centralHeader->fileNameLength+1);
        for(u16 si = 0; si < centralHeader->fileNameLength; si++){
            result->fileNames[ei][si] = centralHeader->fileName[si];
        }
        result->fileNames[ei][centralHeader->fileNameLength] = '\0';
        
        result->files[ei].size = centralHeader->uncompressedSize;
        result->files[ei].contents = &PPUSHA(char, centralHeader->uncompressedSize);
        
        //deflate
        ASSERT(centralHeader->compressionMethod == 8);
        ZipLocalFileHeader * localHeader = (ZipLocalFileHeader *) (zipArchive->contents + centralHeader->localFileHeaderOffset);
        ASSERT(localHeader->compressedSize == 0 || localHeader->compressedSize == centralHeader->compressedSize);
        ASSERT(localHeader->uncompressedSize == 0 || localHeader->uncompressedSize == centralHeader->uncompressedSize);
        ASSERT(localHeader->compressionMethod == centralHeader->compressionMethod);
        
        
        decompressDeflate(zipArchive->contents + centralHeader->localFileHeaderOffset + sizeof(ZipLocalFileHeader) - 1 + localHeader->fileNameLength + localHeader->extraFieldLength, centralHeader->compressedSize, result->files[ei].contents);
        
        
        customOffset += centralHeader->fileNameLength + centralHeader->fileCommentLength + centralHeader->extraFieldLength;
    }
    
    return result;
    
}

#endif
