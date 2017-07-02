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

#include <time.h>
#include "FFTWave.h"


#include <iostream>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
FFTWave::FFTWave(float tileSize, int waveResolution, const Vec2f& windDirection, float windSpeed, float lambda) :
    m_TileSize(tileSize),
    m_WaveResolution(waveResolution),
    m_WindDirection(glm::normalize(windDirection)),
    m_WinSpeed(windSpeed),
    m_Lambda(lambda)
{
    static_assert(sizeof(std::complex<float> ) == sizeof(fftwf_complex), "Incompatible type: sizeof(float) != sizeof(fftwf_complex");
    m_RandGenerator.seed(static_cast<unsigned int>(time(NULL)));

    setWaveResolution(waveResolution);
    initFFTW();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//Eq19
void FFTWave::buildHeightField(float time)
{
    ////////////////////////////////////////////////////////////////////////////////
    // => compute data
    tbb::parallel_for(tbb::blocked_range<int>(0, m_WaveResolution), [&](tbb::blocked_range<int> r)
                      {
                          for(int n = r.begin(), nEnd = r.end(); n != nEnd; ++n)
                          {
                              for(int m = 0; m < m_WaveResolution; ++m)
                              {
                                  auto index = m * m_WaveResolution + n;
                                  auto h_twiddle = func_h_twiddle(n, m, time);
                                  auto kVec = compute_kvec(n, m);
                                  auto kLength2 = glm::length2(kVec);
                                  auto kVecNormalized = kLength2 == 0 ? kVec : glm::normalize(kVec);

                                  m_FFTWData.value_h_twiddle[index] = h_twiddle;
                                  m_FFTWData.slope_x_term[index] = std::complex<float>(0, kVec.x) * h_twiddle;
                                  m_FFTWData.slope_z_term[index] = std::complex<float>(0, kVec.y) * h_twiddle;
                                  m_FFTWData.D_x_term[index] = std::complex<float>(0, -kVecNormalized.x) * h_twiddle;
                                  m_FFTWData.D_z_term[index] = std::complex<float>(0, -kVecNormalized.y) * h_twiddle;
                              }
                          }
                      });

    ////////////////////////////////////////////////////////////////////////////////
    // => execute plans
    fftwf_execute(m_FFTWData.p_height);
    fftwf_execute(m_FFTWData.p_slope_x);
    fftwf_execute(m_FFTWData.p_slope_z);
    fftwf_execute(m_FFTWData.p_D_x);
    fftwf_execute(m_FFTWData.p_D_z);

    ////////////////////////////////////////////////////////////////////////////////
    // => get out data
    tbb::parallel_for(tbb::blocked_range<int>(0, m_WaveResolution), [&](tbb::blocked_range<int> r)
                      {
                          for(int n = r.begin(); n != r.end(); ++n)
                          {
                              for(int m = 0; m < m_WaveResolution; ++m)
                              {
                                  int index = m * m_WaveResolution + n;

                                  // Flip the sign
                                  float sign = ((m + n) & 1) ? -1.0 : 1.0;

                                  glm::vec3 normal(sign * m_FFTWData.out_slope_x[index][0],
                                                   1,
                                                   sign * m_FFTWData.out_slope_z[index][0]);
                                  m_NormalField[index] = normal; // glm::normalize(normal);

                                  m_HeightField[index] = glm::vec3(
                                      (static_cast<float>(n) - static_cast<float>(m_WaveResolution) / 2.0f) * m_TileSize / static_cast<float>(m_WaveResolution) - sign * m_Lambda * m_FFTWData.out_D_x[index][0],
                                      sign * m_FFTWData.out_height[index][0],
                                      (static_cast<float>(m) - static_cast<float>(m_WaveResolution) / 2.0f) * m_TileSize / static_cast<float>(m_WaveResolution) - sign * m_Lambda * m_FFTWData.out_D_z[index][0]);
                              }
                          }
                      });
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::getHeightFieldMinMax(float& minValue, float& maxValue)
{
    HeightFieldMinMax hfMM(m_HeightField);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, m_HeightField.size()), hfMM);

    minValue = hfMM.result_min;
    maxValue = hfMM.result_max;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::setWaveResolution(int waveResolution)
{
    m_WaveResolution = waveResolution;
    m_kNum           = waveResolution * waveResolution;

    m_HeightField.resize(m_kNum);
    m_NormalField.resize(m_kNum);

    if(m_FFTWData.initialize)
    {
        deallocateFFTWMemory();
        allocateFFTWMemory();

        destroyFFTWPlans();
        createFFTWPlans();
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::setTileSize(float tileSize)
{
    m_TileSize = tileSize;
    init_value_h_twiddle_0();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::setWinSpeed(float winSpeed)
{
    m_WinSpeed = winSpeed;
    init_value_h_twiddle_0();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::setNumThreads(int numThreads)
{
    m_NumThreads = numThreads;
    m_TBBinit.terminate();
    m_TBBinit.initialize(m_NumThreads);

    destroyFFTWPlans();
    createFFTWPlans();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::allocateFFTWMemory()
{
    size_t arraySize = sizeof(fftwf_complex) * m_kNum;
    m_FFTWData.value_h_twiddle_0      = (std::complex<float>*)fftwf_malloc(arraySize);
    m_FFTWData.value_h_twiddle_0_conj = (std::complex<float>*)fftwf_malloc(arraySize);
    m_FFTWData.value_h_twiddle        = (std::complex<float>*)fftwf_malloc(arraySize);

    m_FFTWData.slope_x_term = (std::complex<float>*)fftwf_malloc(arraySize);
    m_FFTWData.slope_z_term = (std::complex<float>*)fftwf_malloc(arraySize);

    m_FFTWData.D_x_term = (std::complex<float>*)fftwf_malloc(arraySize);
    m_FFTWData.D_z_term = (std::complex<float>*)fftwf_malloc(arraySize);


    m_FFTWData.out_height  = (fftwf_complex*)fftwf_malloc(arraySize);
    m_FFTWData.out_slope_x = (fftwf_complex*)fftwf_malloc(arraySize);
    m_FFTWData.out_slope_z = (fftwf_complex*)fftwf_malloc(arraySize);
    m_FFTWData.out_D_x     = (fftwf_complex*)fftwf_malloc(arraySize);
    m_FFTWData.out_D_z     = (fftwf_complex*)fftwf_malloc(arraySize);

    m_FFTWData.in_height  = (fftwf_complex*)m_FFTWData.value_h_twiddle;
    m_FFTWData.in_slope_x = (fftwf_complex*)m_FFTWData.slope_x_term;
    m_FFTWData.in_slope_z = (fftwf_complex*)m_FFTWData.slope_z_term;
    m_FFTWData.in_D_x     = (fftwf_complex*)m_FFTWData.D_x_term;
    m_FFTWData.in_D_z     = (fftwf_complex*)m_FFTWData.D_z_term;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::deallocateFFTWMemory()
{
    fftwf_free(m_FFTWData.value_h_twiddle_0);
    fftwf_free(m_FFTWData.value_h_twiddle_0_conj);
    fftwf_free(m_FFTWData.value_h_twiddle);

    fftwf_free(m_FFTWData.slope_x_term);
    fftwf_free(m_FFTWData.slope_z_term);

    fftwf_free(m_FFTWData.D_x_term);
    fftwf_free(m_FFTWData.D_z_term);

    fftwf_free(m_FFTWData.out_height);
    fftwf_free(m_FFTWData.out_slope_x);
    fftwf_free(m_FFTWData.out_slope_z);
    fftwf_free(m_FFTWData.out_D_x);
    fftwf_free(m_FFTWData.out_D_z);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::createFFTWPlans()
{
    fftwf_plan_with_nthreads(m_NumThreads);


    m_FFTWData.p_height  = fftwf_plan_dft_2d(m_WaveResolution, m_WaveResolution, m_FFTWData.in_height, m_FFTWData.out_height, FFTW_BACKWARD, FFTW_ESTIMATE);
    m_FFTWData.p_slope_x = fftwf_plan_dft_2d(m_WaveResolution, m_WaveResolution, m_FFTWData.in_slope_x, m_FFTWData.out_slope_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    m_FFTWData.p_slope_z = fftwf_plan_dft_2d(m_WaveResolution, m_WaveResolution, m_FFTWData.in_slope_z, m_FFTWData.out_slope_z, FFTW_BACKWARD, FFTW_ESTIMATE);
    m_FFTWData.p_D_x     = fftwf_plan_dft_2d(m_WaveResolution, m_WaveResolution, m_FFTWData.in_D_x, m_FFTWData.out_D_x, FFTW_BACKWARD, FFTW_ESTIMATE);
    m_FFTWData.p_D_z     = fftwf_plan_dft_2d(m_WaveResolution, m_WaveResolution, m_FFTWData.in_D_z, m_FFTWData.out_D_z, FFTW_BACKWARD, FFTW_ESTIMATE);

    init_value_h_twiddle_0();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::destroyFFTWPlans()
{
    fftwf_destroy_plan(m_FFTWData.p_height);
    fftwf_destroy_plan(m_FFTWData.p_slope_x);
    fftwf_destroy_plan(m_FFTWData.p_slope_z);
    fftwf_destroy_plan(m_FFTWData.p_D_x);
    fftwf_destroy_plan(m_FFTWData.p_D_z);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::initFFTW()
{
    fftwf_init_threads();

    allocateFFTWMemory();
    createFFTWPlans();

    m_FFTWData.initialize = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::shutdownFFTW()
{
    fftwf_cleanup();
    fftwf_cleanup_threads();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Get k vector from mesh grid (m,n)
inline Vec2f FFTWave::compute_kvec(int m, int n) const
{
    return Vec2f(2 * M_PI * (static_cast<float>(m) - static_cast<float>(m_WaveResolution) / 2.0f) / m_TileSize,
                 2 * M_PI * (static_cast<float>(n) - static_cast<float>(m_WaveResolution) / 2.0f) / m_TileSize);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Eq14
inline float FFTWave::func_omega(float k) const
{
    return sqrtf(GRAVITY * k);
}

#include <QDebug>
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Eq23 Phillips spectrum
inline float FFTWave::func_P_h(const Vec2f& vec_k) const
{
    if(vec_k == Vec2f(0.0f, 0.0f))
    {
        return 0.0f;
    }

    float L = m_WinSpeed * m_WinSpeed / GRAVITY; // Largest possible waves arising from a continuous wind of speed V

    float k     = glm::length(vec_k);
    Vec2f k_hat = glm::normalize(vec_k);

    float dot_k_hat_omega_hat = glm::dot(k_hat, m_WindDirection);
    float result              = WAVE_AMPLITUDE * expf(-1 / (k * L * k * L)) / pow(k, 4) * dot_k_hat_omega_hat * dot_k_hat_omega_hat;

    result *= expf(-k * k * WAVE_LENGTH * WAVE_LENGTH); // Eq24

    return result;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FFTWave::init_value_h_twiddle_0()
{
    ////////////////////////////////////////////////////////////////////////////////
    // Initialize value_h_twiddle_0 and value_h_twiddle_0_conj in Eq26
    tbb::parallel_for(tbb::blocked_range<int>(0, m_WaveResolution), [&](tbb::blocked_range<int> r)
                      {
                          for(int n = r.begin(), nEnd = r.end(); n != nEnd; ++n)
                          {
                              for(int m = 0; m < m_WaveResolution; ++m)
                              {
                                  int index = m * m_WaveResolution + n;
                                  Vec2f k = compute_kvec(m, n);

                                  m_FFTWData.value_h_twiddle_0[index] = func_h_twiddle_0(k);
                                  m_FFTWData.value_h_twiddle_0_conj[index] = std::conj(func_h_twiddle_0(k));
                              }
                          }
                      });
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Eq25
inline std::complex<float> FFTWave::func_h_twiddle_0(const Vec2f& vec_k)
{
    float xi_r = m_NormalDistribution(m_RandGenerator);
    float xi_i = m_NormalDistribution(m_RandGenerator);
    return sqrtf(0.5f) * std::complex<float>(xi_r, xi_i) * sqrtf(func_P_h(vec_k));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Eq26
inline std::complex<float> FFTWave::func_h_twiddle(int kn, int km, float t) const
{
    int                 index = km * m_WaveResolution + kn;
    float               k     = glm::length(compute_kvec(kn, km));
    std::complex<float> term1 = m_FFTWData.value_h_twiddle_0[index] * exp(std::complex<float>(0.0f, func_omega(k) * t));
    std::complex<float> term2 = m_FFTWData.value_h_twiddle_0_conj[index] * exp(std::complex<float>(0.0f, -func_omega(k) * t));
    return term1 + term2;
}
