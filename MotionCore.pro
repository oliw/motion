#-------------------------------------------------
#
# Project created by QtCreator 2013-01-09T23:46:51
#
#-------------------------------------------------

QT       -= gui

TARGET = MotionCore
TEMPLATE = lib
CONFIG += staticlib

SOURCES += videoprocessor.cpp \
    video.cpp \
    frame.cpp \
    displacement.cpp \
    ransacmodel.cpp \
    tools.cpp

HEADERS += videoprocessor.h \
    video.h \
    frame.h \
    displacement.h \
    ransacmodel.h \
    tools.h

INCLUDEPATH += /usr/local/include/

unix|win32: LIBS += -lopencv_core
unix|win32: LIBS += -lopencv_highgui
unix|win32: LIBS += -lopencv_imgproc
unix|win32: LIBS += -lopencv_video
unix|win32: LIBS += -lopencv_features2d
