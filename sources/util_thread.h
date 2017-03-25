#ifndef UTIL_THREAD
#define UTIL_THREAD


extern struct Thread;
    
extern struct Mutex;

bool createThread(Thread * target, void (*procedure)(void *), void * parameter);
    
bool closeThread(Thread * target);
    bool joinThread(Thread * target);


bool createMutex(Mutex * target);
bool destroyMutex(Mutex * target);
    
 bool lock(Mutex * target);
 bool unlock(Mutex * target);


#endif