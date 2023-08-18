#include "devglobaltype.h"
#include "./HysSystem/hyssystem.h"
#include "./Sensor/sensor.h"


/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;
/*********/
static QList<AppUpdateFilePathType> mUpdateFilePathList;

/***定义设备程序更新时的路径及文件名称***/
int DevUpdateFilePathInit(void)
{
    mUpdateFilePathList.clear();

    AppUpdateFilePathType temptab[] =
    {
        /***应用程序***/
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_SC,                 QString(APP_PROGRAM_DOWN_FILE_PATH),QString(SC_PROGRAM_APP_FILE_NAME),          DRIVER_NET,     EXTDEV_TYPE_SC},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_EMVD,               QString(APP_PROGRAM_DOWN_FILE_PATH),QString(EMVD_PROGRAM_APP_BOOT_FILE_NAME),   DRIVER_CAN2,    EXTDEV_TYPE_EMVD},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_ANGLE,              QString(APP_PROGRAM_DOWN_FILE_PATH),QString(ANGLE_PROGRAM_APP_BOOT_FILE_NAME),  DRIVER_CAN1,    EXTDEV_TYPE_ANGLE},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_HIGH,               QString(APP_PROGRAM_DOWN_FILE_PATH),QString(HEIGHT_PROGRAM_APP_BOOT_FILE_NAME), DRIVER_CAN1,    EXTDEV_TYPE_HIGH},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_ALARM,              QString(APP_PROGRAM_DOWN_FILE_PATH),QString(ALARM_PROGRAM_APP_BOOT_FILE_NAME),  DRIVER_CAN1,    EXTDEV_TYPE_ALARM},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_PERSON,             QString(APP_PROGRAM_DOWN_FILE_PATH),QString(PERSON_PROGRAM_APP_BOOT_FILE_NAME), DRIVER_CAN1,    EXTDEV_TYPE_PERSON},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_IRRX,               QString(APP_PROGRAM_DOWN_FILE_PATH),QString(IRRX_PROGRAM_APP_BOOT_FILE_NAME),   DRIVER_CAN1,    EXTDEV_TYPE_IRRX},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_KEYBOARD,           QString(APP_PROGRAM_DOWN_FILE_PATH),QString(KEY_PROGRAM_APP_BOOT_FILE_NAME),    DRIVER_UART2,   EXTDEV_TYPE_KEYBOARD},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_WIRELESS,           QString(APP_PROGRAM_DOWN_FILE_PATH),QString(WIRELESS_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_UART3,  EXTDEV_TYPE_WIRELESS},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_HUB,                QString(APP_PROGRAM_DOWN_FILE_PATH),QString(HUB_PROGRAM_APP_BOOT_FILE_NAME),    DRIVER_CAN1,    EXTDEV_TYPE_HUB_1},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_ANGLE_HEIGHT,       QString(APP_PROGRAM_DOWN_FILE_PATH),QString(HEIGHT_ANGLE_PROGRAM_APP_BOOT_FILE_NAME), DRIVER_CAN1,    EXTDEV_TYPE_HIGH},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_MULTI_SOUND,        QString(APP_PROGRAM_DOWN_FILE_PATH),QString(MULTI_ALARM_PROGRAM_APP_BOOT_FILE_NAME),  DRIVER_CAN1,    EXTDEV_TYPE_ALARM},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_DATA_CONVERTER,     QString(APP_PROGRAM_DOWN_FILE_PATH),QString(DATA_TRANSFER_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,     EXTDEV_TYPE_DATA_CONVERTER},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_SIGNAL_CONVERTER,   QString(APP_PROGRAM_DOWN_FILE_PATH),QString(LIGHT_TRANSFER_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,    EXTDEV_TYPE_DATA_CONVERTER},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_LIGHT_ELECTRIL_TERMINAL, QString(APP_PROGRAM_DOWN_FILE_PATH),QString(LIGHT_TERMINAL_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,EXTDEV_TYPE_DATA_CONVERTER},
        {UPDATE_PRG_PRGTYPE_PROGRAM, UPDATE_PRG_DEVTYPE_PHONE,                   QString(APP_PROGRAM_DOWN_FILE_PATH),QString(TELEPHONE_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,     EXTDEV_TYPE_DATA_CONVERTER},

        /***引导程序***/
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_SC,                  QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(SC_PROGRAM_BOOT_FILE_NAME),         DRIVER_NET,     EXTDEV_TYPE_SC},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_EMVD,                QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(EMVD_PROGRAM_APP_BOOT_FILE_NAME),   DRIVER_CAN2,    EXTDEV_TYPE_EMVD},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_ANGLE,               QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(ANGLE_PROGRAM_APP_BOOT_FILE_NAME),  DRIVER_CAN1,    EXTDEV_TYPE_ANGLE},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_HIGH,                QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(HEIGHT_PROGRAM_APP_BOOT_FILE_NAME), DRIVER_CAN1,    EXTDEV_TYPE_HIGH},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_ALARM,               QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(ALARM_PROGRAM_APP_BOOT_FILE_NAME),  DRIVER_CAN1,    EXTDEV_TYPE_ALARM},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_PERSON,              QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(PERSON_PROGRAM_APP_BOOT_FILE_NAME), DRIVER_CAN1,    EXTDEV_TYPE_PERSON},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_IRRX,                QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(IRRX_PROGRAM_APP_BOOT_FILE_NAME),   DRIVER_CAN1,    EXTDEV_TYPE_IRRX},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_KEYBOARD,            QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(KEY_PROGRAM_APP_BOOT_FILE_NAME),    DRIVER_UART2,   EXTDEV_TYPE_KEYBOARD},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_WIRELESS,            QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(WIRELESS_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_UART3,  EXTDEV_TYPE_WIRELESS},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_HUB,                 QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(HUB_PROGRAM_APP_BOOT_FILE_NAME),    DRIVER_CAN1,    EXTDEV_TYPE_HUB_1},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_ANGLE_HEIGHT,        QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(HEIGHT_ANGLE_PROGRAM_APP_BOOT_FILE_NAME), DRIVER_CAN1,    EXTDEV_TYPE_HIGH},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_MULTI_SOUND,         QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(MULTI_ALARM_PROGRAM_APP_BOOT_FILE_NAME),  DRIVER_CAN1,    EXTDEV_TYPE_ALARM},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_DATA_CONVERTER,      QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(DATA_TRANSFER_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,     EXTDEV_TYPE_DATA_CONVERTER},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_SIGNAL_CONVERTER,    QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(LIGHT_TRANSFER_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,    EXTDEV_TYPE_DATA_CONVERTER},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_LIGHT_ELECTRIL_TERMINAL, QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(LIGHT_TERMINAL_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,EXTDEV_TYPE_DATA_CONVERTER},
        {UPDATE_PRG_PRGTYPE_BOOT,   UPDATE_PRG_DEVTYPE_PHONE,                   QString(BOOT_PROGRAM_DOWN_FILE_PATH),QString(TELEPHONE_PROGRAM_APP_BOOT_FILE_NAME),DRIVER_NET,     EXTDEV_TYPE_DATA_CONVERTER},

        {UPDATE_PRG_PRGTYPE_MAX,    UPDATE_PRG_DEVTYPE_MAX,                 "","",0,0}
    };

    int count = sizeof(temptab)/sizeof (AppUpdateFilePathType);
    for(int i = 0; i < count; i++)
    {
        if((temptab[i].DownPrgType != UPDATE_PRG_PRGTYPE_MAX) \
           || (temptab[i].DownDevType != UPDATE_PRG_DEVTYPE_MAX))
           mUpdateFilePathList.append(temptab[i]);
        else
            break;
    }
    return 0;
}

