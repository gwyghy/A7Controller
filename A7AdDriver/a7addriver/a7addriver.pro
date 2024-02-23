QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

linux-g++-64 {
    INCLUDEPATH += $$PWD/../../../install_pc/include/A7DriverApp
    LIBS += -L$$PWD/../../../install_pc/lib -lA7DriverApp
}

linux-g++ {
    INCLUDEPATH += $$PWD/../../../install_pc/include/A7DriverApp
    LIBS += -L$$PWD/../../../install_pc/lib -lA7DriverApp
}

#linux-gnueabi-oe-g++
#{
#    INCLUDEPATH += $$PWD/../../../install_arm/include/A7DriverApp
#    LIBS += -L$$PWD/../../../install_arm/lib -lA7DriverApp
#}

linux-gnueabi-oe-g++ {
            message("amd64")
            DEFINES += I386
            #DEFINES += QT_NO_DEBUG_OUTPUT

            QMAKE_LFLAGS += "-Wl,-rpath=../../lib"

            INCLUDE_PATH= $$PWD/../../../install_arm/include
            LIB_PATH = $$PWD/../../../install_arm/lib

            INCLUDEPATH += $$INCLUDE_PATH/A7DriverApp/
            LIBS +=-L$$LIB_PATH/ -lA7DriverApp

            APP_INSTALL_PATH_PLATFORM = ../../../install_arm/bin/platform
}
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        adc/adc.cpp \
        analog/analog.cpp \
        analog/libmcp3204.cpp \
        main.cpp \


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    adc/adc.h \
    analog/analog.h \
    analog/libmcp3204.h \
    include.h \

