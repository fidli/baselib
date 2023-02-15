#pragma once 

struct PersistentStackAllocator{
	void * mem_start;
	u64 offset;
	u64 effectiveSize;
};

struct StackAllocator{
    void * mem_start;
    u64 * offsets;
    u16 stackIndex;
	u32 stackSize;
	
    u16 * bulkStackOffsets;
    u16 bulkStackIndex;
	u16 bulkStackSize;
	
	u64 effectiveSize;
};

// NOTE(fidli): more like list of allocators, but this is in every app
struct Memory{
    PersistentStackAllocator persistent;
    StackAllocator temp;   
};
Memory mem;

