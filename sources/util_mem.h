#ifndef UTIL_MEM_H
#define UTIL_MEM_H

struct memory{
    void * persistent;
    void * stack;
    void * temp;
    //persistent stack
    uint64 stackOffset;
    //temp stacks
    uint64 * tempOffsets;
    uint32 tempStackIndex;
    //temp indexStacks
    uint16 * temp2Offsets;
    uint16 temp2StackIndex;
    
};

#define TEMP_MEM_STACK_SIZE 4096
#define TEMP2_MEM_STACK_SIZE 32
#define EFFECTIVE_TEMP_MEM_SIZE (TEMP_MEM - TEMP2_MEM_STACK_SIZE * sizeof(mem.temp2Offsets)  - TEMP_MEM_STACK_SIZE * sizeof(mem.tempOffsets))

memory mem;

void initMemory(void * memoryStart){
    mem.persistent = memoryStart;
    mem.stack = (void *) ((byte *) mem.persistent + PERSISTENT_MEM);
    
    
    mem.tempOffsets = (uint64 *) ((byte *) mem.persistent + PERSISTENT_MEM + STACK_MEM);
    mem.temp2Offsets = (uint16 *) (mem.tempOffsets + TEMP_MEM_STACK_SIZE);
    mem.temp = (void *) (mem.temp2Offsets + TEMP2_MEM_STACK_SIZE);
    ASSERT(EFFECTIVE_TEMP_MEM_SIZE < TEMP_MEM);
    mem.tempOffsets[0] = 0;
    mem.temp2Offsets[0] = 0;
    mem.tempStackIndex = 0;
    mem.temp2StackIndex = 0;
}

#define PUSH(strct)  \
*((strct *)(((byte *) mem.temp) + mem.tempOffsets[mem.tempStackIndex])); \
mem.tempOffsets[mem.tempStackIndex+1] = sizeof(strct) + mem.tempOffsets[mem.tempStackIndex]; \
mem.tempStackIndex++; \
ASSERT(mem.tempOffsets[mem.tempStackIndex] < EFFECTIVE_TEMP_MEM_SIZE); ASSERT(mem.tempStackIndex < TEMP_MEM_STACK_SIZE); \



#define PUSHS(strct, size) \
*((strct *)(((byte *) mem.temp) + mem.tempOffsets[mem.tempStackIndex])); \
mem.tempOffsets[mem.tempStackIndex+1] = (size) + mem.tempOffsets[mem.tempStackIndex]; \
mem.tempStackIndex++;  \
ASSERT(mem.tempOffsets[mem.tempStackIndex] < EFFECTIVE_TEMP_MEM_SIZE); ASSERT(mem.tempStackIndex < TEMP_MEM_STACK_SIZE); \


#define PUSHA(strct, size)  \
*((strct *)(((byte *) mem.temp) + mem.tempOffsets[mem.tempStackIndex])); \
mem.tempOffsets[mem.tempStackIndex+1] = (sizeof(strct) * (size)) + mem.tempOffsets[mem.tempStackIndex]; \
mem.tempStackIndex++; \
ASSERT(mem.tempOffsets[mem.tempStackIndex] < EFFECTIVE_TEMP_MEM_SIZE); ASSERT(mem.tempStackIndex < TEMP_MEM_STACK_SIZE); \

#define PUSHI  \
mem.temp2Offsets[mem.temp2StackIndex+1] = mem.tempStackIndex; \
mem.temp2StackIndex++; \
ASSERT(mem.temp2StackIndex < TEMP2_MEM_STACK_SIZE); \

#define POPI  \
mem.tempStackIndex = mem.temp2Offsets[mem.temp2StackIndex]; \
mem.temp2StackIndex--; \
if(mem.temp2StackIndex < 0) { \
    mem.temp2StackIndex = 0; \
    mem.temp2Offsets[0] = 0; \
} \

#define FLUSH \
mem.tempOffsets[0] = 0; \
mem.tempStackIndex = 0; \
mem.temp2Offsets[0] = 0; \
mem.temp2StackIndex = 0; \

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
