#include "can_device_app.h"
#include "zprint.h"
#include <mutex>
#include "ydshrdlib.h"
#include "dyk_v2pro.h"

std::mutex emvd_mutex;
std::mutex cxb_mutex;
std::mutex angle_mutex;
std::mutex alarmAngle_mutex;
std::mutex highAngle_mutex;
std::mutex person_mutex;
std::mutex alarm_mutex;
std::mutex ir_mutex;
std::mutex wireless_mutex;
std::mutex high_mutex;
std::mutex miniHub_mutex;



DeviceInfo* DeviceInfo::deviceInfo = nullptr; // 初始化静态成员变量

DeviceInfo& DeviceInfo::getInstance()
{
    if (!deviceInfo)
    {
        deviceInfo = new DeviceInfo();
    }
    return *deviceInfo;
}

void DeviceInfo::insert(uint32_t key, EmvdMsgInfo value)
{
     std::lock_guard<std::mutex> lock(emvd_mutex);
     emvdMap[key] = value;
}

int DeviceInfo::find_emvd_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(emvd_mutex);
    if(emvdMap.find(key) != emvdMap.end())
        return 0;
    return -1;
}

void DeviceInfo::insert(uint32_t key, CxbMsgInfo value)
{
     std::lock_guard<std::mutex> lock(cxb_mutex);
     cxbMap[key] = value;
}

int DeviceInfo::find_cxb_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(cxb_mutex);
    if(cxbMap.find(key) != cxbMap.end())
        return 0;
    return -1;
}


void DeviceInfo::insert_angle(uint32_t key, AngleMsgInfo value)
{
    std::lock_guard<std::mutex> lock(angle_mutex);
    angleMap[key] = value;
}

void DeviceInfo::insert_alarmAngle(uint32_t key, AngleMsgInfo value)
{
    std::lock_guard<std::mutex> lock(alarmAngle_mutex);
    alarmAngleMap[key] = value;
}
void DeviceInfo::insert_highAngle(uint32_t key, AngleMsgInfo value)
{
    std::lock_guard<std::mutex> lock(highAngle_mutex);
    highAngleMap[key] = value;
}


int DeviceInfo::find_angle_value(uint8_t device, uint32_t key)
{
    std::lock_guard<std::mutex> lock(angle_mutex);
    switch (device) {
        case ID_ANGLE:
            if(angleMap.find(key) != angleMap.end())
                return 0;
            return -1;
            break;
        case ID_ALARMANGLE:
            if(alarmAngleMap.find(key) != alarmAngleMap.end())
                return 0;
            return -1;
            break;
        case ID_HIGHANGLE:
            if(highAngleMap.find(key) != highAngleMap.end())
                return 0;
            return -1;
            break;
        default:
            break;
    }
    return 0;
}

void DeviceInfo::insert(uint32_t key, PersonMsgInfo value)
{
    std::lock_guard<std::mutex> lock(person_mutex);
    personMap[key] = value;
}

int DeviceInfo::find_person_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(person_mutex);
    if(personMap.find(key) != personMap.end())
        return 0;
    return -1;
}


void DeviceInfo::insert(uint32_t key, AlarmMsgInfo value)
{
    std::lock_guard<std::mutex> lock(alarm_mutex);
    alarmMap[key] = value;
}

int DeviceInfo::find_alarm_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(alarm_mutex);
    if(alarmMap.find(key) != alarmMap.end())
        return 0;
    return -1;
}


void DeviceInfo::insert(uint32_t key, IRMsgInfo value)
{
    std::lock_guard<std::mutex> lock(ir_mutex);
    irMap[key] = value;
}


int DeviceInfo::find_ir_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(ir_mutex);
    if(irMap.find(key) != irMap.end())
        return 0;
    return -1;
}



void DeviceInfo::insert(uint32_t key, WirelessMsgInfo value)
{
    std::lock_guard<std::mutex> lock(wireless_mutex);
    wirelessMap[key] = value;
}


int DeviceInfo::find_wireless_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(wireless_mutex);
    if(wirelessMap.find(key) != wirelessMap.end())
        return 0;
    return -1;
}



void DeviceInfo::insert(uint32_t key, HighMsgInfo value)
{
    std::lock_guard<std::mutex> lock(high_mutex);
    highMap[key] = value;
}


int DeviceInfo::find_high_value(uint32_t key)
{
    std::lock_guard<std::mutex> lock(high_mutex);
    if(highMap.find(key) != highMap.end())
        return 0;
    return -1;
}

void DeviceInfo::insert(uint8_t device, uint32_t key, MiniHubMsgInfo value)
{
    std::lock_guard<std::mutex> lock(miniHub_mutex);
    switch(device)
    {
        case ID_MINIHUB1:
            mini1Map[key] = value;
            break;
        case ID_MINIHUB2:
            mini2Map[key] = value;
            break;
        case ID_MINIHUB3:
            mini3Map[key] = value;
            break;
        default:
            break;
    }
}


int DeviceInfo::find_minihub_value(uint8_t device, uint32_t key)
{
    std::lock_guard<std::mutex> lock(miniHub_mutex);
    switch (device) {
        case ID_MINIHUB1:
            if(mini1Map.find(key) != mini1Map.end())
                return 0;
            break;
        case ID_MINIHUB2:
            if(mini2Map.find(key) != mini2Map.end())
                return 0;
            break;
        case ID_MINIHUB3:
            if(mini3Map.find(key) != mini3Map.end())
                return 0;
            break;
        default:
            break;
    }
    return -1;
}






