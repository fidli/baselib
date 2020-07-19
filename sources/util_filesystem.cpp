
#ifndef UTIL_FILESYSTEM
#define UTIL_FILESYSTEM

#include "common.h"
#include "util_time.h"
#include "util_string.cpp"

/*
NOTE(AK):
use watchFile() and hasFileChanged()
*/

struct FileHandle;


struct FileWatchHandle
{
    char path[256];
    LocalTime lastChangeTime;
};

struct FileContents
{
    //internal, do not use
    uint32 head;
    
    //end of internal
    uint32 size;
    char * contents;
};

struct DirectoryContents
{
    uint32 count;
    char ** files;
};

bool getFileChangeTime(const char * path, LocalTime * result);

bool getFileSize(const char * path, uint32 * result);

bool getNextLine(FileContents * contents, char * line, uint32 linelen)
{
	int32 remains = contents->size-contents->head;
	if(remains <= 0) return false;
    char format[30];
	uint32 res = snprintf(format, 30, "%%%u[^\r\n]", MIN(linelen-1, remains+1));
    if(sscanf(contents->contents + contents->head, format, line) == 1)
    {
        contents->head += strlen(line);
        char trail = contents->contents[contents->head];
        while((trail == '\r' || trail == '\n') && trail != '\0')
        {
            contents->head++;
            trail = contents->contents[contents->head];
        }
        
        return true;
    }
    return false;
}

bool readBytes(FileContents * contents, char * target, int32 bytesize){
    
	int32 remains = contents->size-contents->head;
	if(remains < bytesize) return false;
    memcpy(target, contents->contents + contents->head, bytesize);
    contents->head += bytesize;
    return true;
}

bool ungetLine(FileContents * contents){
    if(contents->head == 0){
        return false;
    }
    if(contents->contents[contents->head-1] == '\n'){
        contents->head--;
    }
    if(contents->head == 0){
        return true;
    }
    if(contents->contents[contents->head-1] == '\r'){
        contents->head--;
    }
    if(contents->head == 0){
        return true;
    }
    int64 newHead = contents->head;
    while(newHead > 0 && contents->contents[newHead] != '\n'){
        newHead--;
    }
    if(newHead != 0){
       newHead++;
    }
    contents->head = newHead;
    return true;
}

bool appendBytes(FileContents * contents, char * bytes, int32 bytesize)
{
	int32 remains = contents->size-contents->head;
	if(remains < bytesize) return false;
    memcpy(contents->contents + contents->head, bytes, bytesize);
	contents->head += bytesize;
    return true;
}

bool appendLine(FileContents * contents, char * line)
{
	int32 remains = contents->size-contents->head;
	int32 linelen = strlen(line);
	if(remains < linelen) return false;
	uint32 res = sprintf(contents->contents + contents->head, "%s\r\n", line);
	contents->head += linelen + 2;
    return res > 0;
}

bool appendLinef(FileContents * contents, char * format, ...)
    {
	bool result = false;
	char * formate = &PUSHA(char, strlen(format) + 2);
	nint res = sprintf(formate, "%s\r\n", format);
	if(res > 0)
    {
		int32 remains = contents->size-contents->head;
		if(remains > 0)
        {
			va_list ap;    
			va_start(ap, format);
			uint32 res = vsnprintf(contents->contents + contents->head, remains, formate, ap);
			va_end(ap);
			if(res > 0)
            {
				contents->head += res;
				result = true;
			}
			
		}
	}
	POP;
	return result;	
}

bool watchFile(const char * path, FileWatchHandle * result)
    {
    if(getFileChangeTime(path, &result->lastChangeTime))
    {
        strncpy(result->path, path, ARRAYSIZE(FileWatchHandle::path));
        result->lastChangeTime = {};
        return true;
    }
    return false;
}

bool acknowledgeWatch(FileWatchHandle * target)
{
	LocalTime newTime;
	if(getFileChangeTime(target->path, &newTime))
    {
		target->lastChangeTime = newTime;
		return true;
	}
	return false;
}

bool hasFileChanged(const FileWatchHandle * target)
{
    LocalTime newTime;
    if(getFileChangeTime(target->path, &newTime)){
        if(newTime != target->lastChangeTime){
            return true;
        }
    }
    return false;
}

void updateFileWatch(FileWatchHandle * target){
    LocalTime newTime;
    if(getFileChangeTime(target->path, &newTime)){
        target->lastChangeTime = newTime;
    }
}

void skipBytes(FileContents * contents, int32 amount)
{
    contents->head += amount;
}

uint8 readUint8(FileContents * contents)
{
    uint16 result = CAST(uint8, *(contents->contents + contents->head));
    contents->head += 1;
    return result;
}

uint16 readUint16(FileContents * contents)
{
    uint16 result = ((CAST(uint16, *(contents->contents + contents->head)) << 8) & 0xFF00) | ((CAST(uint16, *(contents->contents + 1 + contents->head)) & 0x00FF));
    contents->head += 2;
    return result;
}

uint32 readUint32(FileContents * contents)
{
    uint32 result = ((CAST(uint32, *(contents->contents + contents->head)) << 24) & 0xFF000000) | ((CAST(uint32, *(contents->contents + 1 + contents->head)) << 16) & 0x00FF0000)
                    | ((CAST(uint32, *(contents->contents + 2 + contents->head)) << 8) & 0x0000FF00) | ((CAST(uint32, *(contents->contents + 3 + contents->head) & 0x000000FF)));
    contents->head += 4;
    return result;
}

bool readFile(const char * path, FileContents * target);
bool saveFile(const char * path, const FileContents * source);
bool appendFile(const char * path, const FileContents * source);
bool readDirectory(const char * path, DirectoryContents * target);
bool fileExists(const char * path);
bool pathExists(const char * path);
bool dirExists(const char * path);
bool deleteFile(const char * path);
bool moveFile(const char * oldPath, const char * newPath);
bool createDirectory(const char *path);

bool appendFile(const char * path, char * data, uint32 length){
    FileContents c = {};
    c.size = length;
    c.contents = data;
    return appendFile(path, &c); 
}

bool createEmptyFile(const char * path){
    FileContents c = {};
    return saveFile(path, &c);
}

#endif
