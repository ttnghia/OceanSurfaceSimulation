//------------------------------------------------------------------------------------------
// Main.cpp
//
// Created on: 9/3/2016
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------
#include <QApplication>
#include <QSurfaceFormat>
#include <QStyle>
#include <QDesktopWidget>

#include <tbb/tbb.h>

#include <QtAppHelpers/QtAppMacros.h>
#include "FFTOMainWindow.h"

//------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);
//        tbb::task_scheduler_init init(1);

    __BNNQt_RunMainWindow(FFTOMainWindow, argc, argv);
}