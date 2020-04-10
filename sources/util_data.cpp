#ifndef UTIL_DATA
#define UTIL_DATA

#include "util_string.cpp"
#include "util_filesystem.cpp"

struct XMLNode{
    char name[64];
    char * value;
    uint32 valueLength;
    char attributeNames[64][50];
    char attributeValues[64][50];
    uint32 attributesCount;
    
    XMLNode * children[256];
    uint32 childrenCount;
};

struct XMLHeader{
    char version[20];
    char encoding[20];
};

static inline bool isWhitespace(const char input){
	return input == ' ' || input == '\t' || input == '\r' || input == '\n';
}

static void trimWhitespace(char * buffer, uint32 * readIndex, uint32 max){
	uint32 skip = 0;
	while(isWhitespace(*(buffer + *readIndex)) && skip < max){
		(*readIndex)++;
		skip++;
	}
}


static XMLNode * parseXMLRec(char * buffer, const FileContents * xml, uint32 * readIndex, XMLNode * target){
    
    //STEP1: read <node> with parameters and such 
    //TODO(AK): arbitrary length node name and attributes
    XMLNode * newNode = &PUSH(XMLNode);
    newNode->childrenCount = 0;
    newNode->attributesCount = 0;
    buffer[0] = '\0';
    int32 r = sscanf(xml->contents + *readIndex, "<%64[^/ >]", newNode->name);
    ASSERT(r >= 1);
    *readIndex += strlen(newNode->name) + 1;
    
    {//parsing attributes
        char name[50];
        char value[500];
        name[0] = value[0] = 0;
        buffer[0] = 0;
        
        //xml head is at position right behind node name
        trimWhitespace(xml->contents, readIndex, xml->size - *readIndex);
        //PARSE ATTRIBUTES
        //TODO(AK): arbitrary length attributes
        while(sscanf(xml->contents + *readIndex, "%49[^=>/]=\"%499[^\"]\"", name, value) >= 1){
            if(value[0] == 0){
                ASSERT(!strncmp(xml->contents + *readIndex + strlen(name), "=\"\"", 3));
            }
            *readIndex += 3 + strlen(name) + strlen(value);
            //whitespaces between
            trimWhitespace(xml->contents, readIndex, xml->size - *readIndex);
            
            strncpy(newNode->attributeNames[newNode->attributesCount], name, 50);
            strncpy(newNode->attributeValues[newNode->attributesCount], value, 50);
            newNode->attributesCount++;
            ASSERT(newNode->attributesCount <= 64);
            name[0] = value[0] = 0;
        }
    }
    
    //The node is empty, e.g <node />
    if(xml->contents[*readIndex] == '/'){
        ASSERT(target != NULL);
        target->children[target->childrenCount] = newNode;
        target->childrenCount++;
        *readIndex += 2;
        return NULL;
    }
    //closing >
    *readIndex += 1;
    
    trimWhitespace(xml->contents, readIndex, xml->size - *readIndex);
    //STEP2: Insides
    //TODO(AK): CDATA, ARBITRARY LENGTHS
    //zis is parent to many children to come
    if(xml->contents[*readIndex] == '<'){
        
        while(xml->contents[*readIndex] == '<' && xml->contents[*readIndex+1] != '/'){
            parseXMLRec(buffer, xml, readIndex, newNode);
            trimWhitespace(xml->contents, readIndex, xml->size - *readIndex);
        }
        if(target == NULL){
            return newNode;
        }
        ASSERT(target->childrenCount <= ARRAYSIZE(target->children));
        buffer[0] = '\0';
        r = sscanf(xml->contents + *readIndex, "</%1024[^>]>", buffer);
        ASSERT(r >= 1);
        *readIndex += strlen(buffer) + 3;
        ASSERT(strncmp(buffer, newNode->name, 1024) == 0);
        trimWhitespace(xml->contents, readIndex, xml->size - *readIndex);
        target->children[target->childrenCount] = newNode;
        target->childrenCount++;
        
        return NULL;
    }else{
        buffer[0] = '\0';
        r = sscanf(xml->contents + *readIndex, "%1023[^<]", buffer);
        ASSERT(r == 1);
        
        newNode->valueLength = strlen(buffer);
        newNode->value = &PUSHA(char, newNode->valueLength+1);
        strcpy(newNode->value, buffer);
        newNode->value[newNode->valueLength] = '\0';
        *readIndex += newNode->valueLength;
        buffer[0] = '\0';
        r = sscanf(xml->contents + *readIndex, "</%1024[^>]>", buffer);
        ASSERT(r >= 1)
        *readIndex += strlen(buffer) + 3;
        ASSERT(strncmp(buffer, newNode->name, 1024) == 0);
        trimWhitespace(xml->contents, readIndex, xml->size - *readIndex);
        ASSERT(target->childrenCount <= ARRAYSIZE(target->children));
        target->children[target->childrenCount] = newNode;
        target->childrenCount++;
        return NULL;
    }
}

XMLNode * parseXML(const FileContents * xml){
    uint32 readIndex = 0;
    XMLHeader header;
    
    char * buffer = &PUSHA(char, 1024);
    if(sscanf(xml->contents + readIndex, "<?xml%1023[^?]?>", buffer) >= 1){
        //forget header for now
        readIndex += strlen(buffer) + 7;
        trimWhitespace(xml->contents, &readIndex, xml->size - readIndex);
        XMLNode * result = parseXMLRec(buffer, xml, &readIndex, NULL);
        ASSERT(readIndex <= xml->size);
        return result;
    }
    return NULL;
}

#endif
