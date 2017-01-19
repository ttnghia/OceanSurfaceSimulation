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
#ifndef __FFT_WAVE_H__
#define __FFT_WAVE_H__

#include <glm/glm.hpp>
#include <fftw3.h>

#include <random>
#include <complex>

#include <QtAppHelpers/QtAppMacros.h>

//------------------------------------------------------------------------------------------
class FFTWave
{
public:
    // mesh_resolution: resolution of the height field
    // tile_size:  the actual size of the grid (in meters)
    FFTWave(Resolution2D _mesh_resolution, glm::vec2 _tile_size,
            glm::vec2 omega, float wind_speed, float wave_amplitude, float lambda,
            int _compute_threads);
    ~FFTWave();
    void buildHeightField(float time);

    glm::vec3* heightField;
    glm::vec3* normalField;

private:
    void init_fftw_data();
    void shutdown_fftw();

    inline glm::vec2 compute_kvec(unsigned int m, unsigned int n) const;
    inline float func_omega(float k) const;
    inline float func_P_h(glm::vec2 vec_k) const;
    inline std::complex<float> func_h_twiddle_0(glm::vec2 vec_k);
    inline std::complex<float> func_h_twiddle(int kn, int km, float t) const;


    int num_threads;

    std::complex<float>* value_h_twiddle_0;
    std::complex<float>* value_h_twiddle_0_conj;
    std::complex<float>* value_h_twiddle;

    // Eq20 ikh_twiddle
    std::complex<float>* slope_x_term;
    std::complex<float>* slope_z_term;

    // Eq29
    std::complex<float>* D_x_term;
    std::complex<float>* D_z_term;

    std::default_random_engine generator;
    std::normal_distribution<float> normal_dist;

    const float pi = float(M_PI);
    const float g = 9.8f; // Gravitational constant
    const float l = 0.1;
    float wave_amplitude;
    Resolution2D mesh_resolution;
    int kNum;
    float wind_speed;
    glm::vec2 wind_direction;
    float lambda;
    glm::vec2 tile_size;


    ////////////////////////////////////////////////////////////////////////////////
    // => fftw variables
    fftwf_complex* in_height, *in_slope_x, *in_slope_z, *in_D_x, *in_D_z;
    fftwf_complex* out_height, *out_slope_x, *out_slope_z, *out_D_x, *out_D_z;

    fftwf_plan p_height, p_slope_x, p_slope_z, p_D_x, p_D_z;
};

#endif // __FFT_WAVE_H__