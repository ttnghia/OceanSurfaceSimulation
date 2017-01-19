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
#include <time.h>
#include <tbb/tbb.h>
#include "FFTWave.h"


#include <iostream>

//------------------------------------------------------------------------------------------
FFTWave::FFTWave(Resolution2D _mesh_resolution, glm::vec2 _tile_size,
                 glm::vec2 _wind_direction, float _wind_speed, float _wave_amplitude,
                 float lambda, int _compute_threads) :
    mesh_resolution(_mesh_resolution),
    wind_direction(glm::normalize(_wind_direction)),
    wind_speed(_wind_speed),
    tile_size(_tile_size),
    wave_amplitude(_wave_amplitude),
    lambda(lambda),
    num_threads(_compute_threads)
{
    generator.seed(time(NULL));
    kNum = mesh_resolution.x * mesh_resolution.y;

    heightField = new glm::vec3[kNum];
    normalField = new glm::vec3[kNum];

    assert(sizeof(std::complex<float>) == sizeof(fftwf_complex));
    init_fftw_data();

    ////////////////////////////////////////////////////////////////////////////////
    // Initialize value_h_twiddle_0 and value_h_twiddle_0_conj in Eq26
    tbb::parallel_for(tbb::blocked_range<int>(0, mesh_resolution.x),
                      [&](tbb::blocked_range<int> r)
    {
        for(int n = r.begin(); n != r.end(); ++n)
        {
            for (int m = 0; m < mesh_resolution.y; ++m)
            {
                int index = m * mesh_resolution.x + n;
                glm::vec2 k = compute_kvec(m, n);
                value_h_twiddle_0[index] = func_h_twiddle_0(k);
                value_h_twiddle_0_conj[index] = std::conj(func_h_twiddle_0(k));
            }
        }
    });


}

//------------------------------------------------------------------------------------------
FFTWave::~FFTWave()
{
    delete[] heightField;
    delete[] normalField;
}

//------------------------------------------------------------------------------------------
// Eq14
inline float FFTWave::func_omega(float k) const
{
    return sqrt(g * k);
}

//------------------------------------------------------------------------------------------
// Eq23 Phillips spectrum
inline float FFTWave::func_P_h(glm::vec2 vec_k) const
{
    if (vec_k == glm::vec2(0.0f, 0.0f))
    {
        return 0.0f;
    }

    float L = wind_speed * wind_speed /
              g; // Largest possible waves arising from a continuous wind of speed V

    float k = length(vec_k);
    glm::vec2 k_hat = glm::normalize(vec_k);

    float dot_k_hat_omega_hat = glm::dot(k_hat, wind_direction);
    float result = wave_amplitude * exp(-1 / (k * L * k * L)) / pow(k,
                                                                    4) * pow(dot_k_hat_omega_hat, 2);

    result *= exp(-k * k * l * l); // Eq24

    return result;
}

//------------------------------------------------------------------------------------------
// Eq25
inline std::complex<float> FFTWave::func_h_twiddle_0(glm::vec2 vec_k)
{
    float xi_r = normal_dist(generator);
    float xi_i = normal_dist(generator);
    return sqrtf(0.5f) * std::complex<float>(xi_r, xi_i) * sqrtf(func_P_h(vec_k));
}

//------------------------------------------------------------------------------------------
// Eq26
inline std::complex<float> FFTWave::func_h_twiddle(int kn, int km, float t) const
{
    int index = km * mesh_resolution.x + kn;
    float k = length(compute_kvec(kn, km));
    std::complex<float> term1 = value_h_twiddle_0[index] * exp(std::complex<float>(0.0f,
                                                                                   func_omega(k) * t));
    std::complex<float> term2 = value_h_twiddle_0_conj[index] * exp(std::complex<float>(0.0f,
                                                                                        -func_omega(k) * t));
    return term1 + term2;
}

//------------------------------------------------------------------------------------------
//Eq19
void FFTWave::buildHeightField(float time)
{
    ////////////////////////////////////////////////////////////////////////////////
    // => compute data
    tbb::parallel_for(tbb::blocked_range<int>(0, mesh_resolution.x),
                      [&](tbb::blocked_range<int> r)
    {
        for(int n = r.begin(); n != r.end(); ++n)
        {
            for (int m = 0; m < mesh_resolution.y; m++)
            {
                int index = m * mesh_resolution.x + n;

                value_h_twiddle[index] = func_h_twiddle(n, m, time);

                glm::vec2 kVec = compute_kvec(n, m);
                float kLength = glm::length(kVec);
                glm::vec2 kVecNormalized = kLength == 0 ? kVec : glm::normalize(kVec);

                slope_x_term[index] = std::complex<float>(0, kVec.x) * value_h_twiddle[index];
                slope_z_term[index] = std::complex<float>(0, kVec.y) * value_h_twiddle[index];
                D_x_term[index] = std::complex<float>(0, -kVecNormalized.x) * value_h_twiddle[index];
                D_z_term[index] = std::complex<float>(0, -kVecNormalized.y) * value_h_twiddle[index];
            }
        }
    });


    ////////////////////////////////////////////////////////////////////////////////
    // => execute plans
    fftwf_execute(p_height);
    fftwf_execute(p_slope_x);
    fftwf_execute(p_slope_z);
    fftwf_execute(p_D_x);
    fftwf_execute(p_D_z);

    ////////////////////////////////////////////////////////////////////////////////
    // => get out data
    tbb::parallel_for(tbb::blocked_range<int>(0, mesh_resolution.x),
                      [&](tbb::blocked_range<int> r)
    {
        for(int n = r.begin(); n != r.end(); ++n)
        {
            for (int m = 0; m < mesh_resolution.y; m++)
            {
                int index = m * mesh_resolution.x + n;
                float sign = 1;

                // Flip the sign
                if ((m + n) % 2)
                {
                    sign = -1;
                }

                glm::vec3 normal(sign * out_slope_x[index][0],
                                 -1,
                                 sign * out_slope_z[index][0]);
                normalField[index] = glm::normalize(normal);

                heightField[index] = glm::vec3(
                                         (n - mesh_resolution.x / 2) * tile_size.x / mesh_resolution.x -
                                         sign * lambda * out_D_x[index][0],
                                         //
                                         sign * out_height[index][0],
                                         //
                                         (m - mesh_resolution.y / 2) * tile_size.y / mesh_resolution.y -
                                         sign * lambda * out_D_z[index][0]);
            }
        }
    });
}

