QT       -= gui

TARGET = a7uartdriver
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++0x


TEMPLATE = app
#DEFINES += QT_NO_DEBUG_OUTPUT
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += PLATFORM_TYPE        #pc与arm版条件编译
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    timer/timer.cpp \
    uart/uartdrive.cpp \
    uart/uartbus.cpp \
    crc/crc_ccitt.cpp \
    key/key.cpp \
    key/uinput_controller.cpp \
    ir/ir.cpp \
    wirless/wirless.cpp\
    main.cpp \


HEADERS += \
    timer/timer.h \
    uart/uartdrive.h \
    uart/uartbus.h \
    mutex/mutex.h \
    sem/sem.h \
    key/key.h \
    key/uinput_controller.h \
    uart/UartDataType.h \
    crc/crc_ccitt.h \
    wirless/wirless.h\
    ir/ir.h \


 linux-gnueabi-oe-g++ {
            message("a7 arm")
            #DEFINES += I386
            #DEFINES += QT_NO_DEBUG_OUTPUT
            INCLUDE_PATH= $$PWD/../../../install_arm/include
            LIB_PATH = $$PWD/../../../install_arm/lib
            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp

            APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/platform
            # 定义BREAKPAD宏
            DEFINES += BREAKPAD
            # 根据BREAKPAD宏来编译选项
            contains(DEFINES, BREAKPAD) {
                # 编译选项
            INCLUDEPATH += /home/hnlpc141/tools/breakpad_src_new/breakpad/src
            LIBS +=-L/home/hnlpc141/tools/output_arm_new/lib -lbreakpad_client
            }
}
linux-g++-64 {
            message("amd64")
            DEFINES += I386
            #DEFINES += QT_NO_DEBUG_OUTPUT

            INCLUDE_PATH= $$PWD/../../../install_pc/include
            LIB_PATH = $$PWD/../../../install_pc/lib
            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp

            APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/platform
}

linux-g++ {
            message("amd64")
            DEFINES += I386
            #DEFINES += QT_NO_DEBUG_OUTPUT
            INCLUDE_PATH= $$PWD/../../../install_pc/include
            LIB_PATH = $$PWD/../../../install_pc/lib
            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp

            APP_INSTALL_PATH_PLATFORM = ../../../install_pc/bin/platform
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
