
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
    u32 head;
    
    //end of internal
    u32 size;
    char * contents;
};

struct DirectoryContents
{
    u32 count;
    char ** files;
};

bool getFileChangeTime(const char * path, LocalTime * result);

bool getFileSize(const char * path, u32 * result);

bool skipCurrentLine(FileContents * contents){
	i32 remains = contents->size-contents->head;
	if(remains <= 0) return false;
    char trail = contents->contents[contents->head];
    while(trail != '\r' && trail != '\n' && trail != '\0')
    {
        contents->head++;
        trail = contents->contents[contents->head];
    }
    while((trail == '\r' || trail == '\n') && trail != '\0')
    {
        contents->head++;
        trail = contents->contents[contents->head];
    }
    return true;
}

bool getNextLine(FileContents * contents, char * line, u32 linelen)
{
	i32 remains = contents->size-contents->head;
	if(remains <= 0) return false;
    char format[30];
	u32 res = snprintf(format, 30, "%%%u[^\r\n]", MIN(linelen-1, CAST(u32, remains)));
    ASSERT(res > 0);
    memset(line, 0, linelen);
    if(sscanf(contents->contents + contents->head, format, line) == 1)
    {
        nint len = strlen(line);
        contents->head += CAST(u32, len);
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

bool readBytes(FileContents * contents, char * target, i32 bytesize){
    
	i32 remains = contents->size-contents->head;
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
    i32 newHead = contents->head;
    while(newHead > 0 && contents->contents[newHead] != '\n'){
        newHead--;
    }
    if(newHead != 0){
       newHead++;
    }
    contents->head = newHead;
    return true;
}

bool appendBytes(FileContents * contents, const char * bytes, i32 bytesize)
{
	i32 remains = contents->size-contents->head;
	if(remains < bytesize) return false;
    memcpy(contents->contents + contents->head, bytes, bytesize);
	contents->head += bytesize;
    return true;
}

bool appendLine(FileContents * contents, const char * line)
{
	i32 remains = contents->size-contents->head;
	i32 linelen = CAST(i32, strlen(line));
	if(remains < linelen) return false;
	u32 res = sprintf(contents->contents + contents->head, "%s\r\n", line);
	contents->head += linelen + 2;
    return res > 0;
}

bool appendLinef(FileContents * contents, const char * format, ...)
    {
	bool result = false;
	char * formate = &PUSHA(char, strlen(format) + 2);
	nint resp = sprintf(formate, "%s\r\n", format);
	if(resp > 0)
    {
		i32 remains = contents->size-contents->head;
		if(remains > 0)
        {
			va_list ap;    
			va_start(ap, format);
			u32 res = vsnprintf(contents->contents + contents->head, CAST(nint, remains), formate, ap);
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

void skipBytes(FileContents * contents, i32 amount)
{
    contents->head += amount;
}

u8 readUint8(FileContents * contents)
{
    u8 result = CAST(u8, *(contents->contents + contents->head));
    contents->head += 1;
    return result;
}

u16 readUint16(FileContents * contents)
{
    u16 result = ((CAST(u16, *(contents->contents + contents->head)) << 8) & 0xFF00) | ((CAST(u16, *(contents->contents + 1 + contents->head)) & 0x00FF));
    contents->head += 2;
    return result;
}

u32 readUint32(FileContents * contents)
{
    u32 result = ((CAST(u32, *(contents->contents + contents->head)) << 24) & 0xFF000000) | ((CAST(u32, *(contents->contents + 1 + contents->head)) << 16) & 0x00FF0000)
                    | ((CAST(u32, *(contents->contents + 2 + contents->head)) << 8) & 0x0000FF00) | ((CAST(u32, *(contents->contents + 3 + contents->head) & 0x000000FF)));
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

bool appendFile(const char * path, char * data, nint length){
    FileContents c = {};
    c.size = CAST(u32, length);
    c.contents = data;
    return appendFile(path, &c); 
}

bool createEmptyFile(const char * path){
    FileContents c = {};
    return saveFile(path, &c);
}

// NOTE(fidli): this can be platform-split so you can check just one
const char * filename(const char *path){
    i32 len = CAST(i32, strlen(path));
    for(i32 i = len-1; i >= 0; i--){
        if(path[i] == '\\' || path[i] == '/'){
            return &path[i+1];
        }
    }
    return path;
}

const char * extension(const char *path){
    i32 len = CAST(i32, strlen(path));
    for(i32 i = len-1; i >= 0; i--){
        if(path[i] == '.'){
            return &path[i+1];
        }
    }
    return &path[len];
}

#endif