/*!
 * \brief DevGeneralGetUpdateFilePath
 * \param DownPrgType,更新程序类型
 * \param DownDevType，设备类型，与UI选择的范围一致
 * \param filepath，文件的存储路径
 * \param filename，文件夹的存储名称
 * \param DriverTypeId,所对应的驱动的ID号
 * \param DevTypeId,所对应的设备的类型号
 * \return 0正常-1异常
 */
int DevGeneralGetUpdateFilePath(u_int16_t DownPrgType ,u_int16_t DownDevType, QString &filepath, QString &filename,u_int16_t &DriverTypeId,u_int16_t &DevTypeId)
{
    int ret = -1;
    if((DownPrgType > UPDATE_PRG_PRGTYPE_MAX)||( DownDevType > UPDATE_PRG_DEVTYPE_MAX))
        return -1;

    filepath.clear();
    filename.clear();
    for(int i = 0; i < mUpdateFilePathList.size(); i++)
    {
        if((mUpdateFilePathList.at(i).DownPrgType == DownPrgType) && (mUpdateFilePathList.at(i).DownDevType == DownDevType))
        {
            filepath = mUpdateFilePathList.at(i).DownFilePath;
            filename = mUpdateFilePathList.at(i).DownFileName;
            DriverTypeId = mUpdateFilePathList.at(i).DriverTypeId;
            DevTypeId = mUpdateFilePathList.at(i).DevTypeId;
            ret = 0;
            break;
        }
    }
    return ret;
}

/*!
 * \brief DevGeneralIsUpdateFilePathAvalid,判断文件地址的有效性
 * \param pSrcAdd,源地址
 * \param SrcAddrSize,源地址大小
 * \param pDestAddr,目的地址
 * \param DestAddrSize,目的地址大小
 * \return true正确，false异常
 */
