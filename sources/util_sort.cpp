#ifndef UTIL_SORT_C
#define UTIL_SORT_C

//cmp 1 if a > b
//0 if a == b
//-1 if a < b
void mergeSort(byte * target, const uint16 elemsize, int64 chunkSize, int8 (*cmp)(void * a, void * b)){
    int64 half = chunkSize/2;
    if(chunkSize > 2){
        mergeSort(target, elemsize, half, cmp);
        mergeSort(target + half*elemsize, elemsize, chunkSize - half, cmp);
    }
    
    int64 index1 = 0;
    int64 index2 = 0;
    
    byte * temp = &PUSHA(byte, elemsize * chunkSize);
    
    for(int64 index = 0; index < chunkSize; index++){
        if(index1 != half && index2 != chunkSize - half){
            if( cmp(&target[index1*elemsize], &(target+half*elemsize)[index2*elemsize]) > 0){
                //this style of copying is kinda slow. IDC now maybe in the future, do in place?
                for(uint16 p = 0; p < elemsize; p++){
                    temp[index*elemsize + p] = target[index1*elemsize + p];
                }
                index1++;
            }
            else{
                for(uint16 p = 0; p < elemsize; p++){
                    temp[index*elemsize + p] = (target+half*elemsize)[index2*elemsize + p];
                }
                index2++;
            }
        }else{
            if(index1 == half){
                for(uint16 p = 0; p < elemsize; p++){
                    temp[index*elemsize + p] = (target+half*elemsize)[index2*elemsize + p];
                }
                index2++;
            }
            else{
                for(uint16 p = 0; p < elemsize; p++){
                    temp[index*elemsize + p] = target[index1*elemsize + p];
                }
                index1++;
            }
        }
    }
    //again slow copying
    for(int i = 0; i < chunkSize * elemsize; i++){
        target[i] = temp[i];
    }
    
    POP;
}


#endif