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

#pragma once

#include <QtAppHelpers/MaterialSelector.h>
#include <QtAppHelpers/EnhancedSlider.h>
#include <QtAppHelpers/PointLightEditor.h>
#include <QtAppHelpers/EnhancedComboBox.h>

#include <QtWidgets>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class Controller : public QWidget
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit Controller(QWidget* parent) : QWidget(parent)
    {
        setupGUI();
    }

public slots:
    void loadTextures();

private:
    void setupGUI();
    void setupTextureControllers(QBoxLayout* ctrLayout);
    void setupColorControllers(QBoxLayout* ctrLayout);
    void setupWaveControllers(QBoxLayout* ctrLayout);
    void setupButtons(QBoxLayout* ctrLayout);

    ////////////////////////////////////////////////////////////////////////////////
    EnhancedComboBox* m_cbSkyTexture;
    MaterialSelector* m_msSurfaceMaterial;
    QPushButton*      m_btnReloadTextures;
    EnhancedSlider*   m_sldWaveResolution;
    EnhancedSlider*   m_sldTileSize;
    EnhancedSlider*   m_sldWindSpeed;
    EnhancedSlider*   m_sldTimeStep;
    EnhancedSlider*   m_sldNumThreads;

    QPushButton*      m_btnPause;
    PointLightEditor* m_LightEditor;
};