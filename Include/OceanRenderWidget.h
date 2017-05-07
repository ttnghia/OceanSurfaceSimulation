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

#include <tbb/tbb.h>

#include <QtApphelpers/OpenGLWidget.h>
#include <OpenGLHelpers/OpenGLBuffer.h>
#include <OpenGLHelpers/OpenGLTexture.h>
#include <OpenGLHelpers/RenderObjects.h>
#include <QtApphelpers/QtAppShaderProgram.h>

#include "Common.h"
#include "FFTWave.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class OceanRenderWidget : public OpenGLWidget
{
    Q_OBJECT

public:
    OceanRenderWidget(QWidget* parent = 0) : OpenGLWidget(parent)
    {}

    ~OceanRenderWidget() = default;

    const std::shared_ptr<FFTWave>& getWaveModel();

public slots:
    void setSkyBoxTexture(int texIndex);
    void setWaveResolution(int resolution);
    void setTimeStep(int timeStep);

protected:
    virtual void initOpenGL() override;
    virtual void renderOpenGL() override;
    virtual void resizeOpenGLWindow(int, int) override
    {}

private:
    ////////////////////////////////////////////////////////////////////////////////
    void initRDataLight();
    void renderLight();

    ////////////////////////////////////////////////////////////////////////////////
    void initRDataSkyBox();
    void renderSkyBox();

    ////////////////////////////////////////////////////////////////////////////////
    struct
    {
        std::shared_ptr<QtAppShaderProgram> shader;

        bool initialized = false;
    } m_RDataWave;
    void updateWave(float ftime);
    void initRDataWave();
    void renderWave();
    void renderLightSource();


    ////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<FFTWave> m_WaveModel = std::make_shared<FFTWave>(DEFAULT_TILE_SIZE, DEFAULT_WAVE_RESOLUTION, DEFAULT_WAVE_AMPLITUDE, DEFAULT_WIN_DIRECTION, DEFAULT_WIN_SPEED, 1.0f, 4);

    QtAppShaderProgram* lightingShader;
    QtAppShaderProgram* lampShader;

    int   m_WaveResolution = DEFAULT_WAVE_RESOLUTION;
    float m_TimeStep       = DEFAULT_TIMESTEP;

    ////////////////////////////////////////////////////////////////////////////////
    // => rendering variable
    int indexSize;

    GLuint surfaceVAO;
    GLuint lightVAO;

    GLuint surfaceVBO, surfaceEBO;

    float m_ModelScale = 0.01f;
    float m_HeightMax;
    float m_HeightMin;


    // Light attributes
    glm::vec3 lampPos;
    glm::vec3 m_SunDirection = DEFAULT_SUN_DIRECTION;
    glm::vec3 m_SunColor     = DEFAULT_SUN_COLOR;
    glm::vec3 m_SeaColor     = DEFAULT_SEA_COLOR;

    std::shared_ptr<PointLights>      m_Lights;
    std::unique_ptr<SkyBoxRender>     m_SkyBoxRender = nullptr;
    std::unique_ptr<PointLightRender> m_LightRender  = nullptr;
};

