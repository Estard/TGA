#pragma once

// Math is mostly just GLM with a bunch of defines and some additions
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

namespace glm
{
    // Preflipped y-axis, depth range 0.0 to 1.0
    template <typename T>
    GLM_FUNC_QUALIFIER mat<4, 4, T, defaultp> perspective_vk(T fovy, T aspect, T zNear, T zFar)
    {
        assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

        T const tanHalfFovy = tan(fovy / static_cast<T>(2));

        mat<4, 4, T, defaultp> Result(static_cast<T>(0));
        Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
        Result[1][1] = -static_cast<T>(1) / (tanHalfFovy);
        Result[2][2] = zFar / (zNear - zFar);
        Result[2][3] = -static_cast<T>(1);
        Result[3][2] = -(zFar * zNear) / (zFar - zNear);
        return Result;
    }

    // Standard value remapping function
    template <typename T>
    GLM_FUNC_QUALIFIER T remap(T value, T fromMin, T fromMax, T toMin, T toMax)
    {
        return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
    }
};  // namespace glm