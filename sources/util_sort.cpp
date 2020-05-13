#ifndef UTIL_SORT_C
#define UTIL_SORT_C

#include "util_rng.cpp"


void static inline swap(byte * source, byte * destination, const uint16 elemsize){
    byte temp;
    for(uint8 bo = 0; bo < elemsize; bo++){
        temp = *(destination + bo);
        *(destination + bo) = *(source + bo);
        *(source + bo) = temp;
    }
}

//positive if a > b
//0 if a == b
//negative if a < b
template<typename elemType, typename func>
void insertSort(elemType * target, int32 arraySize, func cmp){
    for(uint32 i = 1; i < arraySize; i++){
        uint32 s = i;
        uint32 t = i;
        do{
            t--;
            if(cmp(target[t], target[s]) > 0){
                SWAP(target[s], target[t]);
                s--;
            }else{
                break;
            }
        }while(t != 0);
        
    }
}

//fisher yates - https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
void shuffle(byte * target, const uint16 elemsize, int64 arraySize){
    for(int32 i = arraySize-1; i > 0; i--){
        //terrible rng, but whatever
        uint16 randomIndex = randlcgRange(i+1);
        swap(target + i*elemsize, target+randomIndex*elemsize, elemsize);
    }
    
}

//positive if a > b
//0 if a == b
//negative if a < b
template<typename elemType, typename func>
void mergeSort(elemType * target, int32 chunkSize, func cmp, elemType * temp = NULL){
    if(chunkSize <= 10){
        insertSort(target, chunkSize, cmp);
        return;
    }
    bool uninit = temp == NULL;
    if(uninit){
        temp = &PUSHA(elemType, chunkSize);
    }
    int32 half = chunkSize/2;
    mergeSort(target, half, cmp, temp);
    mergeSort(target + half, chunkSize - half, cmp, temp + half);
    
    int32 index1 = 0;
    int32 index2 = half;
    int32 index = 0;
    for(; index1 < half && index2 < chunkSize; index++){
        if(cmp(target[index1], target[index2]) <= 0){
            temp[index] = target[index1];
            index1++;
        }else{
            temp[index] = target[index2];
            index2++;
        }
    }
    if(index1 == half){
        for(; index < chunkSize; index++, index2++){
            temp[index] = target[index2];
        }
    }else{
        for(; index < chunkSize; index++, index1++){
            temp[index] = target[index1];
        }
    }
    //again slow copying
    for(int i = 0; i < chunkSize; i++){
        target[i] = temp[i];
    }
    if(uninit){
        POP;
    }
    
}

#if 0


//merging happens into sourceA whev swapTargetAFirst is set to true
//otherwise it happens into workarea
void static inline merge(byte * sourceA, byte * sourceB, byte * workArea,  const uint8 elemsize, const uint64 sourceASize, const uint64 sourceBSize, int8 (*cmp)(void * a, void * b), bool swapTargetAFirst){
    //pour elements into workarea first
    if(swapTargetAFirst){
        for(uint64 ci = 0; ci < sourceASize; ci++){
            swap(sourceA + (ci * elemsize), workArea + (ci * elemsize), elemsize);
        }
    }
    uint64 sourceAIndex = 0;
    uint64 sourceBIndex = 0;
    uint64 targetIndex = 0;
    
    byte * target;
    byte * realSourceA;
    if(swapTargetAFirst){
        target = sourceA;
        realSourceA = workArea;
    }else{
        target = workArea;
        realSourceA = sourceA;
    }
    
    while(sourceAIndex < sourceASize && sourceBIndex < sourceBSize){
        byte * firstElement = realSourceA + (sourceAIndex * elemsize);
        byte * secondElement = sourceB + (sourceBIndex * elemsize);
        
        if(cmp(firstElement, secondElement) > 0){
            //secondElement is smaller
            swap(secondElement, target + (targetIndex * elemsize), elemsize);
            sourceBIndex++;
        }else{
            swap(firstElement, target + (targetIndex * elemsize), elemsize);
            sourceAIndex++;
        }
        targetIndex++;
    }
    //copy rest
    while(sourceAIndex < sourceASize){
        swap(realSourceA + (sourceAIndex * elemsize), target + (targetIndex * elemsize), elemsize);
        sourceAIndex++;
        targetIndex++;
    }
    while(sourceBIndex < sourceBSize){
        swap(sourceB + (sourceBIndex * elemsize), target + (targetIndex * elemsize), elemsize);
        sourceBIndex++;
        targetIndex++;
    }
}

