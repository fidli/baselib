#pragma once 

#include "mem_structs.h"

inline
void * allocate(PersistentStackAllocator * allocator, u64 bytes)
{
		void * result = ((byte *)allocator->mem_start) + allocator->offset;
		allocator->offset += bytes;
		if(allocator->offset < allocator->effectiveSize)
		{
			return result;
		}
#ifndef RELEASE
		INV;
#endif
		return NULL;
}

inline
void * allocate(StackAllocator * allocator, u64 bytes)
{
	void * result = ((byte *) allocator->mem_start) + allocator->offsets[allocator->stackIndex];
	allocator->offsets[allocator->stackIndex+1] = bytes + allocator->offsets[allocator->stackIndex];
	allocator->stackIndex++;
	if((allocator->offsets[allocator->stackIndex] < allocator->effectiveSize) && (allocator->stackIndex < allocator->stackSize));
	{
		return result;
	}
	INV;
	return NULL;
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

class ScopeAllocation
{
	private:
		StackAllocator * allocator_;
	public:
		ScopeAllocation(StackAllocator * allocator)
		{
			allocator_ = allocator;
			
		}
	
		~ScopeAllocation()
		{
			deallocate(allocator_);
		}
};

#define TEMP_MEM_STACK_SIZE 4096
#define TEMP_MEM_BULK_STACK_SIZE 32

void initMemory(void * memoryStart){
    mem.persistent.mem_start = memoryStart;
	mem.persistent.effectiveSize = PERSISTENT_MEM;

	mem.temp.offsets = CAST(u64 *, CAST(byte *, mem.persistent.mem_start) + mem.persistent.effectiveSize);
    mem.temp.bulkStackOffsets = CAST(u16 *, CAST(byte *, mem.temp.offsets) + TEMP_MEM_STACK_SIZE*sizeof(*mem.temp.offsets));
	
	u64 tempMemMetadataSize = TEMP_MEM_BULK_STACK_SIZE * sizeof(*mem.temp.bulkStackOffsets) + TEMP_MEM_STACK_SIZE * sizeof(*mem.temp.offsets);
	mem.temp.mem_start = CAST(void *, CAST(byte*, mem.persistent.mem_start) + tempMemMetadataSize + mem.persistent.effectiveSize);
    ASSERT(tempMemMetadataSize < TEMP_MEM);
	mem.temp.effectiveSize = TEMP_MEM - tempMemMetadataSize;
	mem.temp.bulkStackSize = TEMP_MEM_BULK_STACK_SIZE;
	mem.temp.stackSize = TEMP_MEM_STACK_SIZE;
    
	deallocateAll(&mem.temp);
	
}

#define PUSH(strct) *((strct *) allocate(&mem.temp, sizeof(strct)));

#define PUSHA(strct, size) *((strct *) allocate(&mem.temp, (size) * sizeof(strct)));

#define PUSHA_SCOPE(strct, size) *((strct *) allocate(&mem.temp, (size) * sizeof(strct))); ScopeAllocation CONCAT(AutoScopeAllocationFromLine, __LINE__)(&mem.temp);

#define PUSH_SCOPE(strct) *((strct *) allocate(&mem.temp, sizeof(strct))); ScopeAllocation CONCAT(AutoScopeAllocationFromLine, __LINE__)(&mem.temp);

#define PUSHI markAllocation(&mem.temp);

#define POPI deallocateMark(&mem.temp);

#define FLUSH deallocateAll(&mem.temp);

#define POP deallocate(&mem.temp);

#define PPUSH(strct) *((strct *) allocate(&mem.persistent, sizeof(strct)));

#define PPUSHA(strct, size) *((strct *) allocate(&mem.persistent, (size) * sizeof(strct)));
