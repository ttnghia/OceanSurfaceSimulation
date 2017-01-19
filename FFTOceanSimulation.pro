#-------------------------------------------------
#
# Project created by QtCreator 2016-09-03T11:13:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FFTOceanSimulation
TEMPLATE = app


SOURCES +=\
    Main.cpp \
    FFTOceanWidget.cpp \
    FFTWave.cpp \
    FFTOMainWindow.cpp

HEADERS  += \
    FFTOceanWidget.h \
    FFTWave.h \
    FFTOMainWindow.h


include (../../Banana/QtAppHelpers/QtAppHelpers.pri)
include (../../Banana/OpenGLHelpers/OpenGLHelpers.pri)

INCLUDEPATH += $$PWD/Externals/fftw-3.3.5/include

macx {
    INCLUDEPATH += $$PWD/../../Banana/Externals/tbb_osx/include
    LIBS += -ltbb -L$$PWD/../../Banana/Externals/tbb_osx/lib

    LIBS += -lfftw3f_threads -lpthread -L$$PWD/Externals/fftw-3.3.5/lib
    LIBS += -lfftw3f -L$$PWD/Externals/fftw-3.3.5/lib
}
win32 {
    INCLUDEPATH += $$PWD/../../Banana/Externals/tbb_win/include
    LIBS += -ltbb -L$$PWD/../../Banana/Externals/tbb_win/lib/intel64/vc14

    LIBS += -llibfftw3f-3 -L$$PWD/Externals/fftw-3.3.5/lib
}

# Add this to path variable D:\Programming\Libs\tbb44_win\bin\intel64\vc14

RESOURCES += \
    shaders.qrc

#CONFIG += warn_off
