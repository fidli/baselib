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
    
    XMLNode * children[64];
    uint32 childrenCount;
};

struct XMLHeader{
    char version[20];
    char encoding[20];
};

static XMLNode * parseXMLRec(char * buffer, const FileContents * xml, uint32 * readIndex, XMLNode * target){
    //STEP1: read <node> with parameters and such 
    //TODO(AK): arbitrary length node name and attributes
    char waste[4];
    XMLNode * newNode = &PUSH(XMLNode);
    newNode->childrenCount = 0;
    newNode->attributesCount = 0;
    waste[0] = '\0';
    buffer[0] = '\0';
    ASSERT(sscanf(xml->contents + *readIndex, "<%64[^/ >]%1024[^>]>%3[\r\n]", newNode->name, buffer, waste) >= 1);
    uint32 bufflen = strlen(buffer);
    
    
    //buffer may contain attributes
    uint32 attrOffset = 0;
    while(buffer[attrOffset] == ' '){
        attrOffset++;
    }
    char name[50];
    char value[500];
    char waste2[4];
    name[0] = value[0] = waste2[0] = 0;
    //PARSE ATTRIBUTES
    //TODO(AK): arbitrary length attributes
    while(attrOffset < bufflen && sscanf(buffer + attrOffset, "%49[^=]=\"%499[^\"]\"%1[ ]", name, value, waste2) >= 2){
        
        attrOffset += strlen(waste2) + 3 + strlen(name) + strlen(value);
        while(attrOffset < 1023 && buffer[attrOffset] == ' ') attrOffset++;
        ASSERT(attrOffset < 1024);
        strncpy(newNode->attributeNames[newNode->attributesCount], name, 50);
        strncpy(newNode->attributeValues[newNode->attributesCount], value, 50);
        newNode->attributesCount++;
        ASSERT(newNode->attributesCount <= 64);
        name[0] = value[0] = waste2[0] = 0;
    }
    
    *readIndex += strlen(newNode->name) + bufflen + 2 + strlen(waste);
    //The node is empty, e.g <node />
    if(bufflen > 0 && buffer[bufflen-1] == '/'){
        ASSERT(target != NULL);
        target->children[target->childrenCount] = newNode;
        target->childrenCount++;
        return NULL;
    }
    
    //STEP2: Insides
    //TODO(AK): CDATA, ARBITRARY LENGTHS
    //zis is parent to many children to come
    if(xml->contents[*readIndex] == '<'){
        
        while(xml->contents[*readIndex] == '<' && xml->contents[*readIndex+1] != '/'){
            parseXMLRec(buffer, xml, readIndex, newNode);
        }
        if(target == NULL){
            return newNode;
        }
        waste[0] = '\0';
        buffer[0] = '\0';
        ASSERT(sscanf(xml->contents + *readIndex, "</%1024[^>]>%3[\r\n]", buffer, waste) >= 1);
        *readIndex += strlen(buffer) + strlen(waste) + 3;
        target->children[target->childrenCount] = newNode;
        target->childrenCount++;
        
        return NULL;
    }else{
        waste[0] = '\0';
        buffer[0] = '\0';
        ASSERT(sscanf(xml->contents + *readIndex, "%1023[^<]", buffer) == 1);
        
        newNode->valueLength = strlen(buffer);
        newNode->value = &PUSHA(char, newNode->valueLength+1);
        strcpy(newNode->value, buffer);
        newNode->value[newNode->valueLength] = '\0';
        *readIndex += newNode->valueLength;
        waste[0] = '\0';
        buffer[0] = '\0';
        ASSERT(sscanf(xml->contents + *readIndex, "</%1024[^>]>%3[\r\n]", buffer, waste) >= 1);
        *readIndex += strlen(buffer) + strlen(waste) + 3;
        target->children[target->childrenCount] = newNode;
        target->childrenCount++;
        
        return NULL;
    }
}

XMLNode * parseXML(const FileContents * xml){
    uint32 readIndex = 0;
    XMLHeader header;
    
    char * buffer = &PUSHA(char, 1024);
    char waste[4];
    if(sscanf(xml->contents + readIndex, "<?xml%1023[^?]?>%3[\r\n]", buffer, waste) >= 1){
        //forget header for now
        readIndex += strlen(buffer) + 7 + strlen(waste);
        XMLNode * result = parseXMLRec(buffer, xml, &readIndex, NULL);
        ASSERT(readIndex <= xml->size);
        return result;
    }
    return NULL;
}

#endif