void static inline mergeSubSortIterativeInPlace(byte * sortingArea, byte * workArea, const uint8 elemsize, uint64 toSortSize, int8 (*cmp)(void * a, void * b)){
    //classical merge sorting half the array using workArea as memory pool
    //iterative version
    
    //no need to sort one or less elements
    if(toSortSize > 1){
        //exponential grow bottom up
        for(uint64 chunkSize = 2; chunkSize < toSortSize || (chunkSize/2 < toSortSize && chunkSize >= toSortSize); chunkSize *= 2){
            
            //sliding window, partion the array into independent chunks
            uint64 chunkIndexAmount = (toSortSize / chunkSize) + ((toSortSize % chunkSize != 0) ? 1 : 0);
            for(uint64 chunkIndex = 0; chunkIndex < chunkIndexAmount; chunkIndex++){
                //chunk needs to be split into two parts, that need to merge, left part and right part are already sorted
                //first part always has at least one element up to chunkSize/2, second part has 0 up to the chunkSize/2 elements
                uint64 partOneSize = chunkSize/2;
                uint64 partTwoSize = partOneSize;
                //last chunk might not be complete
                if(chunkIndex == chunkIndexAmount - 1){
                    uint64 whatsLeft = (toSortSize - (chunkIndex * chunkSize));
                    if(whatsLeft <= partOneSize){
                        //nothing to merge
                        break;
                    }else{
                        //part two has at least one element
                        partTwoSize = whatsLeft - partOneSize;
                    }
                }
                
                //start merging process
                //"copy" firstPart to working area and then merge
                byte * workSubArea = workArea + (chunkIndex * elemsize * chunkSize);
                byte * subTarget = sortingArea + (chunkIndex * elemsize * chunkSize);
                //merge call
                merge(subTarget, subTarget + (partOneSize * elemsize), workSubArea, elemsize, partOneSize, partTwoSize, cmp, true);
            }
        }
        
        //merge sorting done
    }
}

