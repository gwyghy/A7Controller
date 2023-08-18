#ifndef PART_H
#define PART_H

/*******************************/
#include "./HysDeviceMng/devctrl.h"
#include "./ScRunParamMng/paramglobaldefine.h"
#include "./ScRunParamMng/partparam.h"

/*------------------------state define----------------------*/

typedef enum
{
    ProtectType_Protect=0,
    ProtectType_Normal,			// 正常状态
    ProtectType_LowProtect,		// 超下限保护状态
    ProtectType_HighProtect		// 超上限保护
}ProtectType;

typedef enum
{
    PartDeviceStateEnum_STATE0=0,//状态0：收、降
    PartDeviceStateEnum_STATE1,  //状态1：伸、升
}PartDeviceStateType;

typedef struct
{
    ParamUnit       Id;          //新增加。mry
    ParamUnit		partState;	//部件状态
}PartStateType;



/*------------------------part module define----------------------*/
typedef struct
{
    PartParamType		*pParam;
    PartStateType		*pState;

    int 				ticks_delay[PART_TRIGGER_NUM_MAX];
    u8					alarm_flag[PART_TRIGGER_NUM_MAX];
    ParamUnit			value_bak[PART_TRIGGER_NUM_MAX];
}partModuleType;


/*------------------------other define----------------------*/
void partModuleInit(void);
void partModuleLoad(u8 moduleID,PartParamType *pParam,PartStateType *pState);
u8 partCycleProcess(u8 moduleID,u16 delay) ;
uint8_t getPartInSingleActionOfSensorId(u_int16_t singleActionId);


#endif // PART_H
