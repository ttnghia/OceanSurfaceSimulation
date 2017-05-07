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
#include <QDebug>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::initRDataWave()
{
    assert(isValid());

    unsigned int M = m_WaveResolution;
    unsigned int N = m_WaveResolution;
    indexSize = (N - 1) * (M - 1) * 6;
    GLuint* indices = new GLuint[indexSize];

    unsigned int p = 0;

    for(unsigned int j = 0; j < N - 1; j++)
        for(unsigned int i = 0; i < M - 1; i++)
        {
            indices[p++] = i + j * N;
            indices[p++] = (i + 1) + j * N;
            indices[p++] = i + (j + 1) * N;

            indices[p++] = (i + 1) + j * N;
            indices[p++] = (i + 1) + (j + 1) * N;
            indices[p++] = i + (j + 1) * N;
        }

    // Element buffer object
    glGenVertexArrays(1, &surfaceVAO);
    glBindVertexArray(surfaceVAO);
    glGenBuffers(1, &surfaceVBO);
    glGenBuffers(1, &surfaceEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surfaceEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(GLuint), indices, GL_STATIC_DRAW);

    delete[] indices;
}

#include <QDebug>
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::updateWave(float ftime)
{
    unsigned int M = m_WaveResolution;
    unsigned int N = m_WaveResolution;

    unsigned int nVertex = N * M;

    m_WaveModel->buildHeightField(ftime);
    auto heightField = m_WaveModel->getHeightField();
    auto normalField = m_WaveModel->getNormalField();

    m_WaveModel->getHeightFieldMinMax(m_HeightMin, m_HeightMax);

    ////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(surfaceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);

    unsigned int fieldArraySize = sizeof(glm::vec3) * nVertex;
    glBufferData(GL_ARRAY_BUFFER, fieldArraySize * 2, NULL, GL_STATIC_DRAW);

    // Copy height to buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, fieldArraySize, heightField.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Copy normal to buffer
    glBufferSubData(GL_ARRAY_BUFFER, fieldArraySize, fieldArraySize, normalField.data());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
            (GLvoid*)fieldArraySize);
    glEnableVertexAttribArray(1);
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
const std::shared_ptr<FFTWave>& OceanRenderWidget::getWaveModel()
{
    return m_WaveModel;
}

void OceanRenderWidget::setSkyBoxTexture(int texIndex)
{
    Q_ASSERT(m_SkyBoxRender != nullptr);
    m_SkyBoxRender->setRenderTextureIndex(texIndex);
}

void OceanRenderWidget::setWaveResolution(int resolution)
{
    m_WaveResolution = resolution;
}

void OceanRenderWidget::setTimeStep(int timeStep)
{
    m_TimeStep = static_cast<float>(timeStep) / 100.0f;
}

