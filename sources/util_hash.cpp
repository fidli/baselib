#ifndef UTIL_HASH
#define UTIL_HASH

struct StringHashTableElement{
	void * data;
	char key[20];
	StringHashTableElement * next;
};

struct StringHashTable{
	uint32 size;
	StringHashTableElement ** elements;
	
	uint32 slotCount;
	StringHashTableElement * slots;
	uint32 slotsUsed;
};

void clearStringHashTable(StringHashTable * target){
	memset(target->slots, 0, sizeof(StringHashTableElement) * target->slotCount);
	memset(target->elements, 0, sizeof(StringHashTableElement *) * target->size);
	target->slotsUsed = 0;
}

bool createStringHashTable(StringHashTable * target, int32 size){
	target->size = size*2;
	target->slotCount = size;
	target->slots = &PPUSHA(StringHashTableElement, target->slotCount);
	target->elements = &PPUSHA(StringHashTableElement *, target->size);
	clearStringHashTable(target);
	return true;
}

 


static uint32 stringHash(const char * input, uint32 tableSize){
	//djb http://www.cse.yorku.ca/~oz/hash.html
	uint32 hash = 5381;
	while(*(input++)){
		hash = hash * 33 + *input;
	}
	return hash % tableSize;
}

bool insertIntoStringHashTable(StringHashTable * target, const char * key, void * data){
	if (target->slotsUsed >= target->slotCount) return false;
	ASSERT(strlen(key) < 20);
	int32 index = stringHash(key, target->size);
	StringHashTableElement ** spot = &target->elements[index];
	StringHashTableElement * previous = NULL;
	while(*spot != NULL){
		if(!strncmp(key, (*spot)->key, 20)){
			break;
		}else{
			previous = *spot;
			spot = &((*spot)->next);
		}
	}
	if(*spot == NULL){
		*spot = &target->slots[target->slotsUsed++];
		strncpy((*spot)->key, key, ARRAYSIZE((*spot)->key));
		(*spot)->next = NULL;
		if(previous != NULL){
			previous->next = *spot;
		}
	}
	(*spot)->data = data;
	return true;
}

bool findInStringHashTable(StringHashTable * target, const char * key, void ** data){
	int32 index = stringHash(key, target->size);
	StringHashTableElement ** spot = &target->elements[index];
	while(*spot != NULL){
		if(!strncmp(key, (*spot)->key, 20)){
			*data = (*spot)->data;
			return true;
		}else{
			spot = &((*spot)->next);
		}
	}
	return false;
}



#endif
