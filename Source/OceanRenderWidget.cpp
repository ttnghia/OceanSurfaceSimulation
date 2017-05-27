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

#include "OceanRenderWidget.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
OceanRenderWidget::OceanRenderWidget(QWidget* parent) : OpenGLWidget(parent)
{
    m_Camera->setDefaultCamera(glm::vec3(0.95, 4.35, 8.58), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
const std::shared_ptr<FFTWave>& OceanRenderWidget::getWaveModel()
{
    return m_WaveModel;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::setSkyBoxTexture(int texIndex)
{
    Q_ASSERT(m_SkyBoxRender != nullptr);
    m_SkyBoxRender->setRenderTextureIndex(texIndex);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::reloadTextures()
{
    makeCurrent();
    m_SkyBoxRender->clearTextures();
    m_SkyBoxRender->loadTextures(QDir::currentPath() + "/Textures/Sky/");
    doneCurrent();

    ////////////////////////////////////////////////////////////////////////////////
    QMessageBox::information(this, "Info", "Textures reloaded!");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::setWaveResolution(int resolution)
{
    m_WaveResolution = resolution;

    ////////////////////////////////////////////////////////////////////////////////
    unsigned int M = m_WaveResolution;
    unsigned int N = m_WaveResolution;
    indexSize = (N - 1) * (M - 1) * 6;
    unsigned int p = 0;

    std::vector<GLuint> indices(indexSize);

    for(unsigned int j = 0; j < N - 1; j++)
    {
        for(unsigned int i = 0; i < M - 1; i++)
        {
            indices[p++] = i + j * N;
            indices[p++] = (i + 1) + j * N;
            indices[p++] = i + (j + 1) * N;

            indices[p++] = (i + 1) + j * N;
            indices[p++] = (i + 1) + (j + 1) * N;
            indices[p++] = i + (j + 1) * N;
        }
    }

    assert(m_RDataWave.surfaceMesh != nullptr);
    m_RDataWave.surfaceMesh->setElementIndex(indices);
    m_WaveModel->setWaveResolution(resolution);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::setTimeStep(int timeStep)
{
    m_TimeStep = static_cast<float>(timeStep) / 1000.0f;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::updateLights()
{
    makeCurrent();
    m_Lights->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::tooglePause()
{
    m_bPause = !m_bPause;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::setSurfaceMaterial(const Material::MaterialData& material)
{
    makeCurrent();
    m_RDataWave.surfaceMaterial->setMaterial(material);
    m_RDataWave.surfaceMaterial->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::initOpenGL()
{
    initRDataSkyBox();
    initRDataLight();
    initRDataWave();

//    lightingShader = new QtAppShaderProgram;

//    lightingShader->addVertexShaderFromResource(":/Shaders/surface.vert");
//    lightingShader->addFragmentShaderFromResource(":/Shaders/surface.frag");
//    lightingShader->link();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::renderOpenGL()
{
    renderSkyBox();
    renderLight();

    ////////////////////////////////////////////////////////////////////////////////
    static float ftime = 0;
    if(!m_bPause)
    {
        ftime += m_TimeStep;
        updateWave(ftime);
    }
    renderWave();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::initRDataLight()
{
    m_Lights = std::make_shared<PointLights>();
    m_Lights->setNumLights(1);

    m_Lights->setLightPosition(glm::vec4(0, 100, -100, 1.0), 0);
    m_Lights->setLightDiffuse(glm::vec4(0.7), 0);

    m_Lights->setSceneCenter(glm::vec3(0, 0, 0));
    m_Lights->setLightViewPerspective(30);
    m_Lights->uploadDataToGPU();

    m_LightRender = std::make_unique<PointLightRender>(m_Camera, m_Lights, m_UBufferCamData);
    emit lightsObjChanged(m_Lights);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::renderLight()
{
    Q_ASSERT(m_LightRender != nullptr);
    m_LightRender->render();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::initRDataSkyBox()
{
    Q_ASSERT(m_UBufferCamData != nullptr);

    m_SkyBoxRender = std::make_unique<SkyBoxRender>(m_Camera, QDir::currentPath() + "/Textures/Sky/", m_UBufferCamData);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::renderSkyBox()
{
    Q_ASSERT(m_SkyBoxRender != nullptr);
    m_SkyBoxRender->render();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::initRDataWave()
{
    assert(isValid());

    unsigned int M = m_WaveResolution;
    unsigned int N = m_WaveResolution;
    indexSize = (N - 1) * (M - 1) * 6;
    unsigned int p = 0;

    std::vector<GLfloat> vertices(indexSize);
    std::vector<GLfloat> normals(indexSize);
    std::vector<GLuint>  indices(indexSize);

    for(int i = 0; i < m_WaveResolution * m_WaveResolution; ++i)
    {
        vertices[i] = 0;
        normals[i]  = 0;
    }

    for(unsigned int j = 0; j < N - 1; j++)
    {
        for(unsigned int i = 0; i < M - 1; i++)
        {
            indices[p++] = i + j * N;
            indices[p++] = (i + 1) + j * N;
            indices[p++] = i + (j + 1) * N;

            indices[p++] = (i + 1) + j * N;
            indices[p++] = (i + 1) + (j + 1) * N;
            indices[p++] = i + (j + 1) * N;
        }
    }

    m_RDataWave.surfaceMesh = std::make_shared<MeshObject>();
    m_RDataWave.surfaceMesh->setVertices(vertices);
    m_RDataWave.surfaceMesh->setVertexNormal(normals);
    m_RDataWave.surfaceMesh->setElementIndex(indices);

    m_RDataWave.surfaceMaterial = std::make_shared<Material>();
    m_RDataWave.surfaceMaterial->setMaterial(DEFAULT_SURFACE_MATERIAL);
    m_RDataWave.surfaceMaterial->uploadDataToGPU();

    m_RDataWave.surfaceRender = std::make_unique<MeshRender>(m_RDataWave.surfaceMesh, m_Camera, m_Lights, m_RDataWave.surfaceMaterial, m_UBufferCamData);
    m_RDataWave.surfaceRender->transform(glm::vec3(0), glm::vec3(m_ModelScale));

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataWave.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::updateWave(float ftime)
{
    m_WaveModel->buildHeightField(ftime);
    auto heightField = m_WaveModel->getHeightField();
    auto normalField = m_WaveModel->getNormalField();

    m_WaveModel->getHeightFieldMinMax(m_HeightMin, m_HeightMax);
    m_RDataWave.surfaceMesh->setVertices((void*)heightField.data(), heightField.size() * sizeof(glm::vec3));
    m_RDataWave.surfaceMesh->setVertexNormal((void*)normalField.data(), normalField.size() * sizeof(glm::vec3));
    m_RDataWave.surfaceMesh->uploadDataToGPU();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::renderWave()
{
    assert(m_RDataWave.initialized);
    m_RDataWave.surfaceRender->render();


//    GLint lightPosLoc = glGetUniformLocation(lightingShader->getProgramID(), "light.position");
//    GLint viewPosLoc  = glGetUniformLocation(lightingShader->getProgramID(), "viewPos");
//    glUniform3f(lightPosLoc, lampPos.x,                       lampPos.y,                       lampPos.z);
//    glUniform3f(viewPosLoc,  m_Camera->getCameraPosition().x, m_Camera->getCameraPosition().y, m_Camera->getCameraPosition().z);
//    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "heightMin"),
//            m_HeightMin * m_ModelScale);
//    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "heightMax"),
//            m_HeightMax * m_ModelScale);

//    // Set lights properties
//    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.ambient"),  1.0f, 1.0f,
//            1.0f);
//    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.diffuse"),  1.0f, 1.0f,
//            1.0f);
//    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.specular"), 1.0f, 0.9f,
//            0.7f);
//    // Set material properties
//    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "material.shininess"), 32.0f);

//    // Create camera transformations
////    glm::mat4 view = glm::mat4(1.0);
//    glm::mat4 view       = m_Camera->getViewMatrix();
//    glm::mat4 projection = m_Camera->getProjectionMatrix();
//    // Get the uniform locations
//    GLint modelLoc = glGetUniformLocation(lightingShader->getProgramID(), "model");
//    GLint viewLoc  = glGetUniformLocation(lightingShader->getProgramID(), "view");
//    GLint projLoc  = glGetUniformLocation(lightingShader->getProgramID(), "projection");
//    // Pass the matrices to the shader
//    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

//    // ===== Draw Model =====
//    glBindVertexArray(surfaceVAO);
//    glm::mat4 model;
//    model = glm::mat4();
//    model = glm::scale(model, glm::vec3(m_ModelScale));   // Scale the surface
//    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
}
