#ifndef LINUX_THREADS
#define LINUX_THREADS

#include <pthread.h>

#define FETCH_AND_ADD(addr, addon) {__sync_fetch_and_add((addr), (addon));}

struct Thread{
    pthread_t handle;
};

struct Mutex{
    
};

bool createThread(Thread * target, void (*procedure)(void *), void * parameter){
    return pthread_create(&target->handle, NULL, (void * (*)(void*))procedure, parameter) == 0;
    
}

bool closeThread(Thread * target){
    ASSERT(false);
    return false;
}

bool createMutex(Mutex * target){
    ASSERT(false);
    return false;
}
bool destroyMutex(Mutex * target){
    ASSERT(false);
    return false;
}

bool lock(Mutex * target){
    ASSERT(false);
    return false;
}

bool unlock(Mutex * target){
    ASSERT(false);
    return false;
}


bool joinThread(Thread * target){
    return pthread_join(target->handle, NULL) == 0;
}

#endif