//cmp 1 if a > b
//0 if a == b
//-1 if a < b
void mergeSortInPlace(byte * target, const uint8 elemsize, uint64 arraySize, int8 (*cmp)(void * a, void * b), byte * tempPool = NULL, const uint8 tempPoolSize = 0){
    //Jyrki Katajainen, Tomi Pasanen, Jukka Teuhola. ``Practical in-place mergesort''. Nordic Journal of Computing, 1996.
    
    //STEP 1: clasic merge sort half of the array using allocated space in the other half as temp memory
    uint64 toSortSize = arraySize / 2;
    uint64 workingSize = arraySize - toSortSize;
    
    byte * sortingArea = target + (workingSize * elemsize);
    byte * workArea = target;
    
    mergeSubSortIterativeInPlace(sortingArea, workArea, elemsize, toSortSize, cmp);
    //STEP DONE: second "half" of array is sorted
    
    //STEP2: STEPPING cycle until almost sorted: first half is unsorted, second half is sorted 
    byte * previousSortArea = target + (arraySize * elemsize);
    uint64 totallySorted = toSortSize;
    while(workingSize > 2){
        //STEP 2a: now merge sort first half of unsorted area
        //we use second half of unsorted area as workarea
        uint64 previousToSortSize = toSortSize;
        previousSortArea -= previousToSortSize * elemsize;
        sortingArea = target;
        toSortSize = workingSize/2;
        workingSize = workingSize - toSortSize;
        workArea = sortingArea + (toSortSize * elemsize);
        
        if(toSortSize <= tempPoolSize){
            mergeSort(sortingArea, elemsize, toSortSize, cmp, tempPool);
        }else{
            mergeSubSortIterativeInPlace(sortingArea, workArea, elemsize, toSortSize, cmp);
        }
        
        //STEP 2b: merge first sorted half with already sorted from previous runs
        //we use same workarea, but without copying
        //might wanna not use start of the working area, as that could leave unsorted elements at the end of the original array
        merge(sortingArea, previousSortArea, target + (arraySize - (totallySorted+toSortSize)) * elemsize, elemsize, toSortSize, totallySorted, cmp, false);
        totallySorted += toSortSize;
    }
    
    //STEPs DONE: we have small unsorted part  and sorted part
    
    //STEP3 "bubble sort" tiny remaining part into rest of the array:
    for(int64 startingIndex = (int64)workingSize - 1; startingIndex >= 0; startingIndex--){
        uint64 currentIndex = (uint64)startingIndex;
        while(currentIndex + 1 != arraySize && cmp(target + currentIndex*elemsize, target + (currentIndex+1)*elemsize) > 0){
            swap(target + currentIndex*elemsize, target + (currentIndex+1)*elemsize, elemsize);
            currentIndex++;
        } 
    }
    
    
}



//merging happens into sourceA whev swapTargetAFirst is set to true
//otherwise it happens into workarea
void static inline merge4(int32 * sourceA, int32 * sourceB, int32 * workArea,  const uint64 sourceASize, const uint64 sourceBSize, int8 (*cmp)(void * a, void * b), bool swapTargetAFirst){
    //pour elements into workarea first
    if(swapTargetAFirst){
        for(uint64 ci = 0; ci < sourceASize; ci++){
            swap4(sourceA + ci, workArea + ci);
        }
    }
    uint64 sourceAIndex = 0;
    uint64 sourceBIndex = 0;
    uint64 targetIndex = 0;
    
    int32 * target;
    int32 * realSourceA;
    if(swapTargetAFirst){
        target = sourceA;
        realSourceA = workArea;
    }else{
        target = workArea;
        realSourceA = sourceA;
    }
    
    while(sourceAIndex < sourceASize && sourceBIndex < sourceBSize){
        int32* firstElement = realSourceA + (sourceAIndex);
        int32* secondElement = sourceB + (sourceBIndex);
        
        if(cmp(firstElement, secondElement) > 0){
            //secondElement is smaller
            swap4(secondElement, target + (targetIndex));
            sourceBIndex++;
        }else{
            swap4(firstElement, target + (targetIndex));
            sourceAIndex++;
        }
        targetIndex++;
    }
    //copy rest
    while(sourceAIndex < sourceASize){
        swap4(realSourceA + (sourceAIndex), target + (targetIndex));
        sourceAIndex++;
        targetIndex++;
    }
    while(sourceBIndex < sourceBSize){
        swap4(sourceB + (sourceBIndex), target + (targetIndex));
        sourceBIndex++;
        targetIndex++;
    }
}

