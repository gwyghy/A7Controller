#ifndef ACTIONCTRLDATATYPE_H
#define ACTIONCTRLDATATYPE_H

#include "libhysa7parammng.h"
#include "HysDeviceMng/v4hysapptype.h"
#include "combaction.h"
#include "groupaction.h"

/********/
#define  CTRL_PARAM_LEN_MAX		100
#define  CTRL_BUF_LEN			10
#define  ACTIONINFO_BUF_LEN		40
#define  MANUALACTIONINFO_LEN   10

/********/
typedef struct
{
    u8				  actionType;			//0:单动作 1：组合动作  2：成组动作
    u8				  actionID;
    u8				  ParamLength;
    u8				  ParamBuf[CTRL_PARAM_LEN_MAX];
    ActionCtrlType    ctrl;
    void              *pNext;
}SysCtrlInfoType;

typedef struct
{
    SysCtrlInfoType    buf[CTRL_BUF_LEN];	//发送消息buf
    u8                 Cnt;					//发送链表项数
    SysCtrlInfoType*   pHead;				//发送链表首项
    SysCtrlInfoType*   pEnd;				//发送链表末项
}SysCtrlInfoBufType;

typedef struct
{
    u8				  actionType;			//0:单动作 1：组合动作  2：成组动作
    u8				  actionID;
    u8				  runstate;
    void              *pNext;
}ActionInfoType;

typedef struct
{
    ActionInfoType     buf[ACTIONINFO_BUF_LEN];	//发送消息buf
    u8                 Cnt;					//发送链表项数
    ActionInfoType*    pHead;				//发送链表首项
    ActionInfoType*    pEnd;				//发送链表末项
    ActionInfoType*    pFree;				//空闲链表
}ActionInfoBufType;

#endif // ACTIONCTRLDATATYPE_H
