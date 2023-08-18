#include "devctrl.h"
#include "devicemngapi.h"
#include <QMutex>
#include "./HysSystem/hyssystem.h"
#include <QDebug>
#include <QDateTime>

/***定义驱动类型与程序号设备号的对应关系,除网络设备之外***/
static QMap<int,DevDriveIdTransferType> DeviceTranferMap;

static QMutex m_ctrl_mutex;
/*******************************************************************************/
/*!
 * \brief DevCtrlInit,对设备控制的初始化
 * \return 0正常负数异常
 */
int DevCtrlInit(void)
{
    DevDriveIdTransferType temp;

    m_ctrl_mutex.lock();
    temp.ProgramId = CAN_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_1;
    DeviceTranferMap.insert(static_cast<int>(DRIVER_CAN1),temp);

    temp.ProgramId = CAN_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_2;
    DeviceTranferMap.insert(static_cast<int>(DRIVER_CAN2),temp);

    temp.ProgramId = SELF_AD_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_1;
    DeviceTranferMap.insert(DRIVER_AD,temp);

    temp.ProgramId = SELF_IO_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_1;
    DeviceTranferMap.insert(DRIVER_IO,temp);

    temp.ProgramId = SELF_UART_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_1;
    DeviceTranferMap.insert(DRIVER_UART2,temp);

    temp.ProgramId = SELF_UART_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_2;
    DeviceTranferMap.insert(DRIVER_UART3,temp);

    temp.ProgramId = SELF_UART_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_3;
    DeviceTranferMap.insert(DRIVER_UART4,temp);

    temp.ProgramId = SELF_UART_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_4;
    DeviceTranferMap.insert(DRIVER_UART5,temp);

    temp.ProgramId = SELF_I2C_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_1;
    DeviceTranferMap.insert(DRIVER_I2C,temp);

    temp.ProgramId = UDP_PROGRAM_ID;
    temp.DeviceId_HardWareNumber = HARDWARE_NUMBER_0;
    DeviceTranferMap.insert(DRIVER_NET,temp);

    m_ctrl_mutex.unlock();

    return 0;
}

/*!
 * \brief DevCtrlGetProgramAndHardWareInf
 * \param driverid
 * \param programid
 * \param hardwareid
 * \return
 */
int DevCtrlGetProgramAndHardWareInf(u_int16_t driverid, uchar &programid, uchar &hardwareid)
{
    if(DeviceTranferMap.contains(driverid))
    {
        programid = DeviceTranferMap[driverid].ProgramId;
        hardwareid = DeviceTranferMap[driverid].DeviceId_HardWareNumber;
        return 0;
    }

    return -1;
}
/*!
 * \brief DevCtrlInquire，控制设备请求(非电话,非网络),触发方式。控制设备的动作，对应包类型为7
 * \param scid
 * \param driverid
 * \param devid
 * \param childdevid
 * \param point
 * \param pbuf
 * \param length
 * \return 0正常负数异常
 */
int DevCtrlInquire(u_int16_t driverid, u_int16_t devid, \
                   u_int16_t childdevid, u_int16_t point, \
                   u_int8_t *pbuf, int length)
{
    if(DeviceTranferMap.contains(driverid) == false)
        return -1;

    DevDriveIdTransferType temp = DeviceTranferMap[driverid];

    #if 0//for test
        if(devid == EXTDEV_TYPE_SELF_IO)
        {
            /*****/
            QDateTime time;
            time = QDateTime::currentDateTime();
            qDebug()<<time.toString("yyyy-MM-dd hh:mm:ss.zzz")<<"DevCtrlInquire:"<<temp.ProgramId<<temp.DeviceId_HardWareNumber\
                   <<devid<<childdevid<<point<<V4DRIVER_STATUS_CTRL_DEV<<length<<pbuf[0]<<pbuf[1];
        }
    #endif

    if(devid == EXTDEV_TYPE_EMVD)
    {
        u_int16_t ctrlmode = 0;
        memmove(&ctrlmode,pbuf,0x02);
        if(ctrlmode == DEV_POINT_CTRL_ON)
            DevEmvdInsertRealTimeActList(point);
        else if(ctrlmode == DEV_POINT_CTRL_OFF)
            DevEmvdDeleteRealTimeActList(point);
    }
    DeviceMngApi *pDevApi = DeviceMngApi::getInston();
    pDevApi->writeshmp(temp.ProgramId,temp.DeviceId_HardWareNumber|static_cast<uchar>(devid),\
                       static_cast<uchar>(childdevid),static_cast<uchar>(point),V4DRIVER_STATUS_CTRL_DEV,pbuf,length);
    return 0;
}


/*!
 * \brief DevCtrlContinue 控制设备命令(非电话,非网络),持续发送方式
 * \param scid，控制器编号
 * \param driverid，驱动号
 * \param devid，设备号
 * \param childdevid，子设备号
 * \param point，点号
 * \param pbuf，数据指针
 * \param length,数据长度
 * \return 0正常负数异常
 */
int DevCtrlContinue(u_int16_t scid, u_int16_t driverid,u_int16_t devid,\
                    u_int16_t childdevid,u_int16_t point,void *pbuf,\
                    int length)
{

}

/*!
 * \brief DevCtrlPhoneInquire,控制电话的请求，预留功能
 * \param phoneAddr,电话地址
 * \param pdata，数据指，数据长度
 * \param length，数据长度
 * \return 0正常负数异常
 */
int DevCtrlPhoneInquire(u_int16_t phoneAddr,void *pdata,u_int16_t length)
{
    return 0;
}

/*!
 * \brief UdpAppProcCtrl,发送应用层数据
 * \param childdevid，子设备编号，此处为设备类型
 * \param numb，设备编号
 * \param frametype，包标识符
 * \param pdata，数据指针
 * \param length，数据长度
 * \return 0正常负数异常
 */
int UdpAppProcCtrl(uchar childdevid,uchar numb,u16 frametype,u_int8_t *pdata,u16 length)
{
    if((frametype == 7)&&(*(u_int8_t *)(pdata+1) == 2))
    {
        frametype = 7;
        qDebug()<<"********UdpAppProcCtrl***group ctrl****"<<childdevid<<numb<<frametype<<*(u_int8_t *)pdata<<*(u_int8_t *)(pdata+1)<<*(u_int8_t *)(pdata+2);
    }
    HysSystem *pSystem = HysSystem::getInston();
    u_int16_t devid = 0;

    if(false == pSystem->GetBasicScType(devid))
        return -1;

    DeviceMngApi * pDevApi = DeviceMngApi::getInston();
    pDevApi->writeshmp(UDP_PROGRAM_ID,static_cast<uchar>(devid),\
                       childdevid,numb,frametype,pdata,length);

    return 0;
}







