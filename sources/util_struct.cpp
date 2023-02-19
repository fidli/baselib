#pragma once

struct StringHashTableElement{
	void * data;
	char key[20];
	StringHashTableElement * next;
};

struct StringHashTable{
	u32 size;
	StringHashTableElement ** elements;
	
	u32 slotCount;
	StringHashTableElement * slots;
	u32 slotsUsed;
};

void clearStringHashTable(StringHashTable * target){
	memset(target->slots, 0, sizeof(StringHashTableElement) * target->slotCount);
	memset(target->elements, 0, sizeof(StringHashTableElement *) * target->size);
	target->slotsUsed = 0;
}

bool createStringHashTable(StringHashTable * target, i32 size){
	target->size = size*2;
	target->slotCount = size;
	target->slots = &PPUSHA(StringHashTableElement, target->slotCount);
	target->elements = &PPUSHA(StringHashTableElement *, target->size);
	clearStringHashTable(target);
	return true;
}

 


static u32 stringHash(const char * input, u32 tableSize){
	//djb http://www.cse.yorku.ca/~oz/hash.html
	u32 hash = 5381;
	while(*(input++)){
		hash = hash * 33 + *input;
	}
	return hash % tableSize;
}

bool insertIntoStringHashTable(StringHashTable * target, const char * key, void * data){
	if (target->slotsUsed >= target->slotCount) return false;
	ASSERT(strlen(key) < 20);
	i32 index = stringHash(key, target->size);
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
	i32 index = stringHash(key, target->size);
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

struct DoubleLinkedListElement{
	void * data;
	char key[20];
	DoubleLinkedListElement * previous;
	DoubleLinkedListElement * next;
};

struct DoubleLinkedList{
	u32 size;
	DoubleLinkedListElement ** elements;
	DoubleLinkedListElement * head;
	DoubleLinkedListElement * first;
	
	
	DoubleLinkedListElement * slots;
	u32 slotsUsed;
};

void nextInDoubleLinkedList(DoubleLinkedList * target){
	if(target->head){
		target->head = target->head->next;
	}
}

void previousInDoubleLinkedList(DoubleLinkedList * target){
	if(target->head){
		target->head = target->head->previous;
	}
}

void clearDoubleLinkedList(DoubleLinkedList * target){
	memset(target->slots, 0, sizeof(DoubleLinkedListElement) * target->size);
	memset(target->elements, 0, sizeof(DoubleLinkedListElement *) * target->size);
	target->head = NULL;
	target->slotsUsed = 0;
}

bool createDoubleLinkedList(DoubleLinkedList * target, i32 size){
	target->size = size;
	target->slots = &PPUSHA(DoubleLinkedListElement, target->size);
	target->elements = &PPUSHA(DoubleLinkedListElement *, target->size);
	clearDoubleLinkedList(target);
	return true;
}

bool findInDoubleLinkedList(DoubleLinkedList * target, const char * key, void ** data){
	if(!target->head) return false;
	DoubleLinkedListElement * current = target->head;
	do{
		if(!strncmp(key, current->key, 20)){
			if(data != NULL){
				*data = current->data;
			}
			target->head = current;
			return true;
		}else{
			current = current->next;
		}
	}while(current != target->head);
	return false;
}

bool renameInDoubleLinkedList(DoubleLinkedList * target, const char * key, const char * newkey){
	if(!target->head) return false;
	DoubleLinkedListElement * current = target->head;
	do{
		if(!strncmp(key, current->key, 20)){
			strncpy(current->key, newkey, 20);
			return true;
		}else{
			current = current->next;
		}
	}while(current != target->head);
	return false;
}

bool insertIntoDoubleLinkedList(DoubleLinkedList * target, const char * key, void * data){
	if(target->slotsUsed >= target->size) return false;
	DoubleLinkedListElement * slot = &target->slots[target->slotsUsed++];
	strncpy(slot->key, key, 20);
	slot->data = data;
	if(target->head == NULL){
		slot->next = slot;
		slot->previous = slot;
		target->head = slot;
		target->first = slot;
	}else{
		slot->next = target->head->next;
		slot->previous = target->head;
		
		target->head->next->previous = slot;
		target->head->next = slot;
		
		target->head = slot;
	}
	return true;
}

bool removeFromDoubleLinkedList(DoubleLinkedList * target, DoubleLinkedListElement * toDelete){
    ASSERT(target->slotsUsed > 0);
    ASSERT(CAST(char *, toDelete) >= CAST(char *, target->slots) && CAST(char *, toDelete) <= CAST(char *, target->slots)+ (target->slotsUsed-1)*sizeof(DoubleLinkedListElement));
    for(i32 i = 0; i < target->slotsUsed; i++){
        DoubleLinkedListElement * e = &target->slots[i];
        if(e->previous == toDelete){
            e->previous = toDelete->previous;
        }
        if(e->next == toDelete){
            e->next = toDelete->next;
        }
    }
    if(target->head == toDelete){
        if(target->first == toDelete){
            target->head = toDelete->next;
        }else{
            target->head = toDelete->previous;
        }
    }
    if(target->first == toDelete){
        target->first = toDelete->next;
    }
    if(target->slotsUsed > 1){
        memcpy(CAST(void *, toDelete), CAST(void *, toDelete+1), CAST(char *, &target->slots[target->slotsUsed]) - CAST(char *, toDelete+1));
        target->slotsUsed--;
        for(i32 i = 0; i < target->slotsUsed; i++){
            DoubleLinkedListElement * e = &target->slots[i];
            if(e->previous >= toDelete){
                e->previous--;
            }
            if(e->next >= toDelete){
                e->next--;
            }
        }
        if(target->first >= toDelete){
            target->first--;
        }
        if(target->head >= toDelete){
            target->head--;
        }
    }else{
        target->slotsUsed = 0;
        target->head = NULL;
        target->first = NULL;
    }
    return true;
}


