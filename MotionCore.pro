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
    tools.cpp \
    localransacrejector.cpp \
    graphdrawer.cpp \
    l1model.cpp

HEADERS += videoprocessor.h \
    video.h \
    frame.h \
    displacement.h \
    ransacmodel.h \
    tools.h \
    localransacrejector.h \
    graphdrawer.h \
    l1model.h

INCLUDEPATH += /usr/local/include/

unix|win32: LIBS += -lopencv_core
unix|win32: LIBS += -lopencv_highgui
unix|win32: LIBS += -lopencv_imgproc
unix|win32: LIBS += -lopencv_video
unix|win32: LIBS += -lopencv_features2d

macx: LIBS += -L/Applications/MATLAB_R2012b.app/bin/maci64/ -leng
macx: LIBS += -L/Applications/MATLAB_R2012b.app/bin/maci64/ -lmx
INCLUDEPATH += /Applications/MATLAB_R2012b.app/extern/include
DEPENDPATH += /Applications/MATLAB_R2012b.app/extern/include

macx: LIBS += -L$$PWD/../coin-Clp/lib/ -lClp
INCLUDEPATH += $$PWD/../coin-Clp/include
DEPENDPATH += $$PWD/../coin-Clp/include
