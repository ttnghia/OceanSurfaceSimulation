#-------------------------------------------------
#
# Project created by QtCreator 2016-09-03T11:13:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OceanSimulation
TEMPLATE = app


SOURCES +=\
    Source/Main.cpp \
    Source/OceanRenderWidget.cpp \
    Source/FFTWave.cpp \
    Source/MainWindow.cpp \
    Source/Controller.cpp

HEADERS  += \
    Include/OceanRenderWidget.h \
    Include/FFTWave.h \
    Include/MainWindow.h \
    Include/Controller.h \
    Include/Common.h

include (../../Banana/BananaCore/BananaCore.pri)
include (../../Banana/QtAppHelpers/QtAppHelpers.pri)
include (../../Banana/OpenGLHelpers/OpenGLHelpers.pri);
include (../../Banana/ParallelHelpers/ParallelHelpers.pri)

INCLUDEPATH += $$PWD/Include
INCLUDEPATH += $$PWD/Externals/fftw-3.3.5/include

macx {
    LIBS += -lfftw3f_threads -lpthread -L$$PWD/Externals/fftw-3.3.5/lib
    LIBS += -lfftw3f -L$$PWD/Externals/fftw-3.3.5/lib
}
win32 {
    LIBS += -llibfftw3f-3 -L$$PWD/Externals/fftw-3.3.5/lib
}

RESOURCES += Shaders.qrc