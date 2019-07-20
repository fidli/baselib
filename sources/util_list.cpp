#ifndef UTIL_LIST
#define UTIL_LIST

struct DoubleLinkedListElement{
	void * data;
	char key[20];
	DoubleLinkedListElement * previous;
	DoubleLinkedListElement * next;
};

struct DoubleLinkedList{
	uint32 size;
	DoubleLinkedListElement ** elements;
	DoubleLinkedListElement * head;
	DoubleLinkedListElement * first;
	
	
	DoubleLinkedListElement * slots;
	uint32 slotsUsed;
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

bool createDoubleLinkedList(DoubleLinkedList * target, int32 size){
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




 

#endif