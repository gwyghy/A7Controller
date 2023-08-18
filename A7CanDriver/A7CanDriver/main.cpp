#include <can_bus.h>
#include "candata.h"
#include <stdio.h>
#include "e_poll.h"
#include "timers.h"
#include "pthread.h"
#include "can_device_app.h"
//#include "client/linux/handler/exception_handler.h"

DeviceInfo& deviceInfo1 = DeviceInfo::getInstance();
Can_Data  * gCanInfo = NULL;


/*******/
//static bool dumpCallBack(const google_breakpad::MinidumpDescriptor &descriptor,
//                         void *context,
//                         bool succeeded)
//{
//    zprintf3("main 2,1");
//    QString path = QString(descriptor.path());
//#ifdef PLATFORM_PC
//    qDebug()<<"dumpCallBack path:"<<path;
//    QString cmdStr = "/home/hn/A7Lib/install_pc/bin/breakpad/dump.sh HysA7ControllerLogic " + path +" user";
//#else
//    //todo: path
//   zprintf3("main 2,2");
//   zprintf3("dumpCallBack path:%s",path);
//   zprintf3("main 2,3");
//    QString cmdStr = "/opt/bin/breakpad/dump.sh a7candriver " + path +" platform";
//   zprintf3("main 2,4");
//#endif
//    system(cmdStr.toStdString().c_str());
//    zprintf3("main 2,5");
//    return succeeded;
//}

//void crash() { volatile int* a = (int*)(NULL); *a = 1; }

int main(int argc, char *argv[])
{

    gCanInfo = new Can_Data();
    if(gCanInfo == NULL){
        zprintf1("can data creat failed!\n");
        return -1;
    }

      gCanInfo->creat_can_bus_pro();
      gCanInfo->can_app_init();


//#ifdef PLATFORM_PC
//    //init google_breakpad
//    qDebug()<<"main 1";
//    google_breakpad::MinidumpDescriptor descriptor("/home/hn/A7Lib/install_pc/bin/breakpad/Logs/crashlog");
//    qDebug()<<"main 2";
//    google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallBack, NULL, true, -1);
//    qDebug()<<"main 3";
//#else
    //init google_breakpad
    //todo: path
//    zprintf3("main 1");
//    google_breakpad::MinidumpDescriptor descriptor("/opt/bin/breakpad/Logs/crashlog");
//    zprintf3("main 2");
//    google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallBack, NULL, true, -1);
//    zprintf3("main 3");
//    crash();
//#endif

      while(1)
      {
          sleep(1);
      }

    return 0;
}


