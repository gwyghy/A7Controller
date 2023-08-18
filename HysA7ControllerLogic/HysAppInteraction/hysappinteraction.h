#ifndef HYSAPPINTERACTION_H
#define HYSAPPINTERACTION_H

/*****/
#include "./msg/AppMsg.h"
#include "appmsgtype.h"

/*****/
class HysAppInteraction
{
public:
    HysAppInteraction();
    ~HysAppInteraction();
    static HysAppInteraction *getInston();
    void Init(void);
    void ReceiveMsgProc(void);

    int InsertSendMsgProc(u_int16_t  frametype, u_int16_t  devtype, u_int16_t devid, uint8_t *pbuf, u_int16_t length);
private:
    AppMsg *m_AppMsg;
    /****/
    static HysAppInteraction* that;

private:
    /***消息相关***/
    QMap<u_int16_t,AppMsgProcFuncType> msgproctab;//消息类型，处理函数
};

#endif // HYSAPPINTERACTION_H
