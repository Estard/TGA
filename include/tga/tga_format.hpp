#pragma once

namespace tga
{

    // Format types used to describe the structure of data in e.g. textures and vertices
    enum class Format {
        undefined,
        r8_uint,
        r8_sint,
        r8_srgb,
        r8_unorm,
        r8_snorm,
        r8g8_uint,
        r8g8_sint,
        r8g8_srgb,
        r8g8_unorm,
        r8g8_snorm,
        r8g8b8_uint,
        r8g8b8_sint,
        r8g8b8_srgb,
        r8g8b8_unorm,
        r8g8b8_snorm,
        r8g8b8a8_uint,
        r8g8b8a8_sint,
        r8g8b8a8_srgb,
        r8g8b8a8_unorm,
        r8g8b8a8_snorm,
        r32_uint,
        r32_sint,
        r32_sfloat,
        r32g32_uint,
        r32g32_sint,
        r32g32_sfloat,
        r32g32b32_uint,
        r32g32b32_sint,
        r32g32b32_sfloat,
        r32g32b32a32_uint,
        r32g32b32a32_sint,
        r32g32b32a32_sfloat,
        r16_sfloat,
        r16g16_sfloat,
        r16g16b16_sfloat,
        r16g16b16a16_sfloat
    };

}