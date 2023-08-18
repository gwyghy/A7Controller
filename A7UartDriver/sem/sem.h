#ifndef SEM_H
#define SEM_H
#include "semaphore.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include<stdlib.h>
#include<errno.h>
#include "stdio.h"

class semt
{
private:
    sem_t msgsem;
public:
    semt()
    {
        sem_init(&msgsem,0,0);
    }
    void post_sem()
    {
        sem_post(&msgsem);
    }
    int wait_sem()
    {
        sem_wait(&msgsem);
        return 0;
    }
    ~semt()
    {
        sem_destroy(&msgsem);
    }
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

class semp
{
public:
    semp()
    {
        sem_id = -1;
    }
    int create_sem(key_t  key,int val)
    {
        sem_id = semget(key, 1, 0666 | IPC_CREAT);

        if (sem_id == -1)
        {
            if(errno == ENOENT)
            {
                sem_id = semget(key, 1,IPC_CREAT| 0666);
                if (sem_id == -1)
                {
                    perror("Sem Create Error!\n");
                    return -1;
                }
                if(init_sem(val) == -1)
                {
                    return -1;
                }
            }
            else
            {
                perror("Sem Get Error!\n");
                return -1;
            }
        }
        else
        {
            if(init_sem(val) == -1)
            {
                perror("sem init fail\n");
                return -1;
            }
        }
        return 0;
    }
    int init_sem(int init_value)
    {
        union semun sem_union;
        sem_union.val = init_value;
        if (semctl(sem_id,0,SETVAL,sem_union)==-1)
        {
            perror("Sem init");
            return -1;
        }
        return 0;
    }
    int del_sem()
    {
        union semun sem_union;
        if (semctl(sem_id,0,IPC_RMID,sem_union)==-1) {
            perror("Sem delete");
            return -1;
        }
        return 0;
    }
    int sem_p()
    {
        struct sembuf sem_buf;
        sem_buf.sem_num=0;//信号量编号
        sem_buf.sem_op=-1;//P操作
        sem_buf.sem_flg=0;//系统退出前未释放信号量，系统自动释放

        if (semop(sem_id,&sem_buf,1)==-1)
        {
            perror("Sem P operation");
            return -1;
        }
        return 0;
    }
    int sem_v()
    {
        struct sembuf sem_buf;
        sem_buf.sem_num=0;
        sem_buf.sem_op=1;//V操作
        sem_buf.sem_flg=0;
        if (semop(sem_id,&sem_buf,1)==-1)
        {
            perror("Sem V operation");
            return -1;
        }
        return 0;
    }
    int sem_p(int milliseconds)
    {
        struct sembuf sem_buf;
        struct timespec  timeout;
        //    int ret;
        sem_buf.sem_num=0;//信号量编号
        sem_buf.sem_op=-1;//P操作
        sem_buf.sem_flg=0;//系统退出前未释放信号量，系统自动释放

        timeout.tv_sec = (milliseconds / 1000);
        timeout.tv_nsec = (milliseconds - timeout.tv_sec*1000L)*1000000L;

        return(semtimedop(sem_id,&sem_buf,1, &timeout));
    }
    int get_sem_count(int sem_id)
    {
        union semun sem_union;
        return semctl(sem_id,0, GETVAL, sem_union);
    }
    int get_semid()
    {
        return sem_id;
    }
private:
    int sem_id;
};

#endif // SEM_H
