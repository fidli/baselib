#ifndef UTIL_SORT_C
#define UTIL_SORT_C

#include "util_rng.cpp"


void static inline swap(byte * source, byte * destination, const u16 elemsize){
    byte temp;
    for(u8 bo = 0; bo < elemsize; bo++){
        temp = *(destination + bo);
        *(destination + bo) = *(source + bo);
        *(source + bo) = temp;
    }
}

// cmp true/false if match
template<typename elemType, typename elemType2, typename func>
u32 sortByOtherArray(elemType * target, u32 arraySize, elemType2 * stencilArray, u32 stencilArraySize, func cmp){
    u32 missed = 0;
    for (u32 si = 0; si < stencilArraySize; si++) {
        bool found = false;
        for(u32 ti = si - missed; ti < arraySize && !found; ti++)
        {
            if (cmp(&target[ti], &stencilArray[si]))
            {
                SWAP(target[si], target[ti]);
                found = true;
            }
        }
        if (!found)
        {
            missed++;
        }
    }
    return missed;
}

//positive if a > b
//0 if a == b
//negative if a < b
template<typename elemType, typename func>
void insertSort(elemType * target, u32 arraySize, func cmp){
    for(u32 i = 1; i < arraySize; i++){
        u32 s = i;
        u32 t = i;
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
void shuffle(byte * target, const u16 elemsize, i32 arraySize){
    for(i32 i = arraySize-1; i > 0; i--){
        //terrible rng, but whatever
        u16 randomIndex = randlcgRange(CAST(u16, i+1));
        swap(target + i*elemsize, target+randomIndex*elemsize, elemsize);
    }
    
}

//positive if a > b
//0 if a == b
//negative if a < b
template<typename elemType, typename func>
void mergeSort(elemType * target, i32 chunkSize, func cmp, elemType * temp = NULL){
    if(chunkSize <= 10){
        insertSort(target, chunkSize, cmp);
        return;
    }
    bool uninit = temp == NULL;
    if(uninit){
        temp = &PUSHA(elemType, chunkSize);
    }
    i32 half = chunkSize/2;
    mergeSort(target, half, cmp, temp);
    mergeSort(target + half, chunkSize - half, cmp, temp + half);
    
    i32 index1 = 0;
    i32 index2 = half;
    i32 index = 0;
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
void static inline merge(byte * sourceA, byte * sourceB, byte * workArea,  const u8 elemsize, const u64 sourceASize, const u64 sourceBSize, i8 (*cmp)(void * a, void * b), bool swapTargetAFirst){
    //pour elements into workarea first
    if(swapTargetAFirst){
        for(u64 ci = 0; ci < sourceASize; ci++){
            swap(sourceA + (ci * elemsize), workArea + (ci * elemsize), elemsize);
        }
    }
    u64 sourceAIndex = 0;
    u64 sourceBIndex = 0;
    u64 targetIndex = 0;
    
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

void static inline mergeSubSortIterativeInPlace(byte * sortingArea, byte * workArea, const u8 elemsize, u64 toSortSize, i8 (*cmp)(void * a, void * b)){
    //classical merge sorting half the array using workArea as memory pool
    //iterative version
    
    //no need to sort one or less elements
    if(toSortSize > 1){
        //exponential grow bottom up
        for(u64 chunkSize = 2; chunkSize < toSortSize || (chunkSize/2 < toSortSize && chunkSize >= toSortSize); chunkSize *= 2){
            
            //sliding window, partion the array into independent chunks
            u64 chunkIndexAmount = (toSortSize / chunkSize) + ((toSortSize % chunkSize != 0) ? 1 : 0);
            for(u64 chunkIndex = 0; chunkIndex < chunkIndexAmount; chunkIndex++){
                //chunk needs to be split into two parts, that need to merge, left part and right part are already sorted
                //first part always has at least one element up to chunkSize/2, second part has 0 up to the chunkSize/2 elements
                u64 partOneSize = chunkSize/2;
                u64 partTwoSize = partOneSize;
                //last chunk might not be complete
                if(chunkIndex == chunkIndexAmount - 1){
                    u64 whatsLeft = (toSortSize - (chunkIndex * chunkSize));
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
void mergeSortInPlace(byte * target, const u8 elemsize, u64 arraySize, i8 (*cmp)(void * a, void * b), byte * tempPool = NULL, const u8 tempPoolSize = 0){
    //Jyrki Katajainen, Tomi Pasanen, Jukka Teuhola. ``Practical in-place mergesort''. Nordic Journal of Computing, 1996.
    
    //STEP 1: clasic merge sort half of the array using allocated space in the other half as temp memory
    u64 toSortSize = arraySize / 2;
    u64 workingSize = arraySize - toSortSize;
    
    byte * sortingArea = target + (workingSize * elemsize);
    byte * workArea = target;
    
    mergeSubSortIterativeInPlace(sortingArea, workArea, elemsize, toSortSize, cmp);
    //STEP DONE: second "half" of array is sorted
    
    //STEP2: STEPPING cycle until almost sorted: first half is unsorted, second half is sorted 
    byte * previousSortArea = target + (arraySize * elemsize);
    u64 totallySorted = toSortSize;
    while(workingSize > 2){
        //STEP 2a: now merge sort first half of unsorted area
        //we use second half of unsorted area as workarea
        u64 previousToSortSize = toSortSize;
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
    for(i64 startingIndex = (i64)workingSize - 1; startingIndex >= 0; startingIndex--){
        u64 currentIndex = (u64)startingIndex;
        while(currentIndex + 1 != arraySize && cmp(target + currentIndex*elemsize, target + (currentIndex+1)*elemsize) > 0){
            swap(target + currentIndex*elemsize, target + (currentIndex+1)*elemsize, elemsize);
            currentIndex++;
        } 
    }
    
    
}



//merging happens into sourceA whev swapTargetAFirst is set to true
//otherwise it happens into workarea
void static inline merge4(i32 * sourceA, i32 * sourceB, i32 * workArea,  const u64 sourceASize, const u64 sourceBSize, i8 (*cmp)(void * a, void * b), bool swapTargetAFirst){
    //pour elements into workarea first
    if(swapTargetAFirst){
        for(u64 ci = 0; ci < sourceASize; ci++){
            swap4(sourceA + ci, workArea + ci);
        }
    }
    u64 sourceAIndex = 0;
    u64 sourceBIndex = 0;
    u64 targetIndex = 0;
    
    i32 * target;
    i32 * realSourceA;
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

void static inline mergeSubSortIterativeInPlace4(i32 * sortingArea, i32 * workArea, u64 toSortSize, i8 (*cmp)(void * a, void * b), u16 insertSortSize){
    //classical merge sorting half the array using workArea as memory pool
    //iterative version
    
    
    //no need to sort one or less elements
    if(toSortSize > insertSortSize && toSortSize > 1){
        //exponential grow bottom up
        for(u64 chunkSize = 2; chunkSize < toSortSize || (chunkSize/2 < toSortSize && chunkSize >= toSortSize); chunkSize *= 2){
            
            //sliding window, partion the array into independent chunks
            u64 chunkIndexAmount = (toSortSize / chunkSize) + ((toSortSize % chunkSize != 0) ? 1 : 0);
            for(u64 chunkIndex = 0; chunkIndex < chunkIndexAmount; chunkIndex++){
                //chunk needs to be split into two parts, that need to merge, left part and right part are already sorted
                //first part always has at least one element up to chunkSize/2, second part has 0 up to the chunkSize/2 elements
                u64 partOneSize = chunkSize/2;
                u64 partTwoSize = partOneSize;
                //last chunk might not be complete
                if(chunkIndex == chunkIndexAmount - 1){
                    u64 whatsLeft = (toSortSize - (chunkIndex * chunkSize));
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
                i32 * workSubArea = workArea + (chunkIndex * chunkSize);
                i32 * subTarget = sortingArea + (chunkIndex * chunkSize);
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
void mergeSortInPlace4(i32 * target, u64 arraySize, i8 (*cmp)(void * a, void * b), u16 bubbleSortSize = 2,  const u8 insertSortSize = 0){
    //Jyrki Katajainen, Tomi Pasanen, Jukka Teuhola. ``Practical in-place mergesort''. Nordic Journal of Computing, 1996.
    ASSERT(bubbleSortSize >= 2);
    //STEP 1: clasic merge sort half of the array using allocated space in the other half as temp memory
    u64 toSortSize = arraySize / 2;
    u64 workingSize = arraySize - toSortSize;
    
    i32 * sortingArea = target + (workingSize);
    i32 * workArea = target;
    
    mergeSubSortIterativeInPlace4(sortingArea, workArea, toSortSize, cmp, insertSortSize);
    //STEP DONE: second "half" of array is sorted
    
    //STEP2: STEPPING cycle until almost sorted: first half is unsorted, second half is sorted 
    i32 * previousSortArea = target + (arraySize);
    u64 totallySorted = toSortSize;
    while(workingSize > bubbleSortSize){
        //STEP 2a: now merge sort first half of unsorted area
        //we use second half of unsorted area as workarea
        u64 previousToSortSize = toSortSize;
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
    for(i64 startingIndex = (i64)workingSize - 1; startingIndex >= 0; startingIndex--){
        u64 currentIndex = (u64)startingIndex;
        while(currentIndex + 1 != arraySize && cmp(target + currentIndex, target + (currentIndex+1)) > 0){
            swap4(target + currentIndex, target + (currentIndex+1));
            currentIndex++;
        } 
    }
    
    
}
#endif

#endif
