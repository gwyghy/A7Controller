QT       += core xml

QT       -= gui

TARGET = a7candriver
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++0x


TEMPLATE = app
DEFINES += QT_NO_DEBUG_OUTPUT



linux-arm-gnueabi-g++{
                                         DEFINES += ARM_PLATFORM
}
linux-g++-64{
           DEFINES += PC_PLATFORM
}
linux-g++{
           DEFINES += PC_PLATFORM
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH = .\
        mutex\
        epoll\
        timer\
        zprint\
        canbus\
        canpro\
        files\
        reflect\
        prodata\
        prodata/sem\
        ptxml\
        msg\
        candata\
        modbus\
        app

SOURCES += \
        app/can_device_app.cpp \
        app/dyk_v2callback.cpp \
        app/dyk_v2pro.cpp \
        canbus/can_bus.cpp \
        candata/candata.cpp \
        canpro/can_protocol.cpp \
        main.cpp \
        timer/timers.cpp \
        zprint/zprint.cpp


HEADERS += \
    app/can_device_app.h \
    app/dyk_v2callback.h \
    app/dyk_v2pro.h \
    canbus/can_bus.h \
    canbus/can_relate.h \
    candata/candata.h \
    canpro/can_protocol.h \
    epoll/e_poll.h \
    files/file_handler.h \
    files/fileload.h \
    mutex/mutex.h \
    mutex/mutex.h \
    prodata/pro_data.h \
    timer/timers.h \
    zprint/zprint.h

linux-g++-64 {
            message("amd64")
            DEFINES += I386
            DEFINES += QT_NO_DEBUG_OUTPUT

            INCLUDE_PATH= ../../../install_pc/include
            LIB_PATH = ../../../install_pc/lib

            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp

            APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/platform
}

linux-g++ {
            message("amd64")
            DEFINES += I386
            DEFINES += QT_NO_DEBUG_OUTPUT

            INCLUDE_PATH= ../../../install_pc/include
            LIB_PATH = ../../../install_pc/lib

            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp

            APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/platform
}

linux-gnueabi-oe-g++{
            message("amd64")
            DEFINES += I386
            #DEFINES += QT_NO_DEBUG_OUTPUT

            INCLUDE_PATH= ../../../install_arm/include
            LIB_PATH = ../../../install_arm/lib

            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp


#            INCLUDEPATH+= /home/hn-lpc-131/tools/breakpad_src_new/breakpad/src
#            LIBS +=-L/home/hn-lpc-131/tools/output_arm_new/lib -lbreakpad_client

            APP_INSTALL_PATH_PLATFORM = ../../../install_arm/bin/platform
}

target.path = $$APP_INSTALL_PATH_PLATFORM
INSTALLS += target
