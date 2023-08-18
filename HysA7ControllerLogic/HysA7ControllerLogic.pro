QT +=  core xml
QT -= gui

#CONFIG += c++11 console
CONFIG   += console
CONFIG -= app_bundle

QMAKE_CXXFLAGS += -std=c++0x

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    Sensor/sensor.cpp \
    Action/singleaction.cpp \
    Action/combaction.cpp \
    Action/groupaction.cpp \
    Action/part.cpp \
    Action/alarm.cpp \
    HysController/hyscontrollerlogic.cpp \
    HysSystem/hyssystem.cpp \
    Action/actionctrl.cpp \
    HysDeviceMng/hysdevicemng.cpp \
    HysDeviceMng/devwirelessmodule.cpp \
    HysDeviceMng/devselfio.cpp \
    HysDeviceMng/devselfinfred.cpp \
    HysDeviceMng/devselfad.cpp \
    HysDeviceMng/devperson.cpp \
    HysDeviceMng/devnfc.cpp \
    HysDeviceMng/devglobaltype.cpp \
    HysDeviceMng/devemvd.cpp \
    HysDeviceMng/devctrl.cpp \
    HysDeviceMng/devblueteeth.cpp \
    HysDeviceMng/devalarm.cpp \
    HysDeviceMng/hysdevmsg.cpp \
    HysDeviceMng/v4hysapp.cpp \
    controllerlogic.cpp \
    HysAppInteraction/hysappinteraction.cpp \
    HysAppInteraction/hysappmsg.cpp \
    HysDeviceMng/devkey.cpp


HEADERS += \
    Sensor/sensor.h \
    Action/singleaction.h \
    Action/combaction.h \
    Action/groupaction.h \
    Action/part.h \
    Action/alarm.h \
    HysController/hyscontrollertype.h \
    HysSystem/hysmsg.h \
    HysController/hyscontrollerlogic.h \
    HysSystem/hyssystem.h \
    HysSystem/logdatatype.h \
    Action/actionctrl.h \
    Action/actionctrldatatype.h \
    HysSystem/hyssystemtype.h \
    HysDeviceMng/v4hysapptype.h \
    HysDeviceMng/hysdevicemng.h \
    HysDeviceMng/devwirelessmodule.h \
    HysDeviceMng/devtype.h \
    HysDeviceMng/devselfkeymodule.h \
    HysDeviceMng/devselfio.h \
    HysDeviceMng/devselfinfred.h \
    HysDeviceMng/devselfad.h \
    HysDeviceMng/devperson.h \
    HysDeviceMng/devnfc.h \
    HysDeviceMng/devglobaltype.h \
    HysDeviceMng/devemvd.h \
    HysDeviceMng/devctrl.h \
    HysDeviceMng/devblueteeth.h \
    HysDeviceMng/devalarm.h \
    HysDeviceMng/hysdevmsg.h \
    HysDeviceMng/v4hysapp.h \
    controllerlogic.h \
    HysAppInteraction/hysappinteraction.h \
    HysAppInteraction/appmsgtype.h \
    HysAppInteraction/hysappmsg.h \
    HysDeviceMng/devkey.h

linux-gnueabi-oe-g++{
                message("amd64")
                DEFINES += PLATFORM_ARM
                #DEFINES += QT_NO_DEBUG_OUTPUT

                INCLUDE_PATH= ../../../install_arm/include
                LIB_PATH = ../../../install_arm/lib

                INCLUDEPATH += $$INCLUDE_PATH/LibDevMng/
                 LIBS +=-L$$LIB_PATH/ -lLibDevMng

                INCLUDEPATH +=$$INCLUDE_PATH/IPC/
                LIBS +=-L$$LIB_PATH/ -lIPC

                INCLUDEPATH +=$$INCLUDE_PATH/HysA7ParamMng/
                LIBS +=-L$$LIB_PATH/ -lHysA7ParamMng

                INCLUDEPATH+= /home/hn/tools/breakpad_src_new/breakpad/src
                LIBS +=-L/home/hn/tools/output_arm_new/lib -lbreakpad_client

                APP_INSTALL_PATH_PLATFORM = ../../../install_arm/bin/user
}

linux-g++-64 {
                message("amd64")
                DEFINES += PLATFORM_PC
                #DEFINES += QT_NO_DEBUG_OUTPUT

                INCLUDE_PATH= ../../../install_pc/include
                LIB_PATH = ../../../install_pc/lib

                INCLUDEPATH += $$INCLUDE_PATH/LibDevMng/
                 LIBS +=-L$$LIB_PATH/ -lLibDevMng

                INCLUDEPATH +=$$INCLUDE_PATH/IPC/
                LIBS +=-L$$LIB_PATH/ -lIPC

                INCLUDEPATH +=$$INCLUDE_PATH/HysA7ParamMng/
                LIBS +=-L$$LIB_PATH/ -lHysA7ParamMng

                INCLUDEPATH+= /home/hn/tools/breakpad_arm/src
                LIBS +=-L/home/hn/tools/output/lib -lbreakpad_client

                APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/user
}

linux-g++{
                message("amd64")
                DEFINES += PLATFORM_PC
                #DEFINES += QT_NO_DEBUG_OUTPUT

                INCLUDE_PATH= ../../../install_pc/include
                LIB_PATH = ../../../install_pc/lib

                INCLUDEPATH += $$INCLUDE_PATH/LibDevMng/
                 LIBS +=-L$$LIB_PATH/ -lLibDevMng

                INCLUDEPATH +=$$INCLUDE_PATH/IPC/
                LIBS +=-L$$LIB_PATH/ -lIPC

                INCLUDEPATH +=$$INCLUDE_PATH/HysA7ParamMng/
                LIBS +=-L$$LIB_PATH/ -lHysA7ParamMng

#                INCLUDEPATH+= /home/hn/tools/breakpad_arm/src
#                LIBS +=-L/home/hn/tools/output/lib -lbreakpad_client

                APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/user
}

target.path = $$APP_INSTALL_PATH_PLATFORM
INSTALLS += target
