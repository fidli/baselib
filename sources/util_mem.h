#pragma once 

struct PersistentStackAllocator{
	void * mem_start;
	uint64 offset;
	uint64 effectiveSize;
};

inline
void * allocate(PersistentStackAllocator * allocator, uint64 bytes)
{
		void * result = ((byte *)allocator->mem_start) + allocator->offset + bytes;
		allocator->offset += bytes;
		ASSERT(allocator->offset < allocator->effectiveSize);
		return result;
}

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

inline
void * allocate(StackAllocator * allocator, uint64 bytes)
{
	void * result = ((byte *) allocator->mem_start) + allocator->offsets[allocator->stackIndex];
	allocator->offsets[allocator->stackIndex+1] = bytes + allocator->offsets[allocator->stackIndex];
	allocator->stackIndex++;
	ASSERT(allocator->offsets[allocator->stackIndex] < allocator->effectiveSize);
	ASSERT(allocator->stackIndex < allocator->stackSize);
	return result;
}

inline
void deallocate(StackAllocator * allocator)
{
	// NOTE(fidli): do not underflow the stack mark
	ASSERT(allocator->bulkStackIndex == 0 || allocator->stackIndex > allocator->bulkStackOffsets[allocator->bulkStackIndex-1]);
	ASSERT(allocator->stackIndex != 0);
	allocator->stackIndex--;
}

inline
void markAllocation(StackAllocator * allocator)
{
	allocator->bulkStackOffsets[allocator->bulkStackIndex+1] = allocator->stackIndex;
	allocator->bulkStackIndex++;
	ASSERT(allocator->bulkStackIndex < allocator->bulkStackSize); 
}

inline
void deallocateMark(StackAllocator * allocator)
{
	allocator->stackIndex = allocator->bulkStackOffsets[allocator->bulkStackIndex];
	allocator->bulkStackIndex--;
	if(allocator->bulkStackIndex < 0)
	{
		allocator->bulkStackIndex = 0;
		allocator->bulkStackOffsets[0] = 0;
	}
}

inline
void deallocateAll(StackAllocator * allocator)
{
	allocator->offsets[0] = 0;
    allocator->bulkStackOffsets[0] = 0;
    allocator->stackIndex = 0;
    allocator->bulkStackIndex = 0;
}

struct Memory{
    PersistentStackAllocator persistent;
    StackAllocator temp;   
};
Memory mem;

#define TEMP_MEM_STACK_SIZE 4096
#define TEMP_MEM_BULK_STACK_SIZE 32
#define METADATA_SIZE (TEMP_MEM_BULK_STACK_SIZE * sizeof(*mem.temp.bulkStackOffsets) + TEMP_MEM_STACK_SIZE * sizeof(*mem.temp.offsets))


void initMemory(void * memoryStart){
    mem.persistent.mem_start = memoryStart;
    mem.temp.mem_start = (void *) ((byte *) mem.persistent.mem_start + PERSISTENT_MEM + METADATA_SIZE);
    
    mem.temp.offsets = (uint64 *) ((byte *) mem.persistent.mem_start + PERSISTENT_MEM);
    mem.temp.bulkStackOffsets = (uint16 *) ((byte *) mem.temp.offsets + TEMP_MEM_STACK_SIZE*sizeof(*mem.temp.offsets));
    
    ASSERT(METADATA_SIZE < TEMP_MEM);
	mem.persistent.effectiveSize = PERSISTENT_MEM;
	mem.temp.effectiveSize = TEMP_MEM - METADATA_SIZE;
	mem.temp.bulkStackSize = TEMP_MEM_BULK_STACK_SIZE;
	mem.temp.stackSize = TEMP_MEM_STACK_SIZE;
    
	deallocateAll(&mem.temp);
	
}

#define PUSH(strct) *((strct *) allocate(&mem.temp, sizeof(strct)));

#define PUSHA(strct, size) *((strct *) allocate(&mem.temp, (size) * sizeof(strct)));

#define PUSHI markAllocation(&mem.temp);

#define POPI  deallocateMark(&mem.temp);

#define FLUSH deallocateAll(&mem.temp);

#define POP deallocate(&mem.temp);

#define PPUSH(strct) *((strct *) allocate(&mem.persistent, sizeof(strct)));

#define PPUSHA(strct, size) *((strct *) allocate(&mem.persistent, (size) * sizeof(strct)));
