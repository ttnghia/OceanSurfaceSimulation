//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//  Copyright (c) 2017 by
//       __      _     _         _____
//    /\ \ \__ _| |__ (_) __ _  /__   \_ __ _   _  ___  _ __   __ _
//   /  \/ / _` | '_ \| |/ _` |   / /\/ '__| | | |/ _ \| '_ \ / _` |
//  / /\  / (_| | | | | | (_| |  / /  | |  | |_| | (_) | | | | (_| |
//  \_\ \/ \__, |_| |_|_|\__,_|  \/   |_|   \__,_|\___/|_| |_|\__, |
//         |___/                                              |___/
//
//  <nghiatruong.vn@gmail.com>
//  All rights reserved.
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#include <OpenGLHelpers/OpenGLTexture.h>

#include "Common.h"
#include "Controller.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupGUI()
{
    QVBoxLayout* controlLayout = new QVBoxLayout;
    QVBoxLayout* btnLayout     = new QVBoxLayout;

    setupTextureControllers(controlLayout);
    setupColorControllers(controlLayout);
    setupWaveControllers(controlLayout);
    controlLayout->addStretch();
    setupButtons(btnLayout);

    ////////////////////////////////////////////////////////////////////////////////
    m_LightEditor = new PointLightEditor(nullptr, this);
    QWidget* mainControls = new QWidget;
    mainControls->setLayout(controlLayout);

    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->setTabPosition(QTabWidget::South);
    tabWidget->setTabShape(QTabWidget::Triangular);
    tabWidget->addTab(mainControls,  "Main Controls");
    tabWidget->addTab(m_LightEditor, "Lights");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);

    ////////////////////////////////////////////////////////////////////////////////
    setFixedWidth(300);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::loadTextures()
{
    int currentSkyTexID = m_cbSkyTexture->getComboBox()->currentIndex();
    m_cbSkyTexture->getComboBox()->clear();
    m_cbSkyTexture->getComboBox()->addItem("None");
    QStringList skyTexFolders = OpenGLTexture::getTextureFolders("Sky");

    foreach(const QString &tex, skyTexFolders)
    {
        m_cbSkyTexture->getComboBox()->addItem(tex);
    }

    m_cbSkyTexture->getComboBox()->setCurrentIndex(currentSkyTexID > 0 ? currentSkyTexID : 0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupTextureControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    // sky textures
    m_cbSkyTexture = new EnhancedComboBox;
    ctrLayout->addWidget(m_cbSkyTexture->getGroupBox("Sky Texture"));
    loadTextures();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupColorControllers(QBoxLayout* ctrLayout)
{
    m_msSurfaceMaterial = new MaterialSelector;
    m_msSurfaceMaterial->setCustomMaterial(DEFAULT_SURFACE_MATERIAL);
    m_msSurfaceMaterial->setDefaultCustomMaterial(true);

    QGroupBox* materialGroup = new QGroupBox("Surface Material");
    materialGroup->setLayout(m_msSurfaceMaterial->getLayout());
    ctrLayout->addWidget(materialGroup);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupWaveControllers(QBoxLayout* ctrLayout)
{
    m_sldWaveResolution = new EnhancedSlider;
    m_sldWaveResolution->setRange(1, 4096);
    m_sldWaveResolution->setValue(DEFAULT_WAVE_RESOLUTION);
    m_sldWaveResolution->setTracking(false);

    QGroupBox* waveResolutionGroup = new QGroupBox("Wave Resolution");
    waveResolutionGroup->setLayout(m_sldWaveResolution->getLayout());

    ////////////////////////////////////////////////////////////////////////////////
    m_sldTileSize = new EnhancedSlider;
    m_sldTileSize->setRange(1, 10000);
    m_sldTileSize->setValue(DEFAULT_TILE_SIZE);
    m_sldTileSize->setTracking(false);

    QGroupBox* tileSizeGroup = new QGroupBox("Tile Size");
    tileSizeGroup->setLayout(m_sldTileSize->getLayout());

    ////////////////////////////////////////////////////////////////////////////////
    m_sldTimeStep = new EnhancedSlider;
    m_sldTimeStep->setRange(0, 100);
    m_sldTimeStep->setValue(static_cast<int>(DEFAULT_TIMESTEP * 100));
    m_sldTimeStep->setTracking(false);
    QGridLayout* timestepLayout = new QGridLayout;
    timestepLayout->addLayout(m_sldTimeStep->getLayout(), 0, 0, 1, 5);
    timestepLayout->addWidget(new QLabel("/ 100"), 0, 5, 1, 1);

    QGroupBox* timestepGroup = new QGroupBox("Timestep");
    timestepGroup->setLayout(timestepLayout);

    ////////////////////////////////////////////////////////////////////////////////
    m_sldNumThreads = new EnhancedSlider;
    m_sldNumThreads->setRange(0, 16);
    m_sldNumThreads->setValue(DEFAULT_NUM_THREADS);
    m_sldNumThreads->setTracking(false);

    QGroupBox* numThreadGroup = new QGroupBox("Num. Threads");
    numThreadGroup->setLayout(m_sldNumThreads->getLayout());

    ////////////////////////////////////////////////////////////////////////////////
    ctrLayout->addWidget(waveResolutionGroup);
    ctrLayout->addWidget(tileSizeGroup);
    ctrLayout->addWidget(timestepGroup);
    ctrLayout->addWidget(numThreadGroup);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupButtons(QBoxLayout* ctrLayout)
{
    m_btnReloadTextures = new QPushButton("Reload Textures");
    m_btnPause          = new QPushButton(QString("Pause"));
    m_btnPause->setCheckable(true);

    QGridLayout* btnLayout = new QGridLayout;
    btnLayout->addWidget(m_btnReloadTextures, 0, 0, 1, 1);
    btnLayout->addWidget(m_btnPause,          0, 1, 1, 1);

    ctrLayout->addLayout(btnLayout);
}
