 #ifndef UTIL_DATA
#define UTIL_DATA
 
#include "util_filesystem.h"
 
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
     char waste[4];
     
     XMLNode * newNode = &PUSH(XMLNode);
     newNode->childrenCount = 0;
     newNode->attributesCount = 0;
     waste[0] = '\0';
     buffer[0] = '\0';
     ASSERT(sscanf(xml->contents + *readIndex, "<%64[^/ >]%1024[^>]>%3[\r\n]", newNode->name, buffer, waste) >= 1);
     uint32 bufflen = strlen(buffer);
     
     
     //buffer contains attributes
     uint32 attrOffset = 0;
     while(buffer[attrOffset] == ' '){
         attrOffset++;
     }
     char name[50];
     char value[50];
     char waste2[4];
     while(sscanf(buffer + attrOffset, "%104%49[^=]=\"%49[^\"]\"%1[ ]", name, value, waste2) >= 2){
         
         attrOffset += strlen(waste2) + 3 + strlen(name) + strlen(value);
         while(attrOffset < 1023 && buffer[attrOffset] == ' ') attrOffset++;
         ASSERT(attrOffset < 1024);
         strcpy_n(newNode->attributeNames[newNode->attributesCount], name, 50);
         strcpy_n(newNode->attributeValues[newNode->attributesCount], value, 50);
         newNode->attributesCount++;
         ASSERT(newNode->attributesCount <= 64);
     }
     
     *readIndex += strlen(newNode->name) + bufflen + 2 + strlen(waste);
     if(buffer[bufflen-1] == '/'){
         ASSERT(target != NULL);
         target->children[target->childrenCount] = newNode;
         target->childrenCount++;
         return NULL;
     }
     
     //todo cdata & shit
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
         ASSERT(sscanf(xml->contents + *readIndex, "%1024%1023[^<]", buffer) == 1);
         
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
     if(sscanf(xml->contents + readIndex, "%1013<?xml%1023[^?]?>%3[\r\n]", buffer, waste) >= 1){
         //fuck header for now
         readIndex += strlen(buffer) + 7 + strlen(waste);
         XMLNode * result = parseXMLRec(buffer, xml, &readIndex, NULL);
         ASSERT(readIndex <= xml->size);
         return result;
     }
     return NULL;
 }
 
#endif