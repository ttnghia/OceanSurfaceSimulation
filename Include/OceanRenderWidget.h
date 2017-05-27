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

#include <QtApphelpers/OpenGLWidget.h>
#include <OpenGLHelpers/OpenGLBuffer.h>
#include <OpenGLHelpers/OpenGLTexture.h>
#include <OpenGLHelpers/RenderObjects.h>
#include <OpenGLHelpers/MeshObject.h>
#include <OpenGLHelpers/Material.h>
#include <QtApphelpers/QtAppShaderProgram.h>

#include "Common.h"
#include "FFTWave.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class OceanRenderWidget : public OpenGLWidget
{
    Q_OBJECT

public:
    OceanRenderWidget(QWidget* parent = 0);
    const std::shared_ptr<FFTWave>& getWaveModel();

public slots:
    void setSkyBoxTexture(int texIndex);
    void reloadTextures();
    void setWaveResolution(int resolution);
    void setTimeStep(int timeStep);
    void updateLights();
    void tooglePause();
    void setSurfaceMaterial(const Material::MaterialData& material);

signals:
    void lightsObjChanged(const std::shared_ptr<PointLights>& lights);

protected:
    virtual void initOpenGL() override;
    virtual void resizeOpenGLWindow(int, int) override {}
    virtual void renderOpenGL() override;

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
        std::shared_ptr<QtAppShaderProgram> shader          = nullptr;
        std::shared_ptr<MeshObject>         surfaceMesh     = nullptr;
        std::shared_ptr<Material>           surfaceMaterial = nullptr;
        std::unique_ptr<MeshRender>         surfaceRender   = nullptr;

        bool initialized = false;
    } m_RDataWave;

    void updateWave(float ftime);
    void initRDataWave();
    void renderWave();

    ////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<FFTWave> m_WaveModel = std::make_shared<FFTWave>(DEFAULT_TILE_SIZE, DEFAULT_WAVE_RESOLUTION, DEFAULT_WAVE_AMPLITUDE, DEFAULT_WIN_DIRECTION, DEFAULT_WIN_SPEED, 1.0f);

    QtAppShaderProgram* lightingShader;

    int   m_WaveResolution = DEFAULT_WAVE_RESOLUTION;
    float m_TimeStep       = DEFAULT_TIMESTEP;
    bool  m_bPause         = false;

    ////////////////////////////////////////////////////////////////////////////////
    // => rendering variable
    int indexSize;

    float m_ModelScale = 0.01f;
    float m_HeightMax;
    float m_HeightMin;


    // Light attributes
    glm::vec3 m_SunDirection = DEFAULT_SUN_DIRECTION;
    glm::vec3 lampPos        = m_SunDirection * 50.0f;
    glm::vec3 m_SunColor     = DEFAULT_SUN_COLOR;
    glm::vec3 m_SeaColor     = DEFAULT_SEA_COLOR;

    std::shared_ptr<PointLights>      m_Lights;
    std::unique_ptr<SkyBoxRender>     m_SkyBoxRender = nullptr;
    std::unique_ptr<PointLightRender> m_LightRender  = nullptr;
};