//------------------------------------------------------------------------------------------
void FFTWave::init_fftw_data()
{
    size_t array_size = sizeof(fftwf_complex) * kNum;
    value_h_twiddle_0 = (std::complex<float>*)fftwf_malloc(array_size);
    value_h_twiddle_0_conj = (std::complex<float>*)fftwf_malloc(array_size);
    value_h_twiddle = (std::complex<float>*)fftwf_malloc(array_size);

    slope_x_term = (std::complex<float>*)fftwf_malloc(array_size);
    slope_z_term = (std::complex<float>*)fftwf_malloc(array_size);

    D_x_term = (std::complex<float>*)fftwf_malloc(array_size);
    D_z_term = (std::complex<float>*)fftwf_malloc(array_size);


    out_height = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * kNum);
    out_slope_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * kNum);
    out_slope_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * kNum);
    out_D_x = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * kNum);
    out_D_z = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * kNum);

    in_height = (fftwf_complex*)value_h_twiddle;
    in_slope_x = (fftwf_complex*)slope_x_term;
    in_slope_z = (fftwf_complex*)slope_z_term;
    in_D_x = (fftwf_complex*)D_x_term;
    in_D_z = (fftwf_complex*)D_z_term;

    ////////////////////////////////////////////////////////////////////////////////
    // => create plans
    fftwf_init_threads();
    fftwf_plan_with_nthreads(num_threads);


    p_height = fftwf_plan_dft_2d(mesh_resolution.x, mesh_resolution.y,
                                 in_height, out_height,
                                 FFTW_BACKWARD, FFTW_ESTIMATE);
    p_slope_x = fftwf_plan_dft_2d(mesh_resolution.x, mesh_resolution.y,
                                  in_slope_x, out_slope_x,
                                  FFTW_BACKWARD, FFTW_ESTIMATE);
    p_slope_z = fftwf_plan_dft_2d(mesh_resolution.x, mesh_resolution.y,
                                  in_slope_z, out_slope_z,
                                  FFTW_BACKWARD, FFTW_ESTIMATE);
    p_D_x = fftwf_plan_dft_2d(mesh_resolution.x, mesh_resolution.y,
                              in_D_x, out_D_x,
                              FFTW_BACKWARD, FFTW_ESTIMATE);
    p_D_z = fftwf_plan_dft_2d(mesh_resolution.x, mesh_resolution.y,
                              in_D_z, out_D_z,
                              FFTW_BACKWARD, FFTW_ESTIMATE);
}

//------------------------------------------------------------------------------------------
void FFTWave::shutdown_fftw()
{
    ////////////////////////////////////////////////////////////////////////////////
    // => arrays
    fftwf_free(value_h_twiddle_0);
    fftwf_free(value_h_twiddle_0_conj);
    fftwf_free(value_h_twiddle);

    fftwf_free(slope_x_term);
    fftwf_free(slope_z_term);

    fftwf_free(D_x_term);
    fftwf_free(D_z_term);

    fftwf_free(out_height);
    fftwf_free(out_slope_x);
    fftwf_free(out_slope_z);
    fftwf_free(out_D_x);
    fftwf_free(out_D_z);


    ////////////////////////////////////////////////////////////////////////////////
    // => plans
    fftwf_destroy_plan(p_height);
    fftwf_destroy_plan(p_slope_x);
    fftwf_destroy_plan(p_slope_z);
    fftwf_destroy_plan(p_D_x);
    fftwf_destroy_plan(p_D_z);

    ////////////////////////////////////////////////////////////////////////////////
    // => shutdown
    fftwf_cleanup();
    fftwf_cleanup_threads();
}

//------------------------------------------------------------------------------------------
// Get k vector from mesh grid (m,n)
inline glm::vec2 FFTWave::compute_kvec(unsigned int m, unsigned int n) const
{
    return glm::vec2(2 * M_PI * (m - mesh_resolution.x / 2) / tile_size.x,
                     2 * M_PI * (n  - mesh_resolution.y / 2) / tile_size.y);
}