void static inline mergeSubSortIterativeInPlace4(int32 * sortingArea, int32 * workArea, uint64 toSortSize, int8 (*cmp)(void * a, void * b), uint16 insertSortSize){
    //classical merge sorting half the array using workArea as memory pool
    //iterative version
    
    
    //no need to sort one or less elements
    if(toSortSize > insertSortSize && toSortSize > 1){
        //exponential grow bottom up
        for(uint64 chunkSize = 2; chunkSize < toSortSize || (chunkSize/2 < toSortSize && chunkSize >= toSortSize); chunkSize *= 2){
            
            //sliding window, partion the array into independent chunks
            uint64 chunkIndexAmount = (toSortSize / chunkSize) + ((toSortSize % chunkSize != 0) ? 1 : 0);
            for(uint64 chunkIndex = 0; chunkIndex < chunkIndexAmount; chunkIndex++){
                //chunk needs to be split into two parts, that need to merge, left part and right part are already sorted
                //first part always has at least one element up to chunkSize/2, second part has 0 up to the chunkSize/2 elements
                uint64 partOneSize = chunkSize/2;
                uint64 partTwoSize = partOneSize;
                //last chunk might not be complete
                if(chunkIndex == chunkIndexAmount - 1){
                    uint64 whatsLeft = (toSortSize - (chunkIndex * chunkSize));
                    if(whatsLeft <= partOneSize){
                        //nothing to merge
                        break;
                    }else{
                        //part two has at least one element
                        partTwoSize = whatsLeft - partOneSize;
                    }
                }
                
                //start merging process
                //"copy" firstPart to working area and then merge
                int32 * workSubArea = workArea + (chunkIndex * chunkSize);
                int32 * subTarget = sortingArea + (chunkIndex * chunkSize);
                //merge call
                merge4(subTarget, subTarget + (partOneSize), workSubArea, partOneSize, partTwoSize, cmp, true);
            }
        }
        
        //merge sorting done
    }else{
        if(toSortSize > 1){
            //other sort
            insertSort4(sortingArea, toSortSize, cmp);
        }
    }
}



//cmp 1 if a > b
//0 if a == b
//-1 if a < b
void mergeSortInPlace4(int32 * target, uint64 arraySize, int8 (*cmp)(void * a, void * b), uint16 bubbleSortSize = 2,  const uint8 insertSortSize = 0){
    //Jyrki Katajainen, Tomi Pasanen, Jukka Teuhola. ``Practical in-place mergesort''. Nordic Journal of Computing, 1996.
    ASSERT(bubbleSortSize >= 2);
    //STEP 1: clasic merge sort half of the array using allocated space in the other half as temp memory
    uint64 toSortSize = arraySize / 2;
    uint64 workingSize = arraySize - toSortSize;
    
    int32 * sortingArea = target + (workingSize);
    int32 * workArea = target;
    
    mergeSubSortIterativeInPlace4(sortingArea, workArea, toSortSize, cmp, insertSortSize);
    //STEP DONE: second "half" of array is sorted
    
    //STEP2: STEPPING cycle until almost sorted: first half is unsorted, second half is sorted 
    int32 * previousSortArea = target + (arraySize);
    uint64 totallySorted = toSortSize;
    while(workingSize > bubbleSortSize){
        //STEP 2a: now merge sort first half of unsorted area
        //we use second half of unsorted area as workarea
        uint64 previousToSortSize = toSortSize;
        previousSortArea -= previousToSortSize;
        sortingArea = target;
        toSortSize = workingSize/2;
        workingSize = workingSize - toSortSize;
        workArea = sortingArea + (toSortSize);
        
        
        mergeSubSortIterativeInPlace4(sortingArea, workArea, toSortSize, cmp, insertSortSize);
        
        
        //STEP 2b: merge first sorted half with already sorted from previous runs
        //we use same workarea, but without copying
        //might wanna not use start of the working area, as that could leave unsorted elements at the end of the original array
        merge4(sortingArea, previousSortArea, target + (arraySize - (totallySorted+toSortSize)), toSortSize, totallySorted, cmp, false);
        totallySorted += toSortSize;
    }
    
    //STEPs DONE: we have small unsorted part  and sorted part
    
    //STEP3 "bubble sort" tiny remaining part into rest of the array:
    for(int64 startingIndex = (int64)workingSize - 1; startingIndex >= 0; startingIndex--){
        uint64 currentIndex = (uint64)startingIndex;
        while(currentIndex + 1 != arraySize && cmp(target + currentIndex, target + (currentIndex+1)) > 0){
            swap4(target + currentIndex, target + (currentIndex+1));
            currentIndex++;
        } 
    }
    
    
}
#endif

#endif
