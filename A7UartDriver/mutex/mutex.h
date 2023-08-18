#ifndef MUTEX_H
#define MUTEX_H

#include "pthread.h"

class mutx
{
private:
    pthread_mutex_t pt;
public:
    mutx()
    {
        pthread_mutex_init(&pt,NULL);
    }
    int trylock()
    {
        return pthread_mutex_trylock(&pt);
    }
    int lock()
    {
        return pthread_mutex_lock(&pt);
    }
    int unlock()
    {
       return pthread_mutex_unlock(&pt);
    }
    ~mutx()
    {
        pthread_mutex_destroy(&pt);
    }
};

#endif // MUTEX_H
