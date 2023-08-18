#include <QCoreApplication>
#include "key/key.h"
#include "wirless/wirless.h"
#include "ir/ir.h"
#include "ydshrdlib.h"
#include <unistd.h>
#include "timer/timer.h"
#include <qdebug.h>

#ifdef BREAKPAD
    #include "client/linux/handler/exception_handler.h"

    /*******/
    static bool dumpCallBack(const google_breakpad::MinidumpDescriptor &descriptor,
                             void *context,
                             bool succeeded)
    {
        QString path = QString(descriptor.path());
    #ifdef PLATFORM_PC
        qDebug()<<"dumpCallBack path:"<<path;
        QString cmdStr = "/home/hn/A7Lib/install_pc/bin/breakpad/dump.sh HysA7ControllerLogic " + path +" user";
    #else
        //todo: path
        qDebug()<<"dumpCallBack path:"<<path;
        QString cmdStr = "/opt/bin/breakpad/dump.sh a7uartdriver " + path +" platform";
    #endif
        system(cmdStr.toStdString().c_str());
        return succeeded;
    }
#endif
/*******************************************************************************************
**函数名称：RecvMsgCallbackinit
**函数作用：初始化消息接收回调（8）
**函数参数：uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvMsgCallbackinit(uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len)
{
    (void) childdevice;     // 使用类型转换将未使用的参数转换为空操作，解决警告
    (void) point;           // 使用类型转换将未使用的参数转换为空操作，解决警告

    if(device == ((1<<5)|8))                //键盘消息
    {
        RecvMsgCallbackKeyinit(point,data,len);
    }
    else if(device == ((2<<5)|9))           //无线消息
    {
        RecvMsgCallback_Wlinit(point,data,len);
    }
    else
    {

    }
}

/*******************************************************************************************
**函数名称：RecvMsgCallbackFileDownLoad
**函数作用：文件下载消息接收回调
**函数参数：uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvMsgCallbackFileDownLoad(uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len)
{
    (void) childdevice;     // 使用类型转换将未使用的参数转换为空操作，解决警告

    if(device == ((1<<5)|8))                //键盘
    {
        RecvMsgCallback_KeyFileDownLoad(point,data,len);
    }
    else if(device == ((2<<5)|9))           //无线
    {
        RecvMsgCallback_WlFileDownLoad(point,data,len);
    }
    else
    {

    }
}

/*******************************************************************************************
**函数名称：RecvShmCallbackinit
**函数作用：线程接收共享内存回调
**函数参数：uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvShmCallbackinit(uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len)
{
    (void) childdevice;     // 使用类型转换将未使用的参数转换为空操作，解决警告

    if(device == ((1<<5)|8))                //键盘
    {
        RecvShmCallback_KeyLedCtrl(point,data,len);
    }
    else if(device == ((2<<5)|9))           //无线
    {
        RecvShmCallback_Wl(point,data,len);
    }
    else
    {

    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    #ifdef BREAKPAD
        #ifdef PLATFORM_PC
            //init google_breakpad
            qDebug()<<"main 1";
            google_breakpad::MinidumpDescriptor descriptor("/home/hn/A7Lib/install_pc/bin/breakpad/Logs/crashlog");
            qDebug()<<"main 2";
            google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallBack, NULL, true, -1);
            qDebug()<<"main 3";
        #else
            //init google_breakpad
            //todo: path
            qDebug()<<"main 1";
            google_breakpad::MinidumpDescriptor descriptor("/opt/bin/breakpad/Logs/crashlog");
            qDebug()<<"main 2";
            google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallBack, NULL, true, -1);
            qDebug()<<"main 3";
        #endif
    #endif
    //共享库初始化
    Ydshrdlib::getInston()->InitLibParam(61,87654333,87654332);
    //注册消息接收处理回调(8，初始化设备)
    Ydshrdlib::getInston()->setMsgCallBack(1,8,RecvMsgCallbackinit);
    //注册消息接收处理回调(2，文件下载)
    Ydshrdlib::getInston()->setMsgCallBack(1,2,RecvMsgCallbackFileDownLoad);
    //注册共享内存接收处理回调（57，驱动控制）
    Ydshrdlib::getInston()->setShmCallBack(1,57,RecvShmCallbackinit);

    //键盘逻辑
    KeyAppStart();

    //红外逻辑
    InfraredAppStart();

    //无线逻辑
    WirlessAppStart();

    while(1)
    {
        Timer::getInstance().Util_Timer_Task(); //定时器周期任务
        usleep(1000*10);
    }

    return a.exec();
}
