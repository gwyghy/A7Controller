#include <QCoreApplication>
#include "controllerlogic.h"
#include <QDebug>
#include <signal.h>
#include "qtextcodec.h"
#include "client/linux/handler/exception_handler.h"
#include "unistd.h"

/****/
ControllerLogic *pControllerLogic = nullptr;

/******/
void signalTermHandler(int sig)
{
    qDebug()<<"Hys Server signalTermHandler sig:"<<sig;

    qApp->exit(0);
}

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
    QString cmdStr = "/opt/bin/breakpad/dump.sh HysA7ControllerLogic " + path +" usr";
#endif
    system(cmdStr.toStdString().c_str());
    return succeeded;
}

int main(int argc, char *argv[])
{
#if 0
    /**结束/终止进程运行信号的处理**/
    signal(SIGTERM,signalTermHandler);// 终止进程     软件终止信号
    signal(SIGKILL,signalTermHandler);//终止进程     杀死进程
    signal (SIGINT, signalTermHandler);//终止进程     中断进程
#endif

    QCoreApplication a(argc, argv);

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

    qDebug()<<"*************ControllerLogic,main,1,ing**********";
    pControllerLogic = new ControllerLogic();
    qDebug()<<"*************ControllerLogic,main,2,ing**********";

    return a.exec();
}
