#pragma once 

struct PersistentStackAllocator{
	void * mem_start;
	uint64 offset;
	uint64 effectiveSize;
};

struct StackAllocator{
    void * mem_start;
    uint64 * offsets;
    uint32 stackIndex;
	uint32 stackSize;
	
    uint16 * bulkStackOffsets;
    uint16 bulkStackIndex;
	uint16 bulkStackSize;
	
	uint64 effectiveSize;
};

// NOTE(fidli): more like list of allocators, but this is in every app
struct Memory{
    PersistentStackAllocator persistent;
    StackAllocator temp;   
};
Memory mem;

