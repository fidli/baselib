#ifndef COMMON_H
#define COMMON_H

  #ifdef RELEASE
      #define ASSERT
  #else
      #define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
  #endif


  #define ARRAYSIZE(array) sizeof(array) /sizeof(*(array))

  #define KILOBYTE(n) 1024*n
  #define MEGABYTE(n) 1024*KILOBYTE(n)
  #define GIGABYTE(n) 1024*MEGABYTE(n)

  struct memory{
      void * persistent;
      void * stack;
      void * temp;
      uint64 * tempOffsets;
      uint32 tempStackIndex;
      uint64 stackOffset;
  };
  
  memory mem;

  void initMemory(void * memoryStart){
      mem.persistent = memoryStart;
      mem.stack = (void *) ((byte *) mem.persistent + PERSISTENT_MEM);
      
      #define TEMP_MEM_STACK_SIZE 1024
      mem.tempOffsets = (uint64 *) ((byte *) mem.persistent + PERSISTENT_MEM + STACK_MEM);
      mem.temp = (void *) (mem.tempOffsets + TEMP_MEM_STACK_SIZE);
      ASSERT(TEMP_MEM_STACK_SIZE * sizeof(mem.tempOffsets) < TEMP_MEM);
      mem.tempOffsets[0] = 0;
 }
  
#define PUSH(strct)  \
     *((strct *)(((byte *) mem.temp) + mem.tempOffsets[mem.tempStackIndex])); \
     mem.tempOffsets[mem.tempStackIndex+1] = sizeof(strct) + mem.tempOffsets[mem.tempStackIndex]; \
     mem.tempStackIndex++; \
 ASSERT(mem.tempOffsets[mem.tempStackIndex] < TEMP_MEM - (TEMP_MEM_STACK_SIZE * sizeof(*mem.tempOffsets))); ASSERT(mem.tempStackIndex < TEMP_MEM_STACK_SIZE); \


 
#define PUSHS(strct, size) \
     *((strct *)(((byte *) mem.temp) + mem.tempOffsets[mem.tempStackIndex])); \
 mem.tempOffsets[mem.tempStackIndex+1] = (size) + mem.tempOffsets[mem.tempStackIndex]; \
     mem.tempStackIndex++;  \
 ASSERT(mem.tempOffsets[mem.tempStackIndex] < TEMP_MEM - (TEMP_MEM_STACK_SIZE * sizeof(*mem.tempOffsets))); ASSERT(mem.tempStackIndex < TEMP_MEM_STACK_SIZE); \

 
#define PUSHA(strct, size)  \
     *((strct *)(((byte *) mem.temp) + mem.tempOffsets[mem.tempStackIndex])); \
 mem.tempOffsets[mem.tempStackIndex+1] = (sizeof(strct) * (size)) + mem.tempOffsets[mem.tempStackIndex]; \
     mem.tempStackIndex++; \
 ASSERT(mem.tempOffsets[mem.tempStackIndex] < TEMP_MEM - (TEMP_MEM_STACK_SIZE * sizeof(*mem.tempOffsets))); ASSERT(mem.tempStackIndex < TEMP_MEM_STACK_SIZE); \


#define FLUSH \
 mem.tempOffsets[0] = 0; \
 mem.tempStackIndex = 0; \
 
#define POP \
 mem.tempStackIndex--; \
 if(mem.tempStackIndex < 0) { \
     FLUSH; \
 } \
 
#define PPUSH(strct)  \
     *((strct *)(((byte *) mem.stack) + mem.stackOffset)); \
     mem.stackOffset += sizeof(strct); \
     ASSERT(mem.stackOffset < STACK_MEM); \
 
 
#define PPUSHS(strct, size)  \
     *((strct *)(((byte *) mem.stack) + mem.stackOffset)); \
     mem.stackOffset += size; \
     ASSERT(mem.stackOffset < STACK_MEM); \
 

 #define PPUSHA(strct, size)  \
     *((strct *)(((byte *) mem.stack) + mem.stackOffset)); \
 mem.stackOffset += (size) * sizeof(strct); \
     ASSERT(mem.stackOffset < STACK_MEM); \

 
#endif