int DeviceInfo:: setDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,st_DeviceStatusMsg * deviceMsg){

    uint32_t deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    uint16_t reportTimer;
    EmvdMsgInfo emvdMsgInfo;
    AngleMsgInfo angleMsgInfo;
    IRMsgInfo irMsgInfo;
    PersonMsgInfo personMsgInfo;
    AlarmMsgInfo alarmMsgInfo;
    HighMsgInfo highMsgInfo;
    MiniHubMsgInfo minihubMsgInfo;
    CxbMsgInfo cxbMsgInfo;
    WirelessMsgInfo wirelessMsgInfo;

    switch(device)
    {
        case ID_EMVD:
            if(find_emvd_value(deviceTmp) == 0)
            {
                emvdMsgInfo = emvdMap[deviceTmp]; 
                if(emvdMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = emvdMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&emvdMsgInfo.deviceStatusMsg, deviceMsg, sizeof(st_DeviceStatusMsg));
                emvdMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(deviceTmp,emvdMsgInfo);
            }
            else
            {
                memset(&emvdMsgInfo,0,sizeof(EmvdMsgInfo));
                memcpy(&emvdMsgInfo.deviceStatusMsg ,deviceMsg, sizeof(st_DeviceStatusMsg));
                emvdMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(deviceTmp,emvdMsgInfo);
            }
            break;
        case ID_CXB:
            if(find_emvd_value(deviceTmp) == 0)
            {
                cxbMsgInfo = cxbMap[deviceTmp];
                if(cxbMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = cxbMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&cxbMsgInfo.deviceStatusMsg, deviceMsg, sizeof(st_DeviceStatusMsg));
                cxbMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(deviceTmp,cxbMsgInfo);
            }
            else
            {
                memset(&cxbMsgInfo,0,sizeof(CxbMsgInfo));
                memcpy(&cxbMsgInfo.deviceStatusMsg ,deviceMsg, sizeof(st_DeviceStatusMsg));
                cxbMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(deviceTmp,cxbMsgInfo);
            }
            break;
        case ID_ANGLE:
            if(find_angle_value(ID_ANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = angleMap[deviceTmp];
                if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = angleMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&angleMsgInfo.deviceStatusMsg, deviceMsg, sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert_angle(deviceTmp,angleMsgInfo);
            }
            else
            {
                memset(&angleMsgInfo,0,sizeof(AngleMsgInfo));
                memcpy(&angleMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert_angle(deviceTmp,angleMsgInfo);
            }
            break;
        case ID_ALARMANGLE:
            if(find_angle_value(ID_ALARMANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = alarmAngleMap[deviceTmp];
                if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = angleMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&angleMsgInfo.deviceStatusMsg, deviceMsg, sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert_alarmAngle(deviceTmp,angleMsgInfo);
            }
            else
            {
                memset(&angleMsgInfo,0,sizeof(AngleMsgInfo));
                memcpy(&angleMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert_alarmAngle(deviceTmp,angleMsgInfo);
            }
            break;
        case ID_HIGHANGLE:
            if(find_angle_value(ID_HIGHANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = highAngleMap[deviceTmp];
                if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = angleMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&angleMsgInfo.deviceStatusMsg, deviceMsg, sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert_highAngle(deviceTmp,angleMsgInfo);
            }
            else
            {
                memset(&angleMsgInfo,0,sizeof(AngleMsgInfo));
                memcpy(&angleMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert_highAngle(deviceTmp,angleMsgInfo);
            }
            break;
        case ID_HIGH:
            if(find_high_value(deviceTmp) == 0)
            {
                highMsgInfo = highMap[deviceTmp];
                if(highMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = highMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&highMsgInfo.deviceStatusMsg, deviceMsg, sizeof(st_DeviceStatusMsg));
                highMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(deviceTmp,highMsgInfo);
            }
            else
            {
                memset(&highMsgInfo,0,sizeof(HighMsgInfo));
                memcpy(&highMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(deviceTmp,highMsgInfo);
            }
            break;
        case ID_ALARM:
            if(find_alarm_value(deviceTmp) == 0)
            {
                alarmMsgInfo = alarmMap[deviceTmp];
                if(alarmMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = alarmMsgInfo.deviceStatusMsg.ReportTimer;

                if(alarmMsgInfo.deviceStatusMsg.deviceVersion[0] == 0 && alarmMsgInfo.deviceStatusMsg.deviceVersion[1] == 0
                        && (deviceMsg->deviceVersion[0] != 0 || deviceMsg->deviceVersion[1] != 0))
                   deviceMsg->ReportState = STATE_MODE_REPORT;
                memcpy(&alarmMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                alarmMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(deviceTmp,alarmMsgInfo);

//                zprintf3("alarm device report state verson deviceMsg%d,%d,%d\n",
//                        deviceMsg->deviceVersion[0],
//                        deviceMsg->deviceVersion[1],
//                        deviceMsg->deviceVersion[2],
//                        deviceMsg->deviceVersion[3]);


//                zprintf3("alarm device report state verson alarmMsgInfo%d,%d,%d\n",
//                        alarmMsgInfo.deviceStatusMsg.deviceVersion[0],
//                        alarmMsgInfo.deviceStatusMsg.deviceVersion[1],
//                        alarmMsgInfo.deviceStatusMsg.deviceVersion[2],
//                        alarmMsgInfo.deviceStatusMsg.deviceVersion[3]);
            }
            else
            {
                memset(&alarmMsgInfo,0,sizeof(AlarmMsgInfo));
                memcpy(&alarmMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                alarmMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(deviceTmp,alarmMsgInfo);
            }
            break;
        case ID_PERSON:
            if(find_person_value(deviceTmp) == 0)
            {
                personMsgInfo = personMap[deviceTmp];
                if(personMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                if(personMsgInfo.deviceStatusMsg.deviceVersion[0] == 0 && personMsgInfo.deviceStatusMsg.deviceVersion[1] == 0
                        && (deviceMsg->deviceVersion[0] != 0 || deviceMsg->deviceVersion[1] != 0))
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = personMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&personMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                personMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(deviceTmp,personMsgInfo);
            }
            else
            {
                memset(&personMsgInfo,0,sizeof(PersonMsgInfo));
                memcpy(&personMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                personMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(deviceTmp,personMsgInfo);
            }
            break;
        case ID_IRRX:
            if(find_ir_value(deviceTmp) == 0)
            {
                irMsgInfo = irMap[deviceTmp];
                if(irMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = irMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&irMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                irMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(deviceTmp,irMsgInfo);
            }
            else
            {
                memset(&irMsgInfo,0,sizeof(IRMsgInfo));
                memcpy(&irMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                irMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(deviceTmp,irMsgInfo);
            }
            break;

        case ID_WIRELESS:
        if(find_wireless_value(deviceTmp) == 0)
        {
            wirelessMsgInfo = wirelessMap[deviceTmp];
            if(wirelessMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                deviceMsg->ReportState = STATE_MODE_REPORT;
            reportTimer = wirelessMsgInfo.deviceStatusMsg.ReportTimer;
            memcpy(&wirelessMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
            wirelessMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
            insert(deviceTmp,wirelessMsgInfo);
        }
        else
        {
            memset(&wirelessMsgInfo,0,sizeof(IRMsgInfo));
            memcpy(&wirelessMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
            wirelessMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            insert(deviceTmp,wirelessMsgInfo);
        }
        break;
        case ID_MINIHUB1:
            if(find_minihub_value(ID_MINIHUB1,deviceTmp) == 0)
            {
                minihubMsgInfo = mini1Map[deviceTmp];
                if(minihubMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = minihubMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&minihubMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                minihubMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(ID_MINIHUB1,deviceTmp,minihubMsgInfo);
            }
            else
            {
                memset(&minihubMsgInfo,0,sizeof(MiniHubMsgInfo));
                memcpy(&minihubMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                minihubMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(ID_MINIHUB1,deviceTmp,minihubMsgInfo);
            }
            break;
        case ID_MINIHUB2:
            if(find_minihub_value(ID_MINIHUB2,deviceTmp) == 0)
            {
                minihubMsgInfo = mini2Map[deviceTmp];
                if(minihubMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = minihubMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&minihubMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                minihubMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(ID_MINIHUB2,deviceTmp,minihubMsgInfo);
            }
            else
            {
                memset(&minihubMsgInfo,0,sizeof(MiniHubMsgInfo));
                memcpy(&minihubMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                minihubMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(ID_MINIHUB2,deviceTmp,minihubMsgInfo);
            }
            break;
        case ID_MINIHUB3:
            if(find_minihub_value(ID_MINIHUB3,deviceTmp) == 0)
            {
                minihubMsgInfo = mini3Map[deviceTmp];
                if(minihubMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    deviceMsg->ReportState = STATE_MODE_REPORT;
                reportTimer = minihubMsgInfo.deviceStatusMsg.ReportTimer;
                memcpy(&minihubMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                minihubMsgInfo.deviceStatusMsg.ReportTimer = reportTimer;
                insert(ID_MINIHUB3,deviceTmp,minihubMsgInfo);
            }
            else
            {
                memset(&minihubMsgInfo,0,sizeof(MiniHubMsgInfo));
                memcpy(&minihubMsgInfo.deviceStatusMsg ,deviceMsg,sizeof(st_DeviceStatusMsg));
                minihubMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
                insert(ID_MINIHUB3,deviceTmp,minihubMsgInfo);
            }
            break;
        default:
            break;

    }
    return 0;
}


int DeviceInfo::setDeviceStateProc_MiniHub(uint8_t MiniHubIndex,uint32_t deviceIndex,uint8_t subDevice,st_DeviceStatusMsg* deviceMsg)
{
    MiniHubMsgInfo minihubMsgInfo;
    switch (MiniHubIndex) {
        case ID_MINIHUB1:
            if(find_minihub_value(ID_MINIHUB1,deviceIndex) == 0)
            {
                minihubMsgInfo = mini1Map[deviceIndex];
                memcpy(&minihubMsgInfo.AngleMsg[subDevice].deviceStatusMsg,deviceMsg,sizeof(st_DeviceStatusMsg));
                insert(ID_MINIHUB1,deviceIndex,minihubMsgInfo);
            }
            break;
        case ID_MINIHUB2:
            if(find_minihub_value(ID_MINIHUB2,deviceIndex) == 0)
            {
                minihubMsgInfo = mini2Map[deviceIndex];
                memcpy(&minihubMsgInfo.AngleMsg[subDevice].deviceStatusMsg,deviceMsg,sizeof(st_DeviceStatusMsg));
                insert(ID_MINIHUB2,deviceIndex,minihubMsgInfo);
            }
            break;
        case ID_MINIHUB3:
            if(find_minihub_value(ID_MINIHUB3,deviceIndex) == 0)
            {
                minihubMsgInfo = mini3Map[deviceIndex];
                memcpy(&minihubMsgInfo.AngleMsg[subDevice].deviceStatusMsg,deviceMsg,sizeof(st_DeviceStatusMsg));
                insert(ID_MINIHUB3,deviceIndex,minihubMsgInfo);
            }
            break;
        default:
            break;
    }
    return 1;
}


int DeviceInfo::getDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,st_DeviceStatusMsg* deviceMsg)
{
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    EmvdMsgInfo emvdMsgInfo;
    AngleMsgInfo angleMsgInfo;
    AlarmMsgInfo alarmMsgInfo;
    PersonMsgInfo personMsgInfo;
    switch(device)
    {
        case ID_EMVD:
            memset(&emvdMsgInfo,0,sizeof(EmvdMsgInfo));
            if(find_emvd_value(deviceTmp) == 0)
            {
                emvdMsgInfo = emvdMap[deviceTmp];
//                if(emvdMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
//                    return -1;
                *deviceMsg = emvdMsgInfo.deviceStatusMsg;
            }
            else
            {
                return -1;
            }
            break;
        case ID_ANGLE:
            memset(&angleMsgInfo,0,sizeof(AngleMsgInfo));
            if(find_angle_value(ID_ANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = angleMap[deviceTmp];
                if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    return -1;
                *deviceMsg = angleMsgInfo.deviceStatusMsg;
            }
            else
            {
                return -1;
            }
            break;
        case ID_ALARM:
            memset(&alarmMsgInfo,0,sizeof(AlarmMsgInfo));
            if(find_alarm_value(deviceTmp) == 0)
            {
                alarmMsgInfo = alarmMap[deviceTmp];
//                if(alarmMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
//                    return -1;
                *deviceMsg = alarmMsgInfo.deviceStatusMsg;
            }
            else
            {
                return -1;
            }
            break;

        case ID_PERSON:
            memset(&personMsgInfo,0,sizeof(PersonMsgInfo));
            if(find_person_value(deviceTmp) == 0)
            {
                personMsgInfo = personMap[deviceTmp];
    //                if(alarmMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
    //                    return -1;
                *deviceMsg = personMsgInfo.deviceStatusMsg;
            }
            else
            {
                return -1;
            }
            break;

    }
    return 0;
}

int getDeviceStateMidProc(st_DeviceStatusMsg deviceMsg,DeviceStateType deviceType, uint16_t* value)
{
    switch (deviceType) {
            case STATE_ISONLINE:
                *value = deviceMsg.isOnline;
                break;
            case STATE_HEARTTIMER:
                *value = deviceMsg.HeartTimer;
                break;
            case STATE_REPORTTIMER:
                *value = deviceMsg.ReportTimer;
                break;
            case STATE_DEVICESTATUS:
                *value = deviceMsg.deviceStatus;
                break;
            default:
                return -1; // 未知的设备状态类型
        }
        return 0; // 获取成功
}


int DeviceInfo::getDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,DeviceStateType deviceType, uint16_t* value)
{
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    EmvdMsgInfo emvdMsgInfo;
    AngleMsgInfo angleMsgInfo;
    AlarmMsgInfo alarmMsgInfo;
    CxbMsgInfo cxbMsgInfo;
    switch(device)
    {
        case ID_EMVD:

            if(find_emvd_value(deviceTmp) == 0)
            {
                emvdMsgInfo = emvdMap[deviceTmp];
                getDeviceStateMidProc(emvdMsgInfo.deviceStatusMsg,deviceType,value);
            }
            else
                return -1;
            break;
        case ID_CXB:

            if(find_cxb_value(deviceTmp) == 0)
            {
                cxbMsgInfo = cxbMap[deviceTmp];
                getDeviceStateMidProc(cxbMsgInfo.deviceStatusMsg,deviceType,value);
            }
            else
                return -1;
            break;
        case ID_ANGLE:
            if(find_angle_value(ID_ANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = angleMap[deviceTmp];
                getDeviceStateMidProc(angleMsgInfo.deviceStatusMsg,deviceType,value);
            }
            else
            {
                return -1;
            }
            break;
        case ID_ALARM:
            if(find_alarm_value(deviceTmp) == 0)
            {
                alarmMsgInfo = alarmMap[deviceTmp];
                getDeviceStateMidProc(alarmMsgInfo.deviceStatusMsg,deviceType,value);
            }
            else
            {
                return -1;
            }
            break;

    }
    return 0;
}


int setDeviceStateMidProc(st_DeviceStatusMsg deviceMsg,DeviceStateType deviceType, uint16_t value)
{
    switch (deviceType) {
            case STATE_ISONLINE:
                deviceMsg.isOnline = value;
                break;
            case STATE_HEARTTIMER:
                deviceMsg.HeartTimer = value;
                break;
            case STATE_REPORTTIMER:
                deviceMsg.ReportTimer = value;
                break;
            case STATE_DEVICESTATUS:
                deviceMsg.deviceStatus = value;
                break;
            default:
                return -1; // 未知的设备状态类型
        }
        return 0; // 获取成功
}


int DeviceInfo::setDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,DeviceStateType deviceType, uint16_t value)
{
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    EmvdMsgInfo emvdMsgInfo;
    AngleMsgInfo angleMsgInfo;
    switch(device)
    {
        case ID_EMVD:
            if(find_emvd_value(deviceTmp) == 0)
            {
                emvdMsgInfo = emvdMap[deviceTmp];
                setDeviceStateMidProc(emvdMsgInfo.deviceStatusMsg,deviceType,value);
                insert(deviceTmp,emvdMsgInfo);
            }
            else
                return -1;
            break;
        case ID_ANGLE:
            if(find_angle_value(ID_ANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = angleMap[deviceTmp];
                setDeviceStateMidProc(angleMsgInfo.deviceStatusMsg,deviceType,value);
                insert_angle(deviceTmp,angleMsgInfo);
            }
            else
            {
                return -1;
            }
            break;

    }
    return 0;
}

int DeviceInfo::getDeviceAppProc_EMVD(uint8_t driver,uint8_t device,uint8_t subDevice,EMVD_APP type,uint32_t *value){

    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    EmvdMsgInfo emvdMsgInfo;
    if(find_emvd_value(deviceTmp) == 0)
    {
        emvdMsgInfo = emvdMap[deviceTmp];
    }
    else
        return -1;
    switch(type)
    {
        case APP_EMVD_IO:
            *value = emvdMsgInfo.iovalue;
            break;
        case APP_EMVD_STATE:
            *value = emvdMsgInfo.state;
            break;
        case APP_EMVD_CHECKIO:
            *value = emvdMsgInfo.checkio;
            break;
        case APP_EMVD_FLAG:
            *value = emvdMsgInfo.flag;
            break;
        default:
            break;
    }
    return 0;
}



int DeviceInfo::setDeviceAppProc_EMVD(uint8_t driver,uint8_t device,uint8_t subDevice,EMVD_APP type,uint16_t point,uint8_t state){
    uint32_t deviceTmp = 0;
    uint32_t value = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    EmvdMsgInfo emvdMsgInfo;
    if(find_emvd_value(deviceTmp) == 0)
    {
        emvdMsgInfo = emvdMap[deviceTmp];
    }
    else
        return -1;
    switch(type)
    {
        case APP_EMVD_IO:
            if(state == EMVD_OPEN)
            {
                value = (1<<point);
                emvdMsgInfo.iovalue |= value;
            }
            else if(state == EMVD_CLOSE)
            {
                if(point == 0)
                {
                    emvdMsgInfo.iovalue = 0;
                }
                else
                {
                    uint32_t value = 1<<point;
                    if((emvdMsgInfo.iovalue & value) == value)
                        emvdMsgInfo.iovalue -= value;
                }
            }
            break;
        case APP_EMVD_STATE:
            emvdMsgInfo.state = point;
            break;
        case APP_EMVD_CHECKIO:
            emvdMsgInfo.checkio = point;
            break;
        case APP_EMVD_FLAG:
            emvdMsgInfo.flag = point;
            break;
        case APP_EMVD_ACTIONTIMER:
            emvdMsgInfo.actionTimer = point;
            break;
        default:
            break;
    }
    insert(deviceTmp,emvdMsgInfo);
    return 0;
}


int DeviceInfo::setDeviceAppProc_MINIHUB(uint8_t MiniHubIndex, uint32_t DeviceIndex, uint8_t point, uint8_t *data, uint8_t len){

    MiniHubMsgInfo msgInfo;

    switch(MiniHubIndex)
    {
        case ID_MINIHUB1:
            if(find_minihub_value(ID_MINIHUB1, DeviceIndex) == 0)
                msgInfo = mini1Map[DeviceIndex];
            else
                return -1;
            break;
        case ID_MINIHUB2:
            if(find_minihub_value(ID_MINIHUB2, DeviceIndex) == 0)
                msgInfo = mini2Map[DeviceIndex];
            else
                return -1;
            break;
        case ID_MINIHUB3:
            if(find_minihub_value(ID_MINIHUB3, DeviceIndex) == 0)
                msgInfo = mini3Map[DeviceIndex];
            else
                return -1;
            break;
        default:
            return -1;
            break;
    }
    switch(point)
    {
        case MINIHUB_ANGLOG1:
            msgInfo.deviceStatusMsg.HeartTimer = HUB_HEART_TIMER;
            memcpy(&msgInfo.analog1,data,len);
            break;
        case MINIHUB_ANGLOG2:
            memcpy(&msgInfo.analog2,data,len);
            break;
        case MINIHUB_ANGLOG3:
            memcpy(&msgInfo.analog3,data,len);
            break;
        case MINIHUB_ANGLOG4:
            memcpy(&msgInfo.analog4,data,len);
            break;
        case MINIHUB_X1:
            msgInfo.deviceStatusMsg.HeartTimer = HUB_HEART_TIMER;
            memcpy(&msgInfo.AngleMsg[0].angle_x,data,len);
            break;
        case MINIHUB_Y1:
            memcpy(&msgInfo.AngleMsg[0].angle_y,data,len);
            break;
        case MINIHUB_Z1:
            msgInfo.deviceStatusMsg.HeartTimer = HUB_HEART_TIMER;
            memcpy(&msgInfo.AngleMsg[0].angle_z,data,len);
            break;
        case MINIHUB_X2:
            memcpy(&msgInfo.AngleMsg[1].angle_x,data,len);
            break;
        case MINIHUB_Y2:
            memcpy(&msgInfo.AngleMsg[1].angle_y,data,len);
            break;
        case MINIHUB_Z2:
            memcpy(&msgInfo.AngleMsg[1].angle_z,data,len);
            break;
        case MINIHUB_X3:
            msgInfo.deviceStatusMsg.HeartTimer = HUB_HEART_TIMER;
            memcpy(&msgInfo.AngleMsg[2].angle_x,data,len);
            break;
        case MINIHUB_Y3:
            memcpy(&msgInfo.AngleMsg[2].angle_y,data,len);
            break;
        case MINIHUB_Z3:
            memcpy(&msgInfo.AngleMsg[2].angle_z,data,len);
            break;
        case MINIHUB_X4:
            memcpy(&msgInfo.AngleMsg[3].angle_x,data,len);
            break;
        case MINIHUB_Y4:
            memcpy(&msgInfo.AngleMsg[3].angle_y,data,len);
            break;
        case MINIHUB_Z4:
            memcpy(&msgInfo.AngleMsg[3].angle_z,data,len);
            break;
    }

    insert(MiniHubIndex,DeviceIndex,msgInfo);
    return 0;
}

int DeviceInfo::setDeviceAppProc_WL(uint8_t driver,uint8_t device,uint8_t subDevice,uint8_t type){
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    IRMsgInfo irMsgInfo;
    if(find_ir_value(deviceTmp) == 0)
    {
        irMsgInfo = irMap[deviceTmp];
    }
    else
        return -1;

    if(type == 0)
    {
        irMsgInfo.irCounts = 0;
    }
    else
    {
        irMsgInfo.irCounts++;
        irMsgInfo.irCounts%= 0xFFFF;
        irMsgInfo.deviceStatusMsg.HeartTimer = WL_HEART_TIMER;
    }
    insert(deviceTmp,irMsgInfo);

    return 0;
}


int DeviceInfo::setDeviceAppProc_Wireless(uint8_t driver,uint8_t device,uint8_t subDevice,uint8_t type,uint32_t data){
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    WirelessMsgInfo msgInfo;
    if(find_wireless_value(deviceTmp) == 0)
    {
        msgInfo = wirelessMap[deviceTmp];
    }
    else
        return -1;

    switch (type) {
    case WIRELESS_YKQNUM:
        msgInfo.ykqNum = data;
        break;
    case WIRELESS_CODESTATE:
        msgInfo.codeState = data;
        break;
    case WIRELESS_RSSI:
        msgInfo.wirelessRSSI = data;
        break;
    case WIRELESS_ACTIONTYPE:
        msgInfo.actionType = data;
        break;
    case WIRELESS_ACTIONID:
        msgInfo.actionID = data;
        break;
    default:
        break;
    }
    msgInfo.deviceStatusMsg.HeartTimer = ANGLE_HEART_TIMER;
    insert(deviceTmp,msgInfo);
    return 0;
}


int DeviceInfo::getDeviceAppProc_Wireless(uint8_t driver,uint8_t device,uint8_t subDevice,uint8_t type,uint32_t * data){
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    WirelessMsgInfo msgInfo;
    if(find_wireless_value(deviceTmp) == 0)
    {
        msgInfo = wirelessMap[deviceTmp];
    }
    else
        return -1;

    switch (type) {
    case WIRELESS_YKQNUM:
        *data = msgInfo.ykqNum ;
        break;
    case WIRELESS_CODESTATE:
        *data = msgInfo.codeState;
        break;
    case WIRELESS_RSSI:
        *data = msgInfo.wirelessRSSI;
        break;
    case WIRELESS_ACTIONTYPE:
        *data = msgInfo.actionType;
        break;
    case WIRELESS_ACTIONID:
        *data = msgInfo.actionID;
        break;
    default:
        break;
    }
    return 0;
}


int DeviceInfo::setDeviceAppProc_ANGLE(uint8_t driver, uint8_t device, uint8_t subDevice, ANGLE_APP type, uint32_t value){
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    AngleMsgInfo angleMsgInfo;
    memset(&angleMsgInfo,0,sizeof(AngleMsgInfo));
    switch (device) {
        case ID_ANGLE:
            if(find_angle_value(ID_ANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = angleMap[deviceTmp];
                switch (type) {
                    case ANGLE_X:
                        angleMsgInfo.deviceStatusMsg.HeartTimer = ANGLE_HEART_TIMER;
                        angleMsgInfo.angle_x = value;
                        insert_angle(deviceTmp,angleMsgInfo);
                        break;
                    case ANGLE_Y:
                        angleMsgInfo.angle_y = value;
                        insert_angle(deviceTmp,angleMsgInfo);
                        break;
                    case ANGLE_Z:
                        angleMsgInfo.angle_z = value;
                        insert_angle(deviceTmp,angleMsgInfo);
                        break;
                    default:
                        break;
                }
            }
            else
                return -1;
            break;
        case ID_ALARMANGLE:
            if(find_angle_value(ID_ALARMANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = alarmAngleMap[deviceTmp];
                switch (type) {
                    case ANGLE_X:
                        angleMsgInfo.deviceStatusMsg.HeartTimer = ANGLE_HEART_TIMER;
                        angleMsgInfo.angle_x = value;
                        insert_alarmAngle(deviceTmp,angleMsgInfo);
                        break;
                    case ANGLE_Y:
                        angleMsgInfo.angle_y = value;
                        insert_alarmAngle(deviceTmp,angleMsgInfo);
                        break;
                    case ANGLE_Z:
                        angleMsgInfo.angle_z = value;
                        insert_alarmAngle(deviceTmp,angleMsgInfo);
                        break;
                    default:
                        break;
                }
            }
            else
                return -1;
            break;
        case ID_HIGHANGLE:
            if(find_angle_value(ID_HIGHANGLE,deviceTmp) == 0)
            {
                angleMsgInfo = highAngleMap[deviceTmp];
                switch (type) {
                    case ANGLE_X:
                        angleMsgInfo.deviceStatusMsg.HeartTimer = ANGLE_HEART_TIMER;
                        angleMsgInfo.angle_x = value;
                        insert_highAngle(deviceTmp,angleMsgInfo);
                        break;
                    case ANGLE_Y:
                        angleMsgInfo.angle_y = value;
                        insert_highAngle(deviceTmp,angleMsgInfo);
                        break;
                    case ANGLE_Z:
                        angleMsgInfo.angle_z = value;
                        insert_highAngle(deviceTmp,angleMsgInfo);
                        break;
                    default:
                        break;
                }
            }
            else
                return -1;
            break;
    default:
        break;
    }
    return 0;

}

int DeviceInfo::setDeviceAppProc_HIGH(uint8_t driver, uint8_t device, uint8_t subDevice, uint32_t value){
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    HighMsgInfo highMsgInfo;

    if(find_high_value(deviceTmp) == 0)
    {
        memset(&highMsgInfo,0,sizeof(HighMsgInfo));
        highMsgInfo = highMap[deviceTmp];
        highMsgInfo.deviceStatusMsg.HeartTimer = ANGLE_HEART_TIMER;
        highMsgInfo.high = value;
        insert(deviceTmp,highMsgInfo);
    }
    else
        return -1;
    return 0;
}




int DeviceInfo::getDeviceAppProc_ANGLE(uint8_t driver, uint8_t device, uint8_t subDevice, ANGLE_APP type, uint32_t * value){
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);
    AngleMsgInfo angleMsgInfo;
    memset(&angleMsgInfo,0,sizeof(AngleMsgInfo));
    switch (device) {
    case ID_ANGLE:
        if(find_angle_value(ID_ANGLE,deviceTmp) == 0)
        {
            angleMsgInfo = angleMap[deviceTmp];
            switch (type) {
                case ANGLE_X:
                    *value = angleMsgInfo.angle_x;
                    break;
                case ANGLE_Y:
                    *value = angleMsgInfo.angle_y;
                    break;
                case ANGLE_Z:
                    *value = angleMsgInfo.angle_z;
                    break;
                default:
                    break;
            }
        }
        else
            return -1;

        break;
    default:
        break;
    }
    return 0;

}

int DeviceInfo::setDeviceAppProc_PERSON(uint8_t driver, uint8_t device, uint8_t subDevice, uint16_t type, uint16_t id, PersonAppMsg* perData)
{
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);

    PersonMsgInfo personMsgInfo;
    PersonAppMsg personAppMsg;

    memset(&personMsgInfo,0,sizeof(PersonMsgInfo));
    memset(&personAppMsg,0,sizeof(PersonAppMsg));

    if(find_person_value(deviceTmp) == 0)
    {
        personMsgInfo = personMap[deviceTmp];
        personMsgInfo.deviceStatusMsg.HeartTimer = ALARM_HEART_TMIER;
        insert(deviceTmp,personMsgInfo);
    }
    else
        return -1;

    return 0;
}

int DeviceInfo::setDeviceAppProc_ALARM(uint8_t driver, uint8_t device, uint8_t subDevice, uint16_t point, uint8_t workMode)
{
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);

    AlarmMsgInfo alarmMsgInfo;
    memset(&alarmMsgInfo,0,sizeof(AlarmMsgInfo));
    if(find_alarm_value(deviceTmp) == 0)
        alarmMsgInfo = alarmMap[deviceTmp];

    switch(point)
    {
        case ALARMMODE:
            alarmMsgInfo.alarmMode = workMode;
        break;
        case PEERMODE:
            alarmMsgInfo.alarmMode = workMode;
        break;
        default:
            break;

    }
    insert(deviceTmp,alarmMsgInfo);
    return 0;
}


int DeviceInfo::getDeviceAppProc_ALARM(uint8_t driver, uint8_t device, uint8_t subDevice, uint16_t point, uint8_t * workMode)
{
    uint32_t deviceTmp = 0;
    deviceTmp = (driver<<16)|(device<<8)|(subDevice);

    AlarmMsgInfo alarmMsgInfo;
    memset(&alarmMsgInfo,0,sizeof(AlarmMsgInfo));
    if(find_alarm_value(deviceTmp) == 0)
        alarmMsgInfo = alarmMap[deviceTmp];
//    else
//        return -1;

    switch(point)
    {
        case ALARMMODE:
            *workMode = alarmMsgInfo.alarmMode;
        break;
        case PEERMODE:
            *workMode = alarmMsgInfo.peerMode;
        break;
        default:
            break;

    }
    return 0;
}


void getReportDeviceData(uint8_t* reportdata,st_DeviceStatusMsg deviceStatusMsg)
{
    reportdata[0] = deviceStatusMsg.isOnline;
    reportdata[1] = 0;
    reportdata[2] = 0;
    reportdata[3] = 0;
    reportdata[4] = (deviceStatusMsg.deviceVersion[0]&0xFF);
    reportdata[5] = (deviceStatusMsg.deviceVersion[3]&0xFF);
    reportdata[6] = (deviceStatusMsg.deviceVersion[2]&0xFF);
    reportdata[7] = (deviceStatusMsg.deviceVersion[1]&0xFF);
    reportdata[8] = (deviceStatusMsg.deviceBLVersion[0]&0xFF);
    reportdata[9] = (deviceStatusMsg.deviceBLVersion[3]&0xFF);
    reportdata[10] = (deviceStatusMsg.deviceBLVersion[2]&0xFF);
    reportdata[11] = (deviceStatusMsg.deviceBLVersion[1]&0xFF);

}

void DeviceInfo::setShmCallback(uint8_t appid,uint16_t type,uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len){
    int aaa = 0;
    aaa = Ydshrdlib::getInston()->sendShmDataFunc(appid,type,device,childdevice,point,data,len);
//    zprintf3("setShmCallback  %d \n",aaa);
}


int DeviceInfo::queryKeyDataProc_emvd(uint32_t *deviceData)
{
    if (!emvdMap.empty())
    {
        std::lock_guard<std::mutex> lock(emvd_mutex);
        for (auto it = emvdMap.begin(); it != emvdMap.end();++it)
        {
            EmvdMsgInfo emvdMsgInfo = it->second;
            if(emvdMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_angle(uint32_t *deviceData)
{
    if (!angleMap.empty())
    {
        std::lock_guard<std::mutex> lock(angle_mutex);
        for (auto it = angleMap.begin(); it != angleMap.end();++it)
        {
            AngleMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_ir(uint32_t *deviceData)
{
    if (!irMap.empty())
    {
        std::lock_guard<std::mutex> lock(ir_mutex);
        for (auto it = irMap.begin(); it != irMap.end();++it)
        {
            IRMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_alarm(uint32_t *deviceData)
{
    if (!alarmMap.empty())
    {
        std::lock_guard<std::mutex> lock(alarm_mutex);
        for (auto it = alarmMap.begin(); it != alarmMap.end();++it)
        {
            AlarmMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_alarmAngle(uint32_t *deviceData)
{
    if (!alarmAngleMap.empty())
    {
        std::lock_guard<std::mutex> lock(alarmAngle_mutex);
        for (auto it = alarmAngleMap.begin(); it != alarmAngleMap.end();++it)
        {
            AngleMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_highAngle(uint32_t *deviceData)
{
    if (!highAngleMap.empty())
    {
        std::lock_guard<std::mutex> lock(highAngle_mutex);
        for (auto it = highAngleMap.begin(); it != highAngleMap.end();++it)
        {
            AngleMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_high(uint32_t *deviceData)
{
    if (!highMap.empty())
    {
        std::lock_guard<std::mutex> lock(high_mutex);
        for (auto it = highMap.begin(); it != highMap.end();++it)
        {
            HighMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_person(uint32_t *deviceData)
{
    if (!personMap.empty())
    {
        std::lock_guard<std::mutex> lock(person_mutex);
        for (auto it = personMap.begin(); it != personMap.end();++it)
        {
            PersonMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_MiniHub1(uint32_t *deviceData)
{
    if (!mini1Map.empty())
    {
        std::lock_guard<std::mutex> lock(miniHub_mutex);
        for (auto it = mini1Map.begin(); it != mini1Map.end();++it)
        {
            MiniHubMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}

int DeviceInfo::queryKeyDataProc_MiniHub2(uint32_t *deviceData)
{
    if (!mini2Map.empty())
    {
        std::lock_guard<std::mutex> lock(miniHub_mutex);
        for (auto it = mini2Map.begin(); it != mini2Map.end();++it)
        {
            MiniHubMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            *deviceData = it->first;
            return 0;
        }
    }
    return -1;
}


int DeviceInfo::queryKeyDataProc(uint8_t device, uint32_t *deviceData)
{
    switch(device)
    {
        case ID_EMVD:
           if(queryKeyDataProc_emvd(deviceData) == -1)
               return -1;
        break;
        case ID_ANGLE:
           if(queryKeyDataProc_angle(deviceData) == -1)
               return -1;
        break;
        case ID_IRRX:
           if(queryKeyDataProc_ir(deviceData) == -1)
               return -1;
        break;
        case ID_ALARM:
           if(queryKeyDataProc_alarm(deviceData) == -1)
               return -1;
        break;
        case ID_PERSON:
           if(queryKeyDataProc_person(deviceData) == -1)
               return -1;
        break;
        case ID_HIGH:
           if(queryKeyDataProc_high(deviceData) == -1)
               return -1;
        break;
        case ID_ALARMANGLE:
           if(queryKeyDataProc_alarmAngle(deviceData) == -1)
               return -1;
        break;
        case ID_MINIHUB1:
           if(queryKeyDataProc_MiniHub1(deviceData) == -1)
               return -1;
        break;
        case ID_MINIHUB2:
           if(queryKeyDataProc_MiniHub2(deviceData) == -1)
               return -1;
        break;
    }
    return 0;
}


int DeviceInfo::queryDeviceStateProc_emvd(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!emvdMap.empty())
    {
        std::lock_guard<std::mutex> lock(emvd_mutex);
        for (auto it = emvdMap.begin(); it != emvdMap.end();++it)
        {
            key = it->first;
            EmvdMsgInfo emvdMsgInfo = it->second;
            if(emvdMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            if(emvdMsgInfo.deviceStatusMsg.deviceStatus == DEVICEPROGRAM)
                continue;
            if(emvdMsgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                emvdMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,emvdMsgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_EMVD),1,0,data,12);
                zprintf3("emvd device report state init \n");
            }
            if(emvdMsgInfo.deviceStatusMsg.HeartTimer >= timer)
                emvdMsgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               if(emvdMsgInfo.iovalue != 0)
               {
                   StopEmvdAction_Offline((key>>16)&0xFF);
                   emvdMsgInfo.iovalue = 0;
               }
               emvdMsgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,emvdMsgInfo.deviceStatusMsg);
                zprintf3("emvd device report state offline \n");
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_EMVD),1,0,data,12);
            }
            if(emvdMsgInfo.iovalue != 0)
            {
                if(emvdMsgInfo.actionTimer >= 1)
                    emvdMsgInfo.actionTimer--;
                else
                {
                  zprintf3("queryDeviceStateProc_emvd:iovalue%d \n",(key&0xFF));
                  StopEmvdAction_Offline((key>>16)&0xFF);
                  emvdMsgInfo.iovalue = 0;
                   emvdMsgInfo.actionTimer = 0;
                }
            }
            if(emvdMsgInfo.checkio != 0 && emvdMsgInfo.state != 0)
            {
                zprintf3("emvd check io:%d,,,,sate:%d \n",emvdMsgInfo.checkio,emvdMsgInfo.state);
                data[0] = emvdMsgInfo.state;
                setShmCallback(1,52,((((key>>16)&0xFF)<<5)|ID_EMVD),1,emvdMsgInfo.checkio,data,2);
                emvdMsgInfo.checkio = 0;
                emvdMsgInfo.state = 0;
            }
            emvdMap[key] = emvdMsgInfo;
        }
    }
    return 0;
}


int DeviceInfo::queryDeviceStateProc_wireless(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!wirelessMap.empty())
    {
        std::lock_guard<std::mutex> lock(wireless_mutex);
        for (auto it = wirelessMap.begin(); it != wirelessMap.end();++it)
        {
            key = it->first;
            WirelessMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                continue;
            }
            if(msgInfo.deviceStatusMsg.deviceStatus == DEVICEPROGRAM)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_WIRELESS),1,0,data,12);
                zprintf3("wireless device report state init \n");
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
                zprintf3("emvd device report state offline \n");
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_WIRELESS),1,0,data,12);
            }
            wirelessMap[key] = msgInfo;
        }
    }
    return 0;
}


int DeviceInfo::queryDeviceStateProc_angle(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!angleMap.empty())
    {
        std::lock_guard<std::mutex> lock(angle_mutex);
        for (auto it = angleMap.begin(); it != angleMap.end(); ++it)
        {
            key = it->first;
            AngleMsgInfo angleMsgInfo = it->second;
            if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(angleMsgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,angleMsgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
                zprintf3("angle device report state init \n");
            }
            if(angleMsgInfo.deviceStatusMsg.HeartTimer >= timer)
                angleMsgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               angleMsgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,angleMsgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
               zprintf3("angle device report state offline \n");
            }
            if(angleMsgInfo.deviceStatusMsg.ReportTimer >= timer)
               angleMsgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(data,&angleMsgInfo.angle_x,6);
                angleMsgInfo.deviceStatusMsg.ReportTimer = ANGLE_REPORT_TIMER;
//                zprintf3("angle device report xyz  \n");
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,6);
            }
            angleMap[key] = angleMsgInfo;
        }
    }
    return 0;
}
int DeviceInfo::queryDeviceStateProc_alarmAngle(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!alarmAngleMap.empty())
    {
        std::lock_guard<std::mutex> lock(alarmAngle_mutex);
        for (auto it = alarmAngleMap.begin(); it != alarmAngleMap.end(); ++it)
        {
            key = it->first;
            AngleMsgInfo angleMsgInfo = it->second;
            if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(angleMsgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,angleMsgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
                zprintf3("angle_alarm device report state init \n");
            }
            if(angleMsgInfo.deviceStatusMsg.HeartTimer >= timer)
                angleMsgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               angleMsgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,angleMsgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
               zprintf3("angle_alarm device report state offline \n");
            }
            if(angleMsgInfo.deviceStatusMsg.ReportTimer >= timer)
               angleMsgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(data,&angleMsgInfo.angle_x,6);
                angleMsgInfo.deviceStatusMsg.ReportTimer = ANGLE_REPORT_TIMER;
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,6);
//                zprintf3("angle_alarm device report xyz \n");
            }
            alarmAngleMap[key] = angleMsgInfo;
        }
    }
    return 0;
}
int DeviceInfo::queryDeviceStateProc_highAngle(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!highAngleMap.empty())
    {
        std::lock_guard<std::mutex> lock(highAngle_mutex);
        for (auto it = highAngleMap.begin(); it != highAngleMap.end(); ++it)
        {
            key = it->first;
            AngleMsgInfo angleMsgInfo = it->second;
            if(angleMsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(angleMsgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                angleMsgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,angleMsgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
                zprintf3("high_angle device report state init \n");
            }
            if(angleMsgInfo.deviceStatusMsg.HeartTimer >= timer)
                angleMsgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               angleMsgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,angleMsgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
               zprintf3("high_angle device report state offline \n");
            }
            if(angleMsgInfo.deviceStatusMsg.ReportTimer >= timer)
               angleMsgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(data,&angleMsgInfo.angle_x,6);
                angleMsgInfo.deviceStatusMsg.ReportTimer = ANGLE_REPORT_TIMER;
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,6);
//                zprintf3("high_angle device report subdevice: %d \n",(key&0xFF));
            }
            highAngleMap[key] = angleMsgInfo;
        }
    }
    return 0;
}



int DeviceInfo::queryDeviceStateProc_high(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!highMap.empty())
    {
        std::lock_guard<std::mutex> lock(high_mutex);
        for (auto it = highMap.begin(); it != highMap.end(); ++it)
        {
            key = it->first;
            HighMsgInfo highsgInfo = it->second;
            if(highsgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(highsgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                highsgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,highsgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
                zprintf3("high device report state init \n");
            }
            if(highsgInfo.deviceStatusMsg.HeartTimer >= timer)
                highsgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               highsgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,highsgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,12);
               zprintf3("high device report state offline \n");
            }
            if(highsgInfo.deviceStatusMsg.ReportTimer >= timer)
               highsgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(data,&highsgInfo.high,2);
                highsgInfo.deviceStatusMsg.ReportTimer = HIGH_REPORT_TMIER;
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,2);
//                zprintf3("high device report high \n");
            }
            highMap[key] = highsgInfo;
        }
    }
    return 0;
}
int DeviceInfo::queryDeviceStateProc_person(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!personMap.empty())
    {
        std::lock_guard<std::mutex> lock(person_mutex);
        for (auto it = personMap.begin(); it != personMap.end(); ++it)
        {
            key = it->first;
            PersonMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_PERSON),1,0,data,12);
                zprintf3("person device report state init verson%d,%d,%d\n",
                         msgInfo.deviceStatusMsg.deviceVersion[0],
                         msgInfo.deviceStatusMsg.deviceVersion[1],
                         msgInfo.deviceStatusMsg.deviceVersion[2],
                         msgInfo.deviceStatusMsg.deviceVersion[3]);
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_PERSON),1,0,data,12);
               zprintf3("person  report state offline \n");
            }
            personMap[key] = msgInfo;
        }
    }
    return 0;
}
int DeviceInfo::queryDeviceStateProc_alarm(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!alarmMap.empty())
    {
        std::lock_guard<std::mutex> lock(alarm_mutex);
        for (auto it = alarmMap.begin(); it != alarmMap.end(); ++it)
        {
            key = it->first;
            AlarmMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_ALARM),1,0,data,12);
//                zprintf3("alarm device report state init verson%d,%d,%d\n",
//                        msgInfo.deviceStatusMsg.deviceVersion[0],
//                        msgInfo.deviceStatusMsg.deviceVersion[1],
//                        msgInfo.deviceStatusMsg.deviceVersion[2],
//                        msgInfo.deviceStatusMsg.deviceVersion[3]);
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_ALARM),1,0,data,12);
               zprintf3("alarm  report state offline \n");
            }
            alarmMap[key] = msgInfo;
        }
    }
    return 0;
}
int DeviceInfo::queryDeviceStateProc_ir(uint8_t timer)
{
    uint8_t data[12];
    uint32_t key;
    if (!irMap.empty())
    {
        std::lock_guard<std::mutex> lock(ir_mutex);
        for (auto it = irMap.begin(); it != irMap.end(); ++it)
        {
            key = it->first;
            IRMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_IRRX),1,0,data,12);
                zprintf3("ir device report state init\n");
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|ID_IRRX),1,0,data,12);
               zprintf3("ir  report state offline \n");
            }
            if(msgInfo.deviceStatusMsg.ReportTimer >= timer)
               msgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                if(msgInfo.irCounts != 0)
                {
                    zprintf3("ir  report ircount%d \n",msgInfo.irCounts);
                    memset(data,0,sizeof(12));
                    memcpy(&data[4],&msgInfo.irCounts,2);
                    data[0] = 1;
                    msgInfo.deviceStatusMsg.ReportTimer = WL_REPORT_TIMER;
                    msgInfo.irCounts = 0;
                    setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,6);
                }
            }
            irMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::queryDeviceStateProc_MiniHub1(uint8_t timer)
{
    uint8_t data[32];
    uint32_t key;
    if (!mini1Map.empty())
    {
        std::lock_guard<std::mutex> lock(ir_mutex);
        for (auto it = mini1Map.begin(); it != mini1Map.end(); ++it)
        {
            key = it->first;
            MiniHubMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),0,0,data,12);
                zprintf3("minihub1 device report state init\n");
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),1,0,data,12);
               zprintf3("minihub1  report state offline \n");
            }
            if(msgInfo.deviceStatusMsg.ReportTimer >= timer)
               msgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(&data[0],&msgInfo.analog1,8);
                memcpy(&data[8],&msgInfo.AngleMsg[0].angle_x,6);
                memcpy(&data[8+6],&msgInfo.AngleMsg[1].angle_x,6);
                memcpy(&data[8+6+6],&msgInfo.AngleMsg[2].angle_x,6);
                memcpy(&data[8+6+6+6],&msgInfo.AngleMsg[3].angle_x,6);
                msgInfo.deviceStatusMsg.ReportTimer = HUB_REPORT_TIMER;
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,32);
//                zprintf3("minihub1  report ircount \n");
            }
            for(uint8_t i = 0; i<4; i++)
            {
                if(msgInfo.AngleMsg[i].deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    continue;
                if(msgInfo.AngleMsg[i].deviceStatusMsg.ReportState == STATE_MODE_REPORT)
                {
                    msgInfo.AngleMsg[i].deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                    getReportDeviceData(data,msgInfo.AngleMsg[i].deviceStatusMsg);
                    setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),i+1,0,data,12);
                    zprintf3("minihub1 device report state init\n");
                }
                if(msgInfo.AngleMsg[i].deviceStatusMsg.HeartTimer >= timer)
                    msgInfo.AngleMsg[i].deviceStatusMsg.HeartTimer -= timer;
                else
                {
                   msgInfo.AngleMsg[i].deviceStatusMsg.isOnline = DEVICE_OFFLINE;
                   getReportDeviceData(data,msgInfo.deviceStatusMsg);
                   setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),i+1,0,data,12);
                   zprintf3("minihub1  report state offline \n");
                }
            }
            mini1Map[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::queryDeviceStateProc_MiniHub2(uint8_t timer)
{
    uint8_t data[32];
    uint32_t key;
    if (!mini2Map.empty())
    {
        std::lock_guard<std::mutex> lock(ir_mutex);
        for (auto it = mini2Map.begin(); it != mini2Map.end(); ++it)
        {
            key = it->first;
            MiniHubMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),0,0,data,12);
                zprintf3("minihub2 device report state init\n");
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),1,0,data,12);
               zprintf3("minihub2  report state offline \n");
            }
            if(msgInfo.deviceStatusMsg.ReportTimer >= timer)
               msgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(&data[0],&msgInfo.analog1,8);
                memcpy(&data[8],&msgInfo.AngleMsg[0].angle_x,6);
                memcpy(&data[8+6],&msgInfo.AngleMsg[1].angle_x,6);
                memcpy(&data[8+6+6],&msgInfo.AngleMsg[2].angle_x,6);
                memcpy(&data[8+6+6+6],&msgInfo.AngleMsg[3].angle_x,6);
                msgInfo.deviceStatusMsg.ReportTimer = HUB_REPORT_TIMER;
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,32);

                // 获取当前时间
               std::time_t currentTime = std::time(nullptr);

               // 将当前时间转换为本地时间
               std::tm* localTime = std::localtime(&currentTime);

               // 使用 strftime 函数格式化时间
               char timeString[100];
               std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localTime);
               std::cout << "当前时间: " << timeString << std::endl;

