#-------------------------------------------------
#
# Project created by QtCreator 2013-01-10T08:26:47
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = MotionConsole
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    mainapplication.cpp \
    invalidargumentsexception.cpp

HEADERS += \
    mainapplication.h \
    invalidargumentsexception.h

macx: LIBS += -L$$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/ -lMotionCore

INCLUDEPATH += $$PWD/../MotionCore
DEPENDPATH += $$PWD/../MotionCore

macx: PRE_TARGETDEPS += $$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/libMotionCore.a
INCLUDEPATH += /usr/local/include/

unix|win32: LIBS += -lopencv_core
unix|win32: LIBS += -lopencv_highgui
unix|win32: LIBS += -lopencv_imgproc
unix|win32: LIBS += -lopencv_video
unix|win32: LIBS += -lopencv_features2d
