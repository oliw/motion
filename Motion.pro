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

INCLUDEPATH += /usr/local/include/
macx: LIBS += -lopencv_core
macx: LIBS += -lopencv_highgui
macx: LIBS += -lopencv_video
macx: LIBS += -lopencv_imgproc
macx: LIBS += -lopencv_features2d


macx: LIBS += -L$$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/ -lMotionCore

INCLUDEPATH += $$PWD/../MotionCore
DEPENDPATH += $$PWD/../MotionCore

macx: PRE_TARGETDEPS += $$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/libMotionCore.a
