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

#include <Banana/TypeNames.h>

#define DEFAULT_WAVE_RESOLUTION 16
#define DEFAULT_WAVE_AMPLITUDE  1e-6
#define DEFAULT_WIN_SPEED       1000.0f
#define DEFAULT_WIN_DIRECTION   Vec2f(1.0f, 1.0f)
#define DEFAULT_TILE_SIZE       1000.0f

#define DEFAULT_SUN_DIRECTION   glm::normalize(Vec3f(0.0f, 1.0f, -2.0f))
#define DEFAULT_SUN_COLOR       Vec3f(1.0f, 1.0f, 1.0f)
#define DEFAULT_SEA_COLOR       Vec3f(0.0f, 0.64, 0.68)
#define DEFAULT_TIMESTEP        0.2f

#ifndef M_PI
#define M_PI                    3.14159265358979323846f
#endif

#define GRAVITY                 9.8f
#define WAVE_LENGTH             0.1f


#define DEFAULT_SURFACE_MATERIAL           \
    {                                      \
        glm::vec4(0.2 * 0.2),              \
        glm::vec4(0.0f, 0.0f, 0.5f, 1.0f), \
        glm::vec4(1),                      \
        250.0,                             \
        std::string("SurfaceMaterial")     \
    }
#define DEFAULT_NUM_THREADS 4
