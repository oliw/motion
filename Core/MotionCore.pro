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
    l1model.cpp \
    evaluator.cpp \
    coreapplication.cpp \
    l1salientmodel.cpp

HEADERS += videoprocessor.h \
    video.h \
    frame.h \
    displacement.h \
    ransacmodel.h \
    tools.h \
    localransacrejector.h \
    l1model.h \
    evaluator.h \
    coreapplication.h \
    l1salientmodel.h

macx {
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
    LIBS += -L/Applications/MATLAB_R2012b.app/bin/maci64 -leng -lmx -lmat -lm
    QMAKE_RPATHDIR += /Applications/MATLAB_R2012b.app/bin/maci64

    # COIN LP Lib
    LIBS += -L$$PWD/../coin-Clp/lib/ -lOsi -lCoinUtils -lOsiClp -lClp
    INCLUDEPATH += $$PWD/../coin-Clp/include
    DEPENDPATH += $$PWD/../coin-Clp/include

}

unix:!macx: {

#    QMAKE_CC = gcc-4.4
#    QMAKE_CXX = g++-4.4
#    QMAKE_LINK = g++-4.4
#    QMAKE_EXTRA_COMPILERS += fortran

    #LIBS += -lgfortran

    CONFIG += link_pkgconfig

    # MATLAB Engine Library
    LIBS += -L/usr/lib/matlab/R2012a/bin/glnxa64 -lm -lmat -lmx -leng -lstdc++

    QMAKE_RPATHDIR += /usr/lib/matlab/R2012a/bin/glnxa64
    INCLUDEPATH += /usr/lib/matlab/R2012a/extern/include
    DEPENDPATH += /usr/lib/matlab/R2012a/extern/include

    # OPENCV Library
    PKGCONFIG += opencv
    INCLUDEPATH += /vol/bitbucket/osw09/opencv-build-2.4.0/include

    # OPENCV Library
    #INCLUDEPATH += /vol/bitbucket/osw09/opencv-build/include
    #LIBS += -L/vol/bitbucket/osw09/opencv-build/lib -lopencv_core -lopencv_highgui -lopencv_video -lopencv_imgproc -lopencv_features2d

    # COIN LP
    PKGCONFIG += osi-clp
    INCLUDEPATH += $$PWD/../Clp-1.14.8/include
    DEPENDPATH += $$PWD/../Clp-1.14.8/include
}
