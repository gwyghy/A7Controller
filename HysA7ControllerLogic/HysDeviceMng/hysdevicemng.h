#ifndef HYSDEVICEMNG_H
#define HYSDEVICEMNG_H

/********/
#include <QMutex>
#include "devctrl.h"

/********/
class HysDeviceMng
{
public:
    HysDeviceMng();
    ~HysDeviceMng();
    static HysDeviceMng *getInston();

    void Init(void);
    /******/
    int HysSystemAppRecvShmProc(void);
    int HysSystemDevPeriodicProc();

private:
    void HysDeviceMngInit(void);
    void HysDeviceStatusInit(void);
    /******/
    static int AppRecvShmCallback(uchar childdevtype,uchar devtype,ushort numb,ushort frametype,uchar*data,int len);

    int DevRecvShmProc(uchar drivertype,uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len);
    static int DevCanRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len);
    static int DevSelfAdRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len);
    static int DevSelfIoRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len);
    static int DevUartRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len);
    static int DevI2CRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len);

    static int RecvMsgCallback(uint8_t childdevtype, uint8_t devtype, ushort numb, uchar *data, int len);

    /**Data,长度\偏移\数据，length总和长度**/
    int GenerateDataProTab(void);
    /******/
    int HysSystemDevRecvShmProc(void);
    int HysSystemDevCommLedStatusProc(int delay);

    /******/
    int HysSystemEmvdActHeartbeatProc(int delay);
private:
    /****/
    static HysDeviceMng* that;

private:
    /**数据接收相关***/
    /**应用层，特指架间数据**/
    int AppShmRecvTotalumber;
    int AppShmHaveProcTotalumber;
    QList<HysSystemRecvShmClass *> AppRecvDataList;
    QMutex *m_AppRecvMutex;//2023.01.09,为排查故障，改变互斥量的类型，mry
    QMap<u_int16_t,HysAppShmProcFuncType> appshmproctab;//应用层共享内存的处理

    /**设备层**/
    int DevShmRecvTotalumber;
    int DevShmHaveProcTotalumber;
    QList<HysDevRecvShmClass *> DevRecvDataList;
    QMutex *m_DevRecvMutex;//2023.01.09,为排查故障，改变互斥量的类型，mry
    QMap<u_int16_t,HysDevShmProcFuncType> devshmproctab;//设备层共享内存的处理

    /***消息相关***/
    QMap<u_int16_t,HysMsgProcFuncType> msgproctab;//消息类型，处理函数
};

#endif // HYSDEVICEMNG_H