//                zprintf3("minihub2  report app value \n");
            }
            for(uint8_t i = 0; i<4; i++)
            {
                if(msgInfo.AngleMsg[i].deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    continue;
                if(msgInfo.AngleMsg[i].deviceStatusMsg.ReportState == STATE_MODE_REPORT)
                {
                    msgInfo.AngleMsg[i].deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                    getReportDeviceData(data,msgInfo.AngleMsg[i].deviceStatusMsg);
                    setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),i+1,0,data,12);
                    zprintf3("minihub2 device report state init\n");
                }
                if(msgInfo.AngleMsg[i].deviceStatusMsg.HeartTimer >= timer)
                    msgInfo.AngleMsg[i].deviceStatusMsg.HeartTimer -= timer;
                else
                {
                   msgInfo.AngleMsg[i].deviceStatusMsg.isOnline = DEVICE_OFFLINE;
                   getReportDeviceData(data,msgInfo.deviceStatusMsg);
                   setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),i+1,0,data,12);
                   zprintf3("minihub2  report state offline \n");
                }
            }
            mini2Map[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::queryDeviceStateProc_MiniHub3(uint8_t timer)
{
    uint8_t data[32];
    uint32_t key;
    if (!mini3Map.empty())
    {
        std::lock_guard<std::mutex> lock(miniHub_mutex);
        for (auto it = mini3Map.begin(); it != mini3Map.end(); ++it)
        {
            key = it->first;
            MiniHubMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            if(msgInfo.deviceStatusMsg.ReportState == STATE_MODE_REPORT)
            {
                msgInfo.deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                getReportDeviceData(data,msgInfo.deviceStatusMsg);
                setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),0,0,data,12);
                zprintf3("minihub3 device report state init\n");
            }
            if(msgInfo.deviceStatusMsg.HeartTimer >= timer)
                msgInfo.deviceStatusMsg.HeartTimer -= timer;
            else
            {
               msgInfo.deviceStatusMsg.isOnline = DEVICE_OFFLINE;
               getReportDeviceData(data,msgInfo.deviceStatusMsg);
               setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),1,0,data,12);
               zprintf3("minihub3  report state offline \n");
            }
            if(msgInfo.deviceStatusMsg.ReportTimer >= timer)
               msgInfo.deviceStatusMsg.ReportTimer -= timer;
            else
            {
                memset(data,0,sizeof(12));
                memcpy(&data[0],&msgInfo.analog1,8);
                memcpy(&data[8],&msgInfo.AngleMsg[0].angle_x,6);
                memcpy(&data[8+6],&msgInfo.AngleMsg[1].angle_x,6);
                memcpy(&data[8+6+6],&msgInfo.AngleMsg[2].angle_x,6);
                memcpy(&data[8+6+6+6],&msgInfo.AngleMsg[3].angle_x,6);
                msgInfo.deviceStatusMsg.ReportTimer = HUB_REPORT_TIMER;
                setShmCallback(1,55,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),(key&0xFF),0,data,32);
