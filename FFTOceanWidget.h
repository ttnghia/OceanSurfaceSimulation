//------------------------------------------------------------------------------------------
//            .-..-.
//           (-o/\o-)
//          /`""``""`\
//          \ /.__.\ /
//           \ `--` /                                                 Created on: 9/17/2016
//            `)  ('                                                    Author: Nghia Truong
//         ,  /::::\  ,
//         |'.\::::/.'|
//        _|  ';::;'  |_
//       (::)   ||   (::)                       _.
//        "|    ||    |"                      _(:)
//         '.   ||   .'                       /::\
//           '._||_.'                         \::/
//            /::::\                         /:::\
//            \::::/                        _\:::/
//             /::::\_.._  _.._  _.._  _.._/::::\
//             \::::/::::\/::::\/::::\/::::\::::/
//               `""`\::::/\::::/\::::/\::::/`""`
//                    `""`  `""`  `""`  `""`
//------------------------------------------------------------------------------------------
#pragma once

#include <tbb/tbb.h>

#include <QtApphelpers/OpenGLWidget.h>
#include <QtApphelpers/QtAppShader.h>

#include "FFTWave.h"

//------------------------------------------------------------------------------------------
class FFTOceanWidget : public OpenGLWidget
{
    Q_OBJECT

public:
    FFTOceanWidget(QWidget* parent = 0):
        OpenGLWidget(parent),
        mesh_resolution(Resolution2D(512, 512)),
//        mesh_resolution(Resolution2D(256, 256)),
//        mesh_resolution(Resolution2D(128, 128)),
//        mesh_resolution(Resolution2D(64, 64)),
//        mesh_resolution(Resolution2D(16, 16)),
        tile_size(glm::vec2(1000, 1000)),
        wind_speed(1000),
        wind_direction(glm::vec2(1, 1)),
        timestep(0.2),
        wave_amplitude(1e-6),
        sundir(glm::normalize(glm::vec3(0, 1, -2))),
        sun_color(1.0f, 1.0f, 1.0f),
        sea_color(0.0f, 0.0, 0.5)
    {
//        defaultSize = QSize(1500, 1500);
    }
    ~FFTOceanWidget()
    {
        makeCurrent();
        deleteBufferObjects();
    }

    void set_wave_mesh_resolution(Resolution2D resolution)
    {
        mesh_resolution = resolution;
    }

    // QOpenGLWidget interface
protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;

    // AntTweakBarWrapper interface
protected:
    virtual void setupTweakBar() override;

private:
    void initBufferObjects();
    void deleteBufferObjects();
    void buildTessendorfWaveMesh(float ftime);

    void renderWave();
    void renderLightSource();



    FFTWave* waveModel;

    QtAppShader* lightingShader;
    QtAppShader* lampShader;

    Resolution2D mesh_resolution;
    glm::vec2 tile_size;

    float wave_amplitude;
    float wind_speed = 30;
    glm::vec2 wind_direction;

    float timestep;

    ////////////////////////////////////////////////////////////////////////////////
    // => rendering variable
    int indexSize;

    GLuint surfaceVAO;
    GLuint lightVAO;

    GLuint surfaceVBO, surfaceEBO;

    float modelScale = 0.01;
    float heightMax;
    float heightMin;


    // Light attributes
    glm::vec3 lampPos;
    glm::vec3 sundir;
    glm::vec3 sun_color;
    glm::vec3 sea_color;

};

//------------------------------------------------------------------------------------------
class heightfield_min_max
{
    const glm::vec3* v;
public:
    float result_min;  // put the result here
    float result_max;  // put the result here

    // constructor copies the arguments into local storage
    heightfield_min_max(const glm::vec3* vec)
//        : v(vec), result_min(1e100), result_max(-1e100) { }
        : v(vec), result_min(0), result_max(0) { }

    // splitting constructor
    heightfield_min_max(heightfield_min_max& vdp, tbb::split)
        : v(vdp.v), result_min(0), result_max(0) { }

    // overload () so it does finding max
    void operator() (const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); i++)
        {
            result_min = result_min < v[i].y ? result_min : v[i].y;
            result_max = result_max > v[i].y ? result_max : v[i].y;
        }
    }

    void join(heightfield_min_max& v)
    {
        result_min = result_min < v.result_min ? result_min : v.result_min;
        result_max = result_max > v.result_max ? result_max : v.result_max;
    }
};