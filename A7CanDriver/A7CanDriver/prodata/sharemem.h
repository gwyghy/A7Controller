/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
****************************************************/

#ifndef __SHAREMEM_H__
#define __SHAREMEM_H__
#include "pro_data.h"
#include <QDebug>
#include "qdebug.h"

//linux共享内存 不继承线程类
template <class T>
class ShareDataT:public creatdata<T>
{
public:
    key_t  shm_id;
    T               *             readp;
public:
    ShareDataT(){
        shm_id = 20130408;
        readp = NULL;
    }
    ~ShareDataT(){
        printf("share data\n");
        if(this->data != NULL && this->creatmark == 1)
            shmctl(shm_id, IPC_RMID, NULL);
        this->data = NULL;
    }

    int creat_data(int size);
    int creat_data(int size, key_t id);
    int read_creat_data(key_t id = 20130408 ,int size = 0);
};

template <class T>
int ShareDataT<T>::creat_data(int siz)
{
    int shmid ;
    void *shmptr ;
    printf("shm id is %d\n", shm_id);
    if((shmid = shmget(shm_id , siz, SHM_R|SHM_W) ) < 0 )
    {
        if((shmid = shmget(shm_id , siz, SHM_R|SHM_W|IPC_CREAT) ) < 0 ){

            return -1;
        }
        this->creatmark = 1;
    }
    shmptr = shmat(shmid, 0, 0 );

    if (shmptr == (void*) -1 ){
        return -2;
    }

    this->data = (T * )shmptr;
    this->size = siz;
    readp = this->data;

    return 0;
}

template <class T>
int ShareDataT<T>::creat_data(int size, key_t id)
{
    shm_id = id;
    return creat_data(size);
}

template <class T>
int ShareDataT<T>::read_creat_data(key_t id ,int siz)
{
    int shmid ;
    void *shmptr ;
     shm_id = id;
    if((shmid = shmget(shm_id , siz, SHM_R|SHM_W) ) < 0 ){

        return -1;
    }
    shmptr = shmat(shmid, 0, 0 );

    if (shmptr == (void*) -1 ){
        return -2;
    }
    this->data = (T * )shmptr;
    this->size = siz;
    this->readp = this->data;
    return 0;

}
//QT共享内存 不继承线程类
template <class T>
class QT_Share_MemT
{
private:
    QSharedMemory dykshare;
public:
    QString     shm_key;
    T   *         data;
public:
    QT_Share_MemT(){
        shm_key = "dykshare";
        dykshare.setKey(shm_key);
    }
    ~QT_Share_MemT(){

        if(dykshare.isAttached())
        {
            dykshare.detach();
        }
         zprintf3("destory QT_Share_MemT!\n");
    }

    int creat_data(int size);
    T* creat_data(int size, QString keyid);
    void lock_qtshare(void);
    void unlock_qtshare(void);
    void set_data(T * addr, T  val);
};

template <class T>
void QT_Share_MemT<T>::lock_qtshare(void)
{
    dykshare.lock();
}
template <class T>
void QT_Share_MemT<T>::unlock_qtshare(void)
{
    dykshare.unlock();
}

template <class T>
int QT_Share_MemT<T>::creat_data(int siz)
{

     dykshare.setKey(shm_key);
    if(dykshare.isAttached())
    {
        zprintf3("qt share have attach\n");
        dykshare.detach();
    }

    if(!dykshare.create(siz))
    {
        zprintf3("QT  Creat Share Fail!\n");

        if(dykshare.error() == 4)       //已经存在
        {
            if(!dykshare.attach())
            {
                zprintf1("can't attatch qt share!\n");
                return -2;
            }
        }
        else
         return -1;
    }

    this->data = (T * )dykshare.data();

    return 0;
}

template <class T>
T* QT_Share_MemT<T>::creat_data(int size, QString keyid)
{
    shm_key = keyid;
    if(creat_data(size) == 0)
        return this->data;
    return NULL;
}

template <class T>
void QT_Share_MemT<T>::set_data(T * addr, T  val)
{
    if(addr == NULL) return ;
    dykshare.lock();
    *addr = val;
    dykshare.unlock();
}