void OceanRenderWidget::initOpenGL()
{
    lightingShader = new QtAppShaderProgram;
    lampShader     = new QtAppShaderProgram;

#ifdef __APPLE__
    lightingShader->addVertexShaderFromFile("/Users/nghia/Programming/Mango/Projects/OceanSimulation/Shaders/surface.vert");
    lightingShader->addFragmentShaderFromFile("/Users/nghia/Programming/Mango/Projects/OceanSimulation/Shaders/surface.frag");
    lightingShader->link();

    lampShader->addVertexShaderFromFile("/Users/nghia/Programming/Mango/Projects/OceanSimulation/Shaders/lamp.vert");
    lampShader->addFragmentShaderFromFile("/Users/nghia/Programming/Mango/Projects/OceanSimulation/Shaders/lamp.frag");
    lampShader->link();
#else
    lightingShader->addVertexShaderFromFile("D:/Programming/QtApps/OceanSimulation/Shaders/surface.vert");
    lightingShader->addFragmentShaderFromFile("D:/Programming/QtApps/OceanSimulation/Shaders/surface.frag");
    lightingShader->link();
    lampShader->addVertexShaderFromFile("D:/Programming/QtApps/OceanSimulation/Shaders/lamp.vert");
    lampShader->addFragmentShaderFromFile("D:/Programming/QtApps/OceanSimulation/Shaders/lamp.frag");
    lampShader->link();
#endif

    GLfloat vertices[] =
    {
        // Positions          // Normals           // Texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        0.5f,  -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
        0.5f,   0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        0.5f,   0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
        0.5f,  -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
        0.5f,   0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        0.5f,   0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        0.5f,   0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        0.5f,   0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        0.5f,  -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        0.5f,  -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        0.5f,  -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        0.5f,   0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        0.5f,  -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        0.5f,  -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        0.5f,  -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        0.5f,   0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        0.5f,   0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        0.5f,   0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
    };

    // Light
    lampPos = m_SunDirection * 50.0f;
    GLuint VBO;

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid*)0);               // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    initRDataWave();


    m_Camera->setDefaultCamera(glm::vec3(0.95, 4.35, 8.58), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::renderOpenGL()
{
    m_FPSCounter.countFrame();

    ////////////////////////////////////////////////////////////////////////////////
    static float ftime = 0;
    ftime += m_TimeStep;
    updateWave(ftime);

    ////////////////////////////////////////////////////////////////////////////////

//    qDebug() << m_Camera->cameraPosition.x << m_Camera->cameraPosition.y << m_Camera->cameraPosition.z;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    renderWave();
    renderLightSource();


    ////////////////////////////////////////////////////////////////////////////////

    //    endFrameTimer();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::initRDataLight()
{
    m_Lights = std::make_shared<PointLights>();
    m_Lights->setNumLights(1);

    m_Lights->setLightPosition(glm::vec4(0, 100, -100, 1.0), 0);
    m_Lights->setLightPosition(glm::vec4(0, 100, 100, 1.0),  1);
    m_Lights->setLightDiffuse(glm::vec4(0.7), 0);
    m_Lights->setLightDiffuse(glm::vec4(0.7), 1);

    m_Lights->setSceneCenter(glm::vec3(0, 0, 0));
    m_Lights->setLightViewPerspective(30);
    m_Lights->uploadDataToGPU();

    m_LightRender = std::make_unique<PointLightRender>(m_Camera, m_Lights, m_UBufferCamData);
//    emit lightsObjChanged(m_Lights);
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
void OceanRenderWidget::renderWave()
{
    lightingShader->bind();

    GLint lightPosLoc = glGetUniformLocation(lightingShader->getProgramID(), "light.position");
    GLint viewPosLoc  = glGetUniformLocation(lightingShader->getProgramID(), "viewPos");
    glUniform3f(lightPosLoc, lampPos.x,                       lampPos.y,                       lampPos.z);
    glUniform3f(viewPosLoc,  m_Camera->getCameraPosition().x, m_Camera->getCameraPosition().y, m_Camera->getCameraPosition().z);
    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "heightMin"),
            m_HeightMin * m_ModelScale);
    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "heightMax"),
            m_HeightMax * m_ModelScale);

    // Set lights properties
    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.ambient"),  1.0f, 1.0f,
            1.0f);
    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.diffuse"),  1.0f, 1.0f,
            1.0f);
    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.specular"), 1.0f, 0.9f,
            0.7f);
    // Set material properties
    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "material.shininess"), 32.0f);

    // Create camera transformations
//    glm::mat4 view = glm::mat4(1.0);
    glm::mat4 view       = m_Camera->getViewMatrix();
    glm::mat4 projection = m_Camera->getProjectionMatrix();
    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(lightingShader->getProgramID(), "model");
    GLint viewLoc  = glGetUniformLocation(lightingShader->getProgramID(), "view");
    GLint projLoc  = glGetUniformLocation(lightingShader->getProgramID(), "projection");
    // Pass the matrices to the shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // ===== Draw Model =====
    glBindVertexArray(surfaceVAO);
    glm::mat4 model;
    model = glm::mat4();
    model = glm::scale(model, glm::vec3(m_ModelScale));   // Scale the surface
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void OceanRenderWidget::renderLightSource()
{
    // ===== Draw Lamp =====

    // Also draw the lamp object, again binding the appropriate shader
    lampShader->bind();
    // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
    GLint modelLoc = glGetUniformLocation(lampShader->getProgramID(), "model");
    GLint viewLoc  = glGetUniformLocation(lampShader->getProgramID(), "view");
    GLint projLoc  = glGetUniformLocation(lampShader->getProgramID(), "projection");
    // Set matrices
    glm::mat4 view       = m_Camera->getViewMatrix();
    glm::mat4 projection = m_Camera->getProjectionMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glm::mat4 model = glm::mat4();
    model = glm::translate(model, lampPos);
    model = glm::scale(model, glm::vec3(1.0f)); // Make it a smaller cube
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Draw the light object (using light's vertex attributes)
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}