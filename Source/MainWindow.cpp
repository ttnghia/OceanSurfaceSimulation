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

#include "MainWindow.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
MainWindow::MainWindow(QWidget* parent) : OpenGLMainWindow(parent)
{
    instantiateOpenGLWidget();
    setupRenderWidgets();
    connectWidgets();
    setArthurStyle();
    setWindowTitle("Ocean Simulation using Fast Fourier Transform");
    setFocusPolicy(Qt::StrongFocus);
    showCameraPosition(false);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::instantiateOpenGLWidget()
{
    if(m_GLWidget != nullptr)
    {
        delete m_GLWidget;
    }

    m_RenderWidget = new OceanRenderWidget(this);
    m_WaveModel    = m_RenderWidget->getWaveModel();
    setupOpenglWidget(m_RenderWidget);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool MainWindow::processKeyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_Space:
            m_Controller->m_btnPause->click();
            return true;

        ////////////////////////////////////////////////////////////////////////////////
        default:
            return OpenGLMainWindow::processKeyPressEvent(event);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupRenderWidgets()
{
    m_Controller = new Controller(this);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_GLWidget);
    mainLayout->addWidget(m_Controller);

    QWidget* mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::connectWidgets()
{
    ////////////////////////////////////////////////////////////////////////////////
    // textures
    connect(m_Controller->m_cbSkyTexture->getComboBox(), SIGNAL(currentIndexChanged(int)),                m_RenderWidget, SLOT(setSkyBoxTexture(int)));
    connect(m_Controller->m_btnReloadTextures,           SIGNAL(clicked(bool)),                           m_RenderWidget, SLOT(reloadTextures()));
    connect(m_Controller->m_msSurfaceMaterial,           SIGNAL(materialChanged(Material::MaterialData)), m_RenderWidget, SLOT(setSurfaceMaterial(Material::MaterialData)));

    ////////////////////////////////////////////////////////////////////////////////
    // wave
    connect(m_Controller->m_sldWaveResolution->getSlider(), &QSlider::valueChanged, m_RenderWidget,    &OceanRenderWidget::setWaveResolution);
    connect(m_Controller->m_sldTileSize->getSlider(),       &QSlider::valueChanged, m_WaveModel.get(), &FFTWave::setTileSize);
    connect(m_Controller->m_sldWindSpeed->getSlider(),      &QSlider::valueChanged, m_WaveModel.get(), &FFTWave::setWinSpeed);
    connect(m_Controller->m_sldTimeStep->getSlider(),       &QSlider::valueChanged, m_RenderWidget,    &OceanRenderWidget::setTimeStep);
    connect(m_Controller->m_sldNumThreads->getSlider(),     &QSlider::valueChanged, m_WaveModel.get(), &FFTWave::setNumThreads);

    ////////////////////////////////////////////////////////////////////////////////
    // lights
    connect(m_Controller->m_LightEditor, &PointLightEditor::lightsChanged,     m_RenderWidget,              &OceanRenderWidget::updateLights);
    connect(m_RenderWidget,              &OceanRenderWidget::lightsObjChanged, m_Controller->m_LightEditor, &PointLightEditor::setLights);

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    connect(m_Controller->m_btnPause, &QPushButton::clicked, m_RenderWidget, &OceanRenderWidget::tooglePause);
}