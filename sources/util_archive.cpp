#ifndef UTIL_ARCHIVE
#define UTIL_ARCHIVE

#include "util_filesystem.cpp"
#include "util_compress.cpp"

//zip
//https://www.loc.gov/preservation/digital/formats/digformatspecs/APPNOTE%2820120901%29_Version_6.3.3.txt

#pragma pack(push)
#pragma pack(1)
struct ZipCentralDirectoryHeader{
    uint32 signature;
    uint16 compressVersion;
    uint16 minExtractVersion;
    uint16 gpFlags;
    uint16 compressionMethod;
    uint16 mtime;
    uint16 mdate;
    uint32 crc32;
    uint32 compressedSize;
    uint32 uncompressedSize;
    uint16 fileNameLength;
    uint16 extraFieldLength;
    uint16 fileCommentLength;
    uint16 diskNumberOfFileStart;
    uint16 internalFileAttributes;
    uint32 externalFileAttributes;
    uint32 localFileHeaderOffset;
    char fileName[1];
    
};


struct ZipEndOfCentralDirectory{
    uint32 signature;
    uint16 diskIndex;
    uint16 centralDirectoryStartDiskIndex;
    uint16 centralDirectoryRecordOnThisDiskCount;
    uint16 centralDirectoryRecordCount;
    uint32 centralDirectoryByteSize;
    uint32 centralDirectoryOffset;
    uint16 commentLength;
    char comment[1];
    
    
};

struct ZipLocalFileHeader{
    uint32 signature;
    uint16 minExtractVersion;
    uint16 gpFlags;
    uint16 compressionMethod;
    uint16 mtime;
    uint16 mdate;
    uint32 crc32;
    uint32 compressedSize;
    uint32 uncompressedSize;
    uint16 fileNameLength;
    uint16 extraFieldLength;
    char fileName[1];
};

#pragma pack(pop)

struct ZipDirectory{
    char ** fileNames;
    FileContents * files;
    uint16 count;
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
    uint32 customOffset = 0;
    char * firstDirectory = zipArchive->contents + endStruct->centralDirectoryOffset;
    for(uint16 ei = 0; ei < endStruct->centralDirectoryRecordCount; ei++){
        ZipCentralDirectoryHeader * centralHeader = (ZipCentralDirectoryHeader *) (firstDirectory + customOffset + ei*(sizeof(ZipCentralDirectoryHeader)-1));
        ASSERT(centralHeader->signature == 0x02014b50);
        
        result->fileNames[ei] = &PPUSHA(char, centralHeader->fileNameLength+1);
        for(uint16 si = 0; si < centralHeader->fileNameLength; si++){
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
