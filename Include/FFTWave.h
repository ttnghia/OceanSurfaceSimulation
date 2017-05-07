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

#include <glm/glm.hpp>
#include <fftw3.h>
#include <tbb/tbb.h>

#include <QObject>

#include <random>
#include <complex>
#include <memory>

#include "Common.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FFTWave : public QObject
{
    Q_OBJECT
public:
    // mesh resolution: resolution of the height field
    // tile resolution:  the actual size of the grid (in meters)
    FFTWave(float tileSize, int waveResolution, float waveAmplitude, const Vec2f& windDirection, float windSpeed, float lambda, int numthreads);
    ~FFTWave()
    {
        shutdownFFTW();
    }

    void buildHeightField(float time);

    const std::vector<glm::vec3>& getHeightField()
    {
        return m_HeightField;
    }

    const std::vector<glm::vec3>& getNormalField()
    {
        return m_NormalField;
    }

    void getHeightFieldMinMax(float& minValue, float& maxValue);

public slots:
    void setWaveResolution(int waveResolution);
    void setTileResolution(float tileResolution);
    void setWinSpeed(float winSpeed);
    void setWaveAmplitude(float waveAmplitude);
    void setNumThreads(int numThreads);

private:
    void allocateFFTWMemory();
    void deallocateFFTWMemory();
    void createFFTWPlans();
    void destroyFFTWPlans();
    void initFFTW();
    void shutdownFFTW();

    inline Vec2f               compute_kvec(int m, int n) const;
    inline float               func_omega(float k) const;
    inline float               func_P_h(const Vec2f& vec_k) const;
    inline std::complex<float> func_h_twiddle_0(const Vec2f& vec_k);
    inline std::complex<float> func_h_twiddle(int kn, int km, float t) const;


    ////////////////////////////////////////////////////////////////////////////////
    std::vector<glm::vec3> m_HeightField;
    std::vector<glm::vec3> m_NormalField;
    int                    m_NumThreads;

    float m_TileSize;
    int   m_WaveResolution;
    float m_WaveAmplitude;
    int   m_kNum;
    Vec2f m_WindDirection;
    float m_WinSpeed;
    float m_Lambda;

    std::default_random_engine      m_RandGenerator;
    std::normal_distribution<float> m_NormalDistribution;

    ////////////////////////////////////////////////////////////////////////////////
    // => fftw variables
    struct
    {
        std::complex<float>* value_h_twiddle_0;
        std::complex<float>* value_h_twiddle_0_conj;
        std::complex<float>* value_h_twiddle;

        // Eq20 ikh_twiddle
        std::complex<float>* slope_x_term;
        std::complex<float>* slope_z_term;

        // Eq29
        std::complex<float>* D_x_term;
        std::complex<float>* D_z_term;

        fftwf_complex* in_height, * in_slope_x, * in_slope_z, * in_D_x, * in_D_z;
        fftwf_complex* out_height, * out_slope_x, * out_slope_z, * out_D_x, * out_D_z;

        fftwf_plan p_height, p_slope_x, p_slope_z, p_D_x, p_D_z;
    } m_FFTWData;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class HeightFieldMinMax
{
private:
    const std::vector<glm::vec3> v;

public:
    float result_min;
    float result_max;

    HeightFieldMinMax(const std::vector<glm::vec3>& vec) : v(vec), result_min(1e10), result_max(-1e10) {}

    HeightFieldMinMax(HeightFieldMinMax& vdp, tbb::split) : v(vdp.v), result_min(1e10), result_max(-1e10) {}

    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(), iEnd = r.end(); i != iEnd; i++)
        {
            result_min = result_min < v[i].y ? result_min : v[i].y;
            result_max = result_max > v[i].y ? result_max : v[i].y;
        }
    }

    void join(HeightFieldMinMax& hfmm)
    {
        result_min = result_min < hfmm.result_min ? result_min : hfmm.result_min;
        result_max = result_max > hfmm.result_max ? result_max : hfmm.result_max;
    }
};