//                zprintf3("minihub1  report ircount \n");
            }
            for(uint8_t i = 0; i<4; i++)
            {
                if(msgInfo.AngleMsg[i].deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                    continue;
                if(msgInfo.AngleMsg[i].deviceStatusMsg.ReportState == STATE_MODE_REPORT)
                {
                    msgInfo.AngleMsg[i].deviceStatusMsg.ReportState = STATE_MODE_NORMAL;
                    getReportDeviceData(data,msgInfo.AngleMsg[i].deviceStatusMsg);
                    setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),i+1,0,data,12);
                    zprintf3("minihub3 device report state init\n");
                }
                if(msgInfo.AngleMsg[i].deviceStatusMsg.HeartTimer >= timer)
                    msgInfo.AngleMsg[i].deviceStatusMsg.HeartTimer -= timer;
                else
                {
                   msgInfo.AngleMsg[i].deviceStatusMsg.isOnline = DEVICE_OFFLINE;
                   getReportDeviceData(data,msgInfo.deviceStatusMsg);
                   setShmCallback(1,65,((((key>>16)&0xFF)<<5)|((key>>8)&0xFF)),i+1,0,data,12);
                   zprintf3("minihub3  report state offline \n");
                }
            }
            mini3Map[key] = msgInfo;
        }
    }
    return 0;
}


