 #ifndef UTIL_ARCHIVE
#define UTIL_ARCHIVE
 
 
 struct ZipEndOfCentralDirectory{
     uint32 signature;
     uint16 diskIndex;
     uint16 centralDirectoryStartDiskIndex;
     uint16 centralDirectoryRecordOnThisDiskCount;
     uint16 centralDirectoryRecordCount;
     uint32 centralDirectoryByteSize;
     uint32 centralDirectoryOffset;
     uint16 commentLength;
     char * comment;0
     
     
 };
 
 bool parseZipArchive(const FileContents * zipArchive){
     
 }
 
#endif
