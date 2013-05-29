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
    MatToQImage.cpp \
    cropwindowdialog.cpp \
    subselectimage.cpp \
    manualmotiontracker.cpp \
    selectpointimage.cpp

HEADERS  += mainwindow.h \
    player.h \
    MatToQImage.h \
    cropwindowdialog.h \
    subselectimage.h \
    manualmotiontracker.h \
    selectpointimage.h

FORMS    += mainwindow.ui \
    cropwindowdialog.ui \
    manualmotiontracker.ui

macx {

    # Application Icon
    ICON = Resources/icons.icns

    # OPENCV Library
    INCLUDEPATH += /usr/local/include/
    LIBS += -lopencv_core
    LIBS += -lopencv_highgui
    LIBS += -lopencv_video
    LIBS += -lopencv_imgproc
    LIBS += -lopencv_features2d

    # MATLAB Engine Library
    INCLUDEPATH += /Applications/MATLAB_R2012b.app/extern/include
    DEPENDPATH += /Applications/MATLAB_R2012b.app/extern/include
    LIBS += -L/Applications/MATLAB_R2012b.app/bin/maci64 -leng -lmx -lmat
    QMAKE_RPATHDIR += /Applications/MATLAB_R2012b.app/bin/maci64

    # COIN LP Lib
    LIBS += -L$$PWD/../coin-Clp/lib/ -lOsi -lCoinUtils -lOsiClp -lClp
    INCLUDEPATH += $$PWD/../coin-Clp/include
    DEPENDPATH += $$PWD/../coin-Clp/include

    # Motion Backend Library (Static)
    INCLUDEPATH += $$PWD/../Core
    DEPENDPATH += $$PWD/../Core
    LIBS += -L$$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/ -lMotionCore
    PRE_TARGETDEPS += $$PWD/../MotionCore-build-Desktop_Qt_5_0_0_clang_64bit_SDK-Debug/libMotionCore.a
}

unix:!macx: {

    QMAKE_CC = gcc-4.4
    QMAKE_CXX = g++-4.4
    QMAKE_LINK = g++-4.4
    QMAKE_EXTRA_COMPILERS += fortran

   # LIBS += -lgfortran
    CONFIG += link_pkgconfig

    # OPENCV Library
    PKGCONFIG += opencv
    INCLUDEPATH += /vol/bitbucket/osw09/opencv-build-2.4.0/include

    # MATLAB Engine Library
    LIBS += -L/usr/lib/matlab/R2012a/bin/glnxa64 -lm -lmat -lmx -leng

    #QMAKE_RPATHDIR += /usr/lib/matlab/R2012a/bin/glnxa64
    INCLUDEPATH += /usr/lib/matlab/R2012a/extern/include
    DEPENDPATH += /usr/lib/matlab/R2012a/extern/include

    # COIN LP
    PKGCONFIG += osi-clp
    INCLUDEPATH += /homes/osw09/Motion/Clp-1.14.8/include
    DEPENDPATH += /homes/osw09/Motion/Clp-1.14.8/include

    # Link to Core Lib
    LIBS += -L$$PWD/../build/ -lMotionCore
    INCLUDEPATH += $$PWD/../Core
    DEPENDPATH += $$PWD/../Core
    PRE_TARGETDEPS += $$PWD/../build/libMotionCore.a
}
