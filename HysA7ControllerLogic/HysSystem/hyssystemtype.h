#ifndef HYSSYSTEMTYPE_H
#define HYSSYSTEMTYPE_H

#include "./HysDeviceMng/v4hysapptype.h"
#include "./HysAppInteraction/appmsgtype.h"

/*******/
enum
{
    SYSSTOP_UNPRESS =0,
    SYSSTOP_PRESS,
    SYSSTOP_STATE_MAX
};

/*******/
enum
{
    SystemEmergencySrcType_Stop =1,
    SystemEmergencySrcType_Lock,
    SystemEmergencySrcType_End,
    SystemEmergencySrcType_Person_Lock = 12,
    SystemEmergencySrcType_MAX
};

/*******/
typedef union
{
    struct{
        u8		End   		:1;				//停止状态
        u8		Lock        :1;				//闭锁状态
        u8		Stop        :1;				//急停状态
        u8		SoftVersionSame     :1;         //控制器版本状态
        u8      GlobalParamCrcSame :1;         //全局参数不一致
        u8      NONE     		:2;
    } SysStop;
    u8 FuncStop;
}SysStopType;

/***参数修改后，需进行下一部处理的菜单ID***/
typedef struct
{
    u_int16_t menuID;//一级菜单ID
    u_int16_t SubMenuId;//二级菜单ID
    u_int16_t AppMsgType;//需对应的应用消息ID
    HysAppSendMsgProcFuncType msgfunc;//对应的消息发送
}ParamChangedNeedProcType;

#endif // HYSSYSTEMTYPE_H
