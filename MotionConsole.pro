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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../MotionCore/release/ -lMotionCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../MotionCore/debug/ -lMotionCore
else:unix: LIBS += -L$$PWD/../MotionCore/ -lMotionCore

INCLUDEPATH += $$PWD/../MotionCore
DEPENDPATH += $$PWD/../MotionCore

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../MotionCore/release/MotionCore.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../MotionCore/debug/MotionCore.lib
else:unix: PRE_TARGETDEPS += $$PWD/../MotionCore/libMotionCore.a
