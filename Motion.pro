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

# OPENCV Library
INCLUDEPATH += /usr/local/include/
macx: LIBS += -lopencv_core
macx: LIBS += -lopencv_highgui
macx: LIBS += -lopencv_video
macx: LIBS += -lopencv_imgproc
macx: LIBS += -lopencv_features2d

# MATLAB Engine Library
INCLUDEPATH += /Applications/MATLAB_R2012b.app/extern/include
DEPENDPATH += /Applications/MATLAB_R2012b.app/extern/include
macx: LIBS += -L/Applications/MATLAB_R2012b.app/bin/maci64 -leng -lmx
QMAKE_RPATHDIR += /Applications/MATLAB_R2012b.app/bin/maci64

# Motion Backend Library (Static)
INCLUDEPATH += $$PWD/../MotionCore
DEPENDPATH += $$PWD/../MotionCore
macx: LIBS += -L$$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/ -lMotionCore
macx: PRE_TARGETDEPS += $$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/libMotionCore.a


