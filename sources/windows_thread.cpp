#ifndef WINDOWS_THREADS
#define WINDOWS_THREADS

#define THREAD_ID DWORD

#define FETCH_AND_ADD(addr, addon) {ASSERT((addon) == 1); InterlockedIncrement((addr));}


struct Thread{
    HANDLE handle;
};

struct Mutex{
    HANDLE handle;
};

bool createThread(Thread * target, void (*procedure)(void *), void * parameter){
    target->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) procedure, parameter, 0, NULL);
    return target->handle != NULL;
    
}

bool closeThread(Thread * target){
    return CloseHandle(target->handle);
}

bool createMutex(Mutex * target){
    target->handle = CreateMutex(NULL, FALSE, NULL);
    return target->handle != NULL;
}
bool destroyMutex(Mutex * target){
    return CloseHandle(target->handle);
}

bool lock(Mutex * target){
    return WAIT_OBJECT_0 == WaitForSingleObject(target->handle, INFINITE);
}

bool unlock(Mutex * target){
    return ReleaseMutex(target->handle) > 0;
}


bool joinThread(Thread * target){
    return WAIT_OBJECT_0 == WaitForSingleObject(target->handle, INFINITE);
}

#endif