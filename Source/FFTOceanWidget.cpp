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

#include "FFTOceanWidget.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::setupTweakBar()
{
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::initBufferObjects()
{
    assert(isValid());

    unsigned int M = mesh_resolution.x;
    unsigned int N = mesh_resolution.y;

    indexSize = (N - 1) * (M - 1) * 6;
    GLuint* indices = new GLuint[indexSize];

    int p = 0;

    for (int j = 0; j < N - 1; j++)
        for (int i = 0; i < M - 1; i++)
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(GLuint), indices,
                 GL_STATIC_DRAW);

    delete[] indices;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::deleteBufferObjects()
{
    assert(isValid());

    glDeleteVertexArrays(1, &surfaceVAO);
    glDeleteBuffers(1, &surfaceVBO);
    glDeleteBuffers(1, &surfaceEBO);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::buildTessendorfWaveMesh(float ftime)
{
    unsigned int M = mesh_resolution.x;
    unsigned int N = mesh_resolution.y;

    int nVertex = N * M;

    waveModel->buildHeightField(ftime);
    glm::vec3* heightField = waveModel->heightField;
    glm::vec3* normalField = waveModel->normalField;

    static heightfield_min_max min_max(heightField);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, M * N), min_max);

    heightMin = min_max.result_min;
    heightMax = min_max.result_max;

    ////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(surfaceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);

    int fieldArraySize = sizeof(glm::vec3) * nVertex;
    glBufferData(GL_ARRAY_BUFFER, fieldArraySize * 2, NULL, GL_STATIC_DRAW);

    // Copy height to buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, fieldArraySize, heightField);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Copy normal to buffer
    glBufferSubData(GL_ARRAY_BUFFER, fieldArraySize, fieldArraySize, normalField);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          (GLvoid*)fieldArraySize);
    glEnableVertexAttribArray(1);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::initializeGL()
{
    OpenGLWidget::initializeGL();
    lightingShader = new QtAppShaderProgram;
    lampShader = new QtAppShaderProgram;

#ifdef __APPLE__
    lightingShader->addVertexShaderFromFile("/Users/nghia/Programming/Mango/Projects/FFTOceanSimulation/Shaders/surface.vert");
    lightingShader->addFragmentShaderFromFile("/Users/nghia/Programming/Mango/Projects/FFTOceanSimulation/Shaders/surface.frag");
    lightingShader->link();

    lampShader->addVertexShaderFromFile("/Users/nghia/Programming/Mango/Projects/FFTOceanSimulation/Shaders/lamp.vert");
    lampShader->addFragmentShaderFromFile("/Users/nghia/Programming/Mango/Projects/FFTOceanSimulation/Shaders/lamp.frag");
    lampShader->link();
#else
    lightingShader->addVertexShaderFromFile("D:/Programming/QtApps/FFTOceanSimulation/Shaders/surface.vert");
    lightingShader->addFragmentShaderFromFile("D:/Programming/QtApps/FFTOceanSimulation/Shaders/surface.frag");
    lightingShader->link();
    lampShader->addVertexShaderFromFile("D:/Programming/QtApps/FFTOceanSimulation/Shaders/lamp.vert");
    lampShader->addFragmentShaderFromFile("D:/Programming/QtApps/FFTOceanSimulation/Shaders/lamp.frag");
    lampShader->link();
#endif

    GLfloat vertices[] =
    {
        // Positions          // Normals           // Texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // Light
    lampPos = sundir * 50.0f;
    GLuint VBO;

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid*)0); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    initBufferObjects();

    waveModel = new FFTWave(mesh_resolution, tile_size, wind_direction, wind_speed,
                            wave_amplitude, 1, 4);


    m_Camera->setDefaultCamera(glm::vec3(0.95, 4.35, 8.58),
                               glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::paintGL()
{
    m_FPSCounter.countFrame();
    startFrameTimer();

    ////////////////////////////////////////////////////////////////////////////////
    static float ftime = 0;
    ftime += timestep;
    buildTessendorfWaveMesh(ftime);

    ////////////////////////////////////////////////////////////////////////////////

//    qDebug() << m_Camera->cameraPosition.x << m_Camera->cameraPosition.y << m_Camera->cameraPosition.z;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    renderWave();
    renderLightSource();


    ////////////////////////////////////////////////////////////////////////////////

    endFrameTimer();
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::renderWave()
{
    lightingShader->bind();

    GLint lightPosLoc = glGetUniformLocation(lightingShader->getProgramID(), "light.position");
    GLint viewPosLoc = glGetUniformLocation(lightingShader->getProgramID(), "viewPos");
    glUniform3f(lightPosLoc, lampPos.x, lampPos.y, lampPos.z);
    glUniform3f(viewPosLoc, m_Camera->m_CameraPosition.x, m_Camera->m_CameraPosition.y,
                m_Camera->m_CameraPosition.z);
    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "heightMin"),
                heightMin * modelScale);
    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "heightMax"),
                heightMax * modelScale);

    // Set lights properties
    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.ambient"), 1.0f, 1.0f,
                1.0f);
    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.diffuse"), 1.0f, 1.0f,
                1.0f);
    glUniform3f(glGetUniformLocation(lightingShader->getProgramID(), "light.specular"), 1.0f, 0.9f,
                0.7f);
    // Set material properties
    glUniform1f(glGetUniformLocation(lightingShader->getProgramID(), "material.shininess"), 32.0f);

    // Create camera transformations
//    glm::mat4 view = glm::mat4(1.0);
    glm::mat4 view = m_Camera->getViewMatrix();
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
    model = glm::scale(model, glm::vec3(modelScale));	// Scale the surface
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTOceanWidget::renderLightSource()
{
    // ===== Draw Lamp =====

    // Also draw the lamp object, again binding the appropriate shader
    lampShader->bind();
    // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
    GLint modelLoc = glGetUniformLocation(lampShader->getProgramID(), "model");
    GLint viewLoc  = glGetUniformLocation(lampShader->getProgramID(), "view");
    GLint projLoc  = glGetUniformLocation(lampShader->getProgramID(), "projection");
    // Set matrices
    glm::mat4 view = m_Camera->getViewMatrix();
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