//QT共享内存 不继承线程类
template <class T>
class QTShareDataT:public creatdata<T>
{
private:
    QSharedMemory dykshare;
public:
    QString  shm_key;
public:
    QTShareDataT(){
        shm_key = "dykshare";
        dykshare.setKey(shm_key);
    }
    ~QTShareDataT(){
        if(this->data != NULL && this->creatmark == 1){
            dykshare.detach();
            this->data = NULL;
            this->creatmark = 0;
        }
        zprintf1("destory QTShareDataT!\n");
    }

    int creat_data(int size);
    T* creat_data(int size, QString keyid);
    int read_creat_data(int size , QString keyid = "dykshare");
    void set_data(uint add, T  val);
    void set_data(T * addr, T  val);
    T  get_data(uint add);
    T  get_data(T* addr);
    int  get_data(uint add, T & val);
    int  get_data(T* addr, T & val);
    void noblock_set_data(uint add, T  val);
    T  noblock_get_data(uint add);
    int  noblock_get_data(uint add, T & val);
    void lock_qtshare(void);
    void unlock_qtshare(void);
};

template <class T>
void QTShareDataT<T>::lock_qtshare(void)
{
    dykshare.lock();
}
template <class T>
void QTShareDataT<T>::unlock_qtshare(void)
{
    dykshare.unlock();
}

template <class T>
int QTShareDataT<T>::creat_data(int siz)
{

     dykshare.setKey(shm_key);
    if(dykshare.isAttached())
    {
        zprintf3("qt share have attach\n");
        dykshare.detach();
    }

    if(!dykshare.create(siz))
    {
       zprintf3("QT  Creat Share Fail!\n");

        if(dykshare.error() == 4)       //已经存在
        {
            if(!dykshare.attach())
            {
                zprintf3("can't attatch qt share\n");
                return -2;
            }
        }
        else
         return -1;
    }

    this->creatmark = 1;

    this->data = (T * )dykshare.data();
    this->size = siz;
    this->addp = this->data;

    return 0;
}

template <class T>
T* QTShareDataT<T>::creat_data(int size, QString keyid)
{
    shm_key = keyid;
    if(creat_data(size) == 0)
        return this->data;
    return NULL;
}

template <class T>
int QTShareDataT<T>::read_creat_data(int siz , QString keyid)
{
    shm_key = keyid;

    dykshare.setKey(shm_key);

    if(!dykshare.attach())
    {
        printf("can't attatch qt share\n");
        return -1;
    }
    this->data = (T * )dykshare.data();
    this->size = siz;
    this->addp = this->data;
    return 0;
}

template <class T>
void QTShareDataT<T>::set_data(uint add, T  val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }
    dykshare.lock();
    memcpy(this->data+add, &val, sizeof(T));
    dykshare.unlock();
}

template <class T>
void QTShareDataT<T>::set_data(T * addr, T  val)
{
    if((addr - this->data) >= this->size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }
    dykshare.lock();
    *addr = val;
    dykshare.unlock();
}


template <class T>
T  QTShareDataT<T>::get_data(uint add)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    dykshare.lock();
    mid = *(this->data+add);
    dykshare.unlock();
    return mid;
}

template <class T>
T  QTShareDataT<T>::get_data(T * addr)
{
    if((addr - this->data) >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    dykshare.lock();
    mid = *(addr);
    dykshare.unlock();
    return mid;
}

template <class T>
int  QTShareDataT<T>::get_data(uint add, T & val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }

    dykshare.lock();
    val = *(this->data+add);
    dykshare.unlock();
    return 0;
}
template <class T>
int  QTShareDataT<T>::get_data(T* addr, T & val)
{
    if((addr - this->data) >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }

    dykshare.lock();
    val = *(addr);
    dykshare.unlock();
    return 0;
}

template <class T>
void QTShareDataT<T>::noblock_set_data(uint add, T  val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }
    memcpy(this->data+add, &val, sizeof(T));
}

template <class T>
T  QTShareDataT<T>::noblock_get_data(uint add)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    mid = *(this->data+add);
    return mid;
}

template <class T>
int  QTShareDataT<T>::noblock_get_data(uint add, T & val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }
    val = *(this->data+add);
    return 0;
}





#endif /*__SHAREMEM_H__*/
