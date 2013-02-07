#-------------------------------------------------
#
# Project created by QtCreator 2013-01-04T21:25:47
#
#-------------------------------------------------

QT       += core gui

CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Motion
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    player.cpp \
    MatToQImage.cpp

HEADERS  += mainwindow.h \
    player.h \
    MatToQImage.h

FORMS    += mainwindow.ui


unix|macx: LIBS += -lopencv_core
unix|macx: LIBS += -lopencv_features2d
unix|macx: LIBS += -lopencv_highgui
unix|macx: LIBS += -lopencv_video
unix|macx: LIBS += -lopencv_imgproc

macx {
    INCLUDEPATH += /usr/local/include/

    # Link to Core Lib
    LIBS += -L$$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/ -lMotionCore
    INCLUDEPATH += $$PWD/../MotionCore
    DEPENDPATH += $$PWD/../MotionCore
    PRE_TARGETDEPS += $$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/libMotionCore.a
}


unix {
    INCLUDEPATH += /usr/include/

    # Link to OpenCV Using Package Manager
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv

    # Link to Core Lib
    LIBS += -L$$PWD/../MotionCore-build/ -lMotionCore
    INCLUDEPATH += $$PWD/../Core
    DEPENDPATH += $$PWD/../Core
    PRE_TARGETDEPS += $$PWD/../MotionCore-build/libMotionCore.a
}