int DeviceInfo::SetAllDeviceStateProc_emvd(void)
{
    uint32_t key;
    if (!emvdMap.empty())
    {
        std::lock_guard<std::mutex> lock(emvd_mutex);
        for (auto it = emvdMap.begin(); it != emvdMap.end(); ++it)
        {
            key = it->first;
            EmvdMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            emvdMap[key] = msgInfo;
        }
    }
    return 0;
}


int DeviceInfo::SetAllDeviceStateProc_wireless(void)
{
    uint32_t key;
    if (!wirelessMap.empty())
    {
        std::lock_guard<std::mutex> lock(wireless_mutex);
        for (auto it = wirelessMap.begin(); it != wirelessMap.end(); ++it)
        {
            key = it->first;
            WirelessMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            wirelessMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::SetAllDeviceStateProc_ir(void)
{
    uint32_t key;
    if (!irMap.empty())
    {
        std::lock_guard<std::mutex> lock(ir_mutex);
        for (auto it = irMap.begin(); it != irMap.end(); ++it)
        {
            key = it->first;
            IRMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            irMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::SetAllDeviceStateProc_angle(void)
{
    uint32_t key;
    if (!angleMap.empty())
    {
        std::lock_guard<std::mutex> lock(angle_mutex);
        for (auto it = angleMap.begin(); it != angleMap.end(); ++it)
        {
            key = it->first;
            AngleMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            angleMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::SetAllDeviceStateProc_high(void)
{
    uint32_t key;
    if (!highMap.empty())
    {
        std::lock_guard<std::mutex> lock(high_mutex);
        for (auto it = highMap.begin(); it != highMap.end(); ++it)
        {
            key = it->first;
            HighMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            highMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::SetAllDeviceStateProc_highAngle(void)
{
    uint32_t key;
    if (!highAngleMap.empty())
    {
        std::lock_guard<std::mutex> lock(highAngle_mutex);
        for (auto it = highAngleMap.begin(); it != highAngleMap.end(); ++it)
        {
            key = it->first;
            AngleMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            highAngleMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::SetAllDeviceStateProc_alarmAngle(void)
{
    uint32_t key;
    if (!alarmAngleMap.empty())
    {
        std::lock_guard<std::mutex> lock(alarmAngle_mutex);
        for (auto it = alarmAngleMap.begin(); it != alarmAngleMap.end(); ++it)
        {
            key = it->first;
            AngleMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            alarmAngleMap[key] = msgInfo;
        }
    }
    return 0;
}

int DeviceInfo::SetAllDeviceStateProc_person(void)
{
    uint32_t key;
    if (!personMap.empty())
    {
        std::lock_guard<std::mutex> lock(person_mutex);
        for (auto it = personMap.begin(); it != personMap.end(); ++it)
        {
            key = it->first;
            PersonMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            personMap[key] = msgInfo;
        }
    }
    return 0;
}


int DeviceInfo::SetAllDeviceStateProc_alarm(void)
{
    uint32_t key;
    if (!alarmMap.empty())
    {
        std::lock_guard<std::mutex> lock(alarm_mutex);
        for (auto it = alarmMap.begin(); it != alarmMap.end(); ++it)
        {
            key = it->first;
            AlarmMsgInfo msgInfo = it->second;
            if(msgInfo.deviceStatusMsg.isOnline == DEVICE_OFFLINE)
                continue;
            msgInfo.deviceStatusMsg.ReportState = STATE_MODE_REPORT;
            alarmMap[key] = msgInfo;
        }
    }
    return 0;
}





