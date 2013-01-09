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
    videoprocessor.cpp \
    video.cpp \
    frame.cpp \
    player.cpp \
    MatToQImage.cpp

INCLUDEPATH += /usr/local/include/

HEADERS  += mainwindow.h \
    videoprocessor.h \
    video.h \
    frame.h \
    player.h \
    MatToQImage.h

FORMS    += mainwindow.ui

macx: LIBS += -lopencv_core
macx: LIBS += -lopencv_highgui
macx: LIBS += -lopencv_video
macx: LIBS += -lopencv_imgproc