bool DevGeneralIsUpdateFilePathAvalid(char *pSrcAdd,int SrcAddrSize,char *pDestAddr,int DestAddrSize)
{
    if((SrcAddrSize != 300)||(DestAddrSize != 300))
        return false;
    QString SrcStr;
    QString DestStr;
    SrcStr.append(QString::fromLatin1(pSrcAdd,SrcAddrSize));
    DestStr.append(QString::fromLatin1(pDestAddr,DestAddrSize));

    if((DestStr.contains(QString(APP_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(DestStr.contains(QString(BOOT_PROGRAM_DOWN_FILE_PATH)) == false))
        return false;

    if((SrcStr.contains(QString(SS_APP_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(SrcStr.contains(QString(SS_BOOT_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(SrcStr.contains(QString(MD_APP_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(SrcStr.contains(QString(MD_BOOT_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(SrcStr.contains(QString(APP_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(SrcStr.contains(QString(BOOT_PROGRAM_DOWN_FILE_PATH)) == false))
        return false;

    return true;
}

/*!
 * \brief DevGeneralGetUpdateFilePathRelatedInf,获取此目的地址，所对应的设备类型驱动类型等相关信息
 * \param pDestAddr,目的地址
 * \param DestAddrSize,目的地址大小
 * \param DownPrgType,下载程序类型
 * \param DownDevType，下载设备类型
 * \param DriverTypeId，驱动类型
 * \param DevTypeId，所对应的设备的类型号
 * \return
 */
bool  DevGeneralGetUpdateFilePathRelatedInf(char *pDestAddr,int DestAddrSize,\
                                            u_int16_t &DownPrgType,
                                            u_int16_t &DownDevType,
                                            u_int16_t &DriverTypeId,
                                            u_int16_t &DevTypeId)
{
    bool ret = false;
    if(DestAddrSize != 300)
        return false;
    QString DestStr;
    DestStr.append(QString::fromLocal8Bit(pDestAddr));

    if((DestStr.contains(QString(APP_PROGRAM_DOWN_FILE_PATH)) == false)\
        &&(DestStr.contains(QString(BOOT_PROGRAM_DOWN_FILE_PATH)) == false))
        return false;

    for(int i = 0; i < mUpdateFilePathList.size(); i++)
    {
        if(0 == QString::compare(DestStr,mUpdateFilePathList.at(i).DownFilePath+mUpdateFilePathList.at(i).DownFileName))
        {
            ret = true;
            DownPrgType = mUpdateFilePathList.at(i).DownPrgType;
            DownDevType = mUpdateFilePathList.at(i).DownDevType;
            DriverTypeId = mUpdateFilePathList.at(i).DriverTypeId;
            DevTypeId = mUpdateFilePathList.at(i).DevTypeId;
            break;
        }
    }

    return ret;
}

/*******/
/*!
 * \brief DevFrameDriverOriginValueReportGeneralProc,上报状态数据的一般性处理。默认为是传感器类数据上报
 * \param driverid
 * \param devid
 * \param childdevid
 * \param point
 * \param data
 * \param len
 * \return
 */
int DevFrameDriverOriginValueReportGeneralProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
//    qDebug("++++general++++DevFrameDriverOriginValueReportGeneralProc,driverid is %d,devid is %d ,childdevid is %d,point is %d ,len id %d",driverid,devid,childdevid,point,len);

    if(len < 0x02)
        return -1;
    if(point == 0)
    {
        for(u_int16_t i = 0; i < len;i++,i++)
        {
            SensorInsertStateApi(driverid,devid,childdevid,i/2+1,*(u_int16_t *)(data+i));
        }
    }
    else
    {
        SensorInsertStateApi(driverid,devid,childdevid,point, *(u_int16_t *)data);
    }
    return 0;
}

/*!
 * \brief DevFrameDriverExtdevStatusReportProc,上报设备状态的处理
 * \param driverid
 * \param devid
 * \param childdevid
 * \param point
 * \param data
 * \param len
 * \return
 */
int DevFrameDriverExtdevStatusReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    if(len != sizeof (DriverExtDevStatusType))
        return -1;
//    qDebug("++++general++++DevFrameDriverExtdevStatusReportProc,driverid is %d,devid is %d ,childdevid is %d,point is %d ,len id %d",driverid,devid,childdevid,point,len);
    HysSystem *pSystem = HysSystem::getInston();
    pSystem->SetDriverExtDevState(driverid,devid,childdevid,*(DriverExtDevStatusType *)data);
    return 0;
}

/******/
int DevGeneralInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFrameDriverOriginValueReportGeneralProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);

    /******/
    DevUpdateFilePathInit();
    return 0;
}

int DevGeneralFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

