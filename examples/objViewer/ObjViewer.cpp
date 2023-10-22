#include "Framework.hpp"
#include "tga/tga_math.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "tinyobjloader/tiny_obj_loader.h"

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    bool operator==(const Vertex& other) const
    {
        return position == other.position && uv == other.uv && normal == other.normal;
    }
};

struct Camera {
    alignas(16) glm::mat4 view = glm::mat4(1);
    alignas(16) glm::mat4 projection = glm::mat4(1);
    alignas(16) glm::vec3 lightPos = glm::vec3(0);
};

struct Transform {
    alignas(16) glm::mat4 transform = glm::mat4(1);
};

namespace std
{
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& v) const
    {
        return ((hash<glm::vec3>()(v.position) ^ (hash<glm::vec3>()(v.normal) << 1)) >> 1) ^
               (hash<glm::vec2>()(v.uv) << 1);
    }
};
}  // namespace std

// This is a valid SPIR-V Shader
std::array<uint8_t, 2708> objVertSPV = {
    3,   2,   35,  7,   0,   0,   1,   0,   8,   0,   13,  0,   79,  0,   0,   0,   0,   0,   0,   0,   17,  0,   2,
    0,   1,   0,   0,   0,   11,  0,   6,   0,   1,   0,   0,   0,   71,  76,  83,  76,  46,  115, 116, 100, 46,  52,
    53,  48,  0,   0,   0,   0,   14,  0,   3,   0,   0,   0,   0,   0,   1,   0,   0,   0,   15,  0,   16,  0,   0,
    0,   0,   0,   4,   0,   0,   0,   109, 97,  105, 110, 0,   0,   0,   0,   13,  0,   0,   0,   35,  0,   0,   0,
    45,  0,   0,   0,   56,  0,   0,   0,   58,  0,   0,   0,   61,  0,   0,   0,   64,  0,   0,   0,   73,  0,   0,
    0,   74,  0,   0,   0,   76,  0,   0,   0,   77,  0,   0,   0,   3,   0,   3,   0,   2,   0,   0,   0,   194, 1,
    0,   0,   4,   0,   9,   0,   71,  76,  95,  65,  82,  66,  95,  115, 101, 112, 97,  114, 97,  116, 101, 95,  115,
    104, 97,  100, 101, 114, 95,  111, 98,  106, 101, 99,  116, 115, 0,   0,   4,   0,   10,  0,   71,  76,  95,  71,
    79,  79,  71,  76,  69,  95,  99,  112, 112, 95,  115, 116, 121, 108, 101, 95,  108, 105, 110, 101, 95,  100, 105,
    114, 101, 99,  116, 105, 118, 101, 0,   0,   4,   0,   8,   0,   71,  76,  95,  71,  79,  79,  71,  76,  69,  95,
    105, 110, 99,  108, 117, 100, 101, 95,  100, 105, 114, 101, 99,  116, 105, 118, 101, 0,   5,   0,   4,   0,   4,
    0,   0,   0,   109, 97,  105, 110, 0,   0,   0,   0,   5,   0,   6,   0,   11,  0,   0,   0,   103, 108, 95,  80,
    101, 114, 86,  101, 114, 116, 101, 120, 0,   0,   0,   0,   6,   0,   6,   0,   11,  0,   0,   0,   0,   0,   0,
    0,   103, 108, 95,  80,  111, 115, 105, 116, 105, 111, 110, 0,   6,   0,   7,   0,   11,  0,   0,   0,   1,   0,
    0,   0,   103, 108, 95,  80,  111, 105, 110, 116, 83,  105, 122, 101, 0,   0,   0,   0,   6,   0,   7,   0,   11,
    0,   0,   0,   2,   0,   0,   0,   103, 108, 95,  67,  108, 105, 112, 68,  105, 115, 116, 97,  110, 99,  101, 0,
    6,   0,   7,   0,   11,  0,   0,   0,   3,   0,   0,   0,   103, 108, 95,  67,  117, 108, 108, 68,  105, 115, 116,
    97,  110, 99,  101, 0,   5,   0,   3,   0,   13,  0,   0,   0,   0,   0,   0,   0,   5,   0,   4,   0,   18,  0,
    0,   0,   67,  97,  109, 101, 114, 97,  0,   0,   6,   0,   5,   0,   18,  0,   0,   0,   0,   0,   0,   0,   118,
    105, 101, 119, 0,   0,   0,   0,   6,   0,   6,   0,   18,  0,   0,   0,   1,   0,   0,   0,   112, 114, 111, 106,
    101, 99,  116, 105, 111, 110, 0,   0,   6,   0,   6,   0,   18,  0,   0,   0,   2,   0,   0,   0,   108, 105, 103,
    104, 116, 80,  111, 115, 0,   0,   0,   0,   5,   0,   3,   0,   20,  0,   0,   0,   99,  97,  109, 0,   5,   0,
    4,   0,   28,  0,   0,   0,   77,  111, 100, 101, 108, 0,   0,   0,   6,   0,   6,   0,   28,  0,   0,   0,   0,
    0,   0,   0,   111, 98,  106, 84,  111, 87,  111, 114, 108, 100, 0,   0,   5,   0,   4,   0,   30,  0,   0,   0,
    109, 111, 100, 101, 108, 0,   0,   0,   5,   0,   5,   0,   35,  0,   0,   0,   112, 111, 115, 105, 116, 105, 111,
    110, 0,   0,   0,   0,   5,   0,   6,   0,   45,  0,   0,   0,   119, 111, 114, 108, 100, 80,  111, 115, 105, 116,
    105, 111, 110, 0,   0,   0,   5,   0,   4,   0,   56,  0,   0,   0,   111, 117, 116, 85,  86,  0,   0,   0,   5,
    0,   3,   0,   58,  0,   0,   0,   117, 118, 0,   0,   5,   0,   5,   0,   61,  0,   0,   0,   119, 111, 114, 108,
    100, 78,  111, 114, 109, 97,  108, 0,   5,   0,   4,   0,   64,  0,   0,   0,   110, 111, 114, 109, 97,  108, 0,
    0,   5,   0,   5,   0,   73,  0,   0,   0,   111, 117, 116, 84,  97,  110, 103, 101, 110, 116, 0,   0,   5,   0,
    4,   0,   74,  0,   0,   0,   116, 97,  110, 103, 101, 110, 116, 0,   5,   0,   6,   0,   76,  0,   0,   0,   111,
    117, 116, 66,  105, 116, 97,  110, 103, 101, 110, 116, 0,   0,   0,   0,   5,   0,   5,   0,   77,  0,   0,   0,
    98,  105, 116, 97,  110, 103, 101, 110, 116, 0,   0,   0,   72,  0,   5,   0,   11,  0,   0,   0,   0,   0,   0,
    0,   11,  0,   0,   0,   0,   0,   0,   0,   72,  0,   5,   0,   11,  0,   0,   0,   1,   0,   0,   0,   11,  0,
    0,   0,   1,   0,   0,   0,   72,  0,   5,   0,   11,  0,   0,   0,   2,   0,   0,   0,   11,  0,   0,   0,   3,
    0,   0,   0,   72,  0,   5,   0,   11,  0,   0,   0,   3,   0,   0,   0,   11,  0,   0,   0,   4,   0,   0,   0,
    71,  0,   3,   0,   11,  0,   0,   0,   2,   0,   0,   0,   72,  0,   4,   0,   18,  0,   0,   0,   0,   0,   0,
    0,   5,   0,   0,   0,   72,  0,   5,   0,   18,  0,   0,   0,   0,   0,   0,   0,   35,  0,   0,   0,   0,   0,
    0,   0,   72,  0,   5,   0,   18,  0,   0,   0,   0,   0,   0,   0,   7,   0,   0,   0,   16,  0,   0,   0,   72,
    0,   4,   0,   18,  0,   0,   0,   1,   0,   0,   0,   5,   0,   0,   0,   72,  0,   5,   0,   18,  0,   0,   0,
    1,   0,   0,   0,   35,  0,   0,   0,   64,  0,   0,   0,   72,  0,   5,   0,   18,  0,   0,   0,   1,   0,   0,
    0,   7,   0,   0,   0,   16,  0,   0,   0,   72,  0,   5,   0,   18,  0,   0,   0,   2,   0,   0,   0,   35,  0,
    0,   0,   128, 0,   0,   0,   71,  0,   3,   0,   18,  0,   0,   0,   2,   0,   0,   0,   71,  0,   4,   0,   20,
    0,   0,   0,   34,  0,   0,   0,   0,   0,   0,   0,   71,  0,   4,   0,   20,  0,   0,   0,   33,  0,   0,   0,
    0,   0,   0,   0,   72,  0,   4,   0,   28,  0,   0,   0,   0,   0,   0,   0,   5,   0,   0,   0,   72,  0,   5,
    0,   28,  0,   0,   0,   0,   0,   0,   0,   35,  0,   0,   0,   0,   0,   0,   0,   72,  0,   5,   0,   28,  0,
    0,   0,   0,   0,   0,   0,   7,   0,   0,   0,   16,  0,   0,   0,   71,  0,   3,   0,   28,  0,   0,   0,   2,
    0,   0,   0,   71,  0,   4,   0,   30,  0,   0,   0,   34,  0,   0,   0,   0,   0,   0,   0,   71,  0,   4,   0,
    30,  0,   0,   0,   33,  0,   0,   0,   1,   0,   0,   0,   71,  0,   4,   0,   35,  0,   0,   0,   30,  0,   0,
    0,   0,   0,   0,   0,   71,  0,   4,   0,   45,  0,   0,   0,   30,  0,   0,   0,   0,   0,   0,   0,   71,  0,
    4,   0,   56,  0,   0,   0,   30,  0,   0,   0,   1,   0,   0,   0,   71,  0,   4,   0,   58,  0,   0,   0,   30,
    0,   0,   0,   1,   0,   0,   0,   71,  0,   4,   0,   61,  0,   0,   0,   30,  0,   0,   0,   2,   0,   0,   0,
    71,  0,   4,   0,   64,  0,   0,   0,   30,  0,   0,   0,   2,   0,   0,   0,   71,  0,   4,   0,   73,  0,   0,
    0,   30,  0,   0,   0,   3,   0,   0,   0,   71,  0,   4,   0,   74,  0,   0,   0,   30,  0,   0,   0,   3,   0,
    0,   0,   71,  0,   4,   0,   76,  0,   0,   0,   30,  0,   0,   0,   4,   0,   0,   0,   71,  0,   4,   0,   77,
    0,   0,   0,   30,  0,   0,   0,   4,   0,   0,   0,   19,  0,   2,   0,   2,   0,   0,   0,   33,  0,   3,   0,
    3,   0,   0,   0,   2,   0,   0,   0,   22,  0,   3,   0,   6,   0,   0,   0,   32,  0,   0,   0,   23,  0,   4,
    0,   7,   0,   0,   0,   6,   0,   0,   0,   4,   0,   0,   0,   21,  0,   4,   0,   8,   0,   0,   0,   32,  0,
    0,   0,   0,   0,   0,   0,   43,  0,   4,   0,   8,   0,   0,   0,   9,   0,   0,   0,   1,   0,   0,   0,   28,
    0,   4,   0,   10,  0,   0,   0,   6,   0,   0,   0,   9,   0,   0,   0,   30,  0,   6,   0,   11,  0,   0,   0,
    7,   0,   0,   0,   6,   0,   0,   0,   10,  0,   0,   0,   10,  0,   0,   0,   32,  0,   4,   0,   12,  0,   0,
    0,   3,   0,   0,   0,   11,  0,   0,   0,   59,  0,   4,   0,   12,  0,   0,   0,   13,  0,   0,   0,   3,   0,
    0,   0,   21,  0,   4,   0,   14,  0,   0,   0,   32,  0,   0,   0,   1,   0,   0,   0,   43,  0,   4,   0,   14,
    0,   0,   0,   15,  0,   0,   0,   0,   0,   0,   0,   24,  0,   4,   0,   16,  0,   0,   0,   7,   0,   0,   0,
    4,   0,   0,   0,   23,  0,   4,   0,   17,  0,   0,   0,   6,   0,   0,   0,   3,   0,   0,   0,   30,  0,   5,
    0,   18,  0,   0,   0,   16,  0,   0,   0,   16,  0,   0,   0,   17,  0,   0,   0,   32,  0,   4,   0,   19,  0,
    0,   0,   2,   0,   0,   0,   18,  0,   0,   0,   59,  0,   4,   0,   19,  0,   0,   0,   20,  0,   0,   0,   2,
    0,   0,   0,   43,  0,   4,   0,   14,  0,   0,   0,   21,  0,   0,   0,   1,   0,   0,   0,   32,  0,   4,   0,
    22,  0,   0,   0,   2,   0,   0,   0,   16,  0,   0,   0,   30,  0,   3,   0,   28,  0,   0,   0,   16,  0,   0,
    0,   32,  0,   4,   0,   29,  0,   0,   0,   2,   0,   0,   0,   28,  0,   0,   0,   59,  0,   4,   0,   29,  0,
    0,   0,   30,  0,   0,   0,   2,   0,   0,   0,   32,  0,   4,   0,   34,  0,   0,   0,   1,   0,   0,   0,   17,
    0,   0,   0,   59,  0,   4,   0,   34,  0,   0,   0,   35,  0,   0,   0,   1,   0,   0,   0,   43,  0,   4,   0,
    6,   0,   0,   0,   37,  0,   0,   0,   0,   0,   128, 63,  32,  0,   4,   0,   43,  0,   0,   0,   3,   0,   0,
    0,   7,   0,   0,   0,   59,  0,   4,   0,   43,  0,   0,   0,   45,  0,   0,   0,   3,   0,   0,   0,   23,  0,
    4,   0,   54,  0,   0,   0,   6,   0,   0,   0,   2,   0,   0,   0,   32,  0,   4,   0,   55,  0,   0,   0,   3,
    0,   0,   0,   54,  0,   0,   0,   59,  0,   4,   0,   55,  0,   0,   0,   56,  0,   0,   0,   3,   0,   0,   0,
    32,  0,   4,   0,   57,  0,   0,   0,   1,   0,   0,   0,   54,  0,   0,   0,   59,  0,   4,   0,   57,  0,   0,
    0,   58,  0,   0,   0,   1,   0,   0,   0,   32,  0,   4,   0,   60,  0,   0,   0,   3,   0,   0,   0,   17,  0,
    0,   0,   59,  0,   4,   0,   60,  0,   0,   0,   61,  0,   0,   0,   3,   0,   0,   0,   59,  0,   4,   0,   34,
    0,   0,   0,   64,  0,   0,   0,   1,   0,   0,   0,   43,  0,   4,   0,   6,   0,   0,   0,   66,  0,   0,   0,
    0,   0,   0,   0,   59,  0,   4,   0,   60,  0,   0,   0,   73,  0,   0,   0,   3,   0,   0,   0,   59,  0,   4,
    0,   34,  0,   0,   0,   74,  0,   0,   0,   1,   0,   0,   0,   59,  0,   4,   0,   60,  0,   0,   0,   76,  0,
    0,   0,   3,   0,   0,   0,   59,  0,   4,   0,   34,  0,   0,   0,   77,  0,   0,   0,   1,   0,   0,   0,   54,
    0,   5,   0,   2,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   3,   0,   0,   0,   248, 0,   2,   0,
    5,   0,   0,   0,   65,  0,   5,   0,   22,  0,   0,   0,   23,  0,   0,   0,   20,  0,   0,   0,   21,  0,   0,
    0,   61,  0,   4,   0,   16,  0,   0,   0,   24,  0,   0,   0,   23,  0,   0,   0,   65,  0,   5,   0,   22,  0,
    0,   0,   25,  0,   0,   0,   20,  0,   0,   0,   15,  0,   0,   0,   61,  0,   4,   0,   16,  0,   0,   0,   26,
    0,   0,   0,   25,  0,   0,   0,   146, 0,   5,   0,   16,  0,   0,   0,   27,  0,   0,   0,   24,  0,   0,   0,
    26,  0,   0,   0,   65,  0,   5,   0,   22,  0,   0,   0,   31,  0,   0,   0,   30,  0,   0,   0,   15,  0,   0,
    0,   61,  0,   4,   0,   16,  0,   0,   0,   32,  0,   0,   0,   31,  0,   0,   0,   146, 0,   5,   0,   16,  0,
    0,   0,   33,  0,   0,   0,   27,  0,   0,   0,   32,  0,   0,   0,   61,  0,   4,   0,   17,  0,   0,   0,   36,
    0,   0,   0,   35,  0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   38,  0,   0,   0,   36,  0,   0,   0,
    0,   0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   39,  0,   0,   0,   36,  0,   0,   0,   1,   0,   0,
    0,   81,  0,   5,   0,   6,   0,   0,   0,   40,  0,   0,   0,   36,  0,   0,   0,   2,   0,   0,   0,   80,  0,
    7,   0,   7,   0,   0,   0,   41,  0,   0,   0,   38,  0,   0,   0,   39,  0,   0,   0,   40,  0,   0,   0,   37,
    0,   0,   0,   145, 0,   5,   0,   7,   0,   0,   0,   42,  0,   0,   0,   33,  0,   0,   0,   41,  0,   0,   0,
    65,  0,   5,   0,   43,  0,   0,   0,   44,  0,   0,   0,   13,  0,   0,   0,   15,  0,   0,   0,   62,  0,   3,
    0,   44,  0,   0,   0,   42,  0,   0,   0,   65,  0,   5,   0,   22,  0,   0,   0,   46,  0,   0,   0,   30,  0,
    0,   0,   15,  0,   0,   0,   61,  0,   4,   0,   16,  0,   0,   0,   47,  0,   0,   0,   46,  0,   0,   0,   61,
    0,   4,   0,   17,  0,   0,   0,   48,  0,   0,   0,   35,  0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,
    49,  0,   0,   0,   48,  0,   0,   0,   0,   0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   50,  0,   0,
    0,   48,  0,   0,   0,   1,   0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   51,  0,   0,   0,   48,  0,
    0,   0,   2,   0,   0,   0,   80,  0,   7,   0,   7,   0,   0,   0,   52,  0,   0,   0,   49,  0,   0,   0,   50,
    0,   0,   0,   51,  0,   0,   0,   37,  0,   0,   0,   145, 0,   5,   0,   7,   0,   0,   0,   53,  0,   0,   0,
    47,  0,   0,   0,   52,  0,   0,   0,   62,  0,   3,   0,   45,  0,   0,   0,   53,  0,   0,   0,   61,  0,   4,
    0,   54,  0,   0,   0,   59,  0,   0,   0,   58,  0,   0,   0,   62,  0,   3,   0,   56,  0,   0,   0,   59,  0,
    0,   0,   65,  0,   5,   0,   22,  0,   0,   0,   62,  0,   0,   0,   30,  0,   0,   0,   15,  0,   0,   0,   61,
    0,   4,   0,   16,  0,   0,   0,   63,  0,   0,   0,   62,  0,   0,   0,   61,  0,   4,   0,   17,  0,   0,   0,
    65,  0,   0,   0,   64,  0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   67,  0,   0,   0,   65,  0,   0,
    0,   0,   0,   0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   68,  0,   0,   0,   65,  0,   0,   0,   1,   0,
    0,   0,   81,  0,   5,   0,   6,   0,   0,   0,   69,  0,   0,   0,   65,  0,   0,   0,   2,   0,   0,   0,   80,
    0,   7,   0,   7,   0,   0,   0,   70,  0,   0,   0,   67,  0,   0,   0,   68,  0,   0,   0,   69,  0,   0,   0,
    66,  0,   0,   0,   145, 0,   5,   0,   7,   0,   0,   0,   71,  0,   0,   0,   63,  0,   0,   0,   70,  0,   0,
    0,   79,  0,   8,   0,   17,  0,   0,   0,   72,  0,   0,   0,   71,  0,   0,   0,   71,  0,   0,   0,   0,   0,
    0,   0,   1,   0,   0,   0,   2,   0,   0,   0,   62,  0,   3,   0,   61,  0,   0,   0,   72,  0,   0,   0,   61,
    0,   4,   0,   17,  0,   0,   0,   75,  0,   0,   0,   74,  0,   0,   0,   62,  0,   3,   0,   73,  0,   0,   0,
    75,  0,   0,   0,   61,  0,   4,   0,   17,  0,   0,   0,   78,  0,   0,   0,   77,  0,   0,   0,   62,  0,   3,
    0,   76,  0,   0,   0,   78,  0,   0,   0,   253, 0,   1,   0,   56,  0,   1,   0};

class ObjViewer : public Framework {
public:
    ObjViewer(std::string const& _objFilepath, std::string const& _diffuseMapPath = "",
              std::string const& _normalMapPath = "")
        : objFilepath(_objFilepath), diffuseMapPath(_diffuseMapPath), normalMapPath(_normalMapPath)
    {}

private:
    void OnCreate()
    {
        LoadObj();
        LoadData();
        CreateRenderPass();
        CreateInputSets();
    }
    void OnUpdate(uint32_t frame)
    {
        UpdateCam();
        tga::CommandRecorder cmd{tgai, cmdBuffer};
        cmd.bindVertexBuffer(vertexBuffer)
            .bindIndexBuffer(indexBuffer)
            .setRenderPass(renderPass, frame)
            .bindInputSet(camInputSet);

        if (diffuseMapInputSet) cmd.bindInputSet(diffuseMapInputSet);
        if (normalMapInputSet) cmd.bindInputSet(normalMapInputSet);
        cmdBuffer = cmd.drawIndexed(modelVertexCount, 0, 0).endRecording();
        tgai.execute(cmdBuffer);
    }

    void LoadObj()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFilepath.c_str()))
            throw std::runtime_error(warn + err);

        std::vector<Vertex> preVertexBuffer;
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                                   attrib.vertices[3 * index.vertex_index + 1],
                                   attrib.vertices[3 * index.vertex_index + 2]};
                vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]};
                vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                             1.f - attrib.texcoords[2 * index.texcoord_index + 1]};
                preVertexBuffer.emplace_back(vertex);

                circleRadius = std::max(circleRadius, vertex.position.x);
                circleRadius = std::max(circleRadius, vertex.position.y);
                circleRadius = std::max(circleRadius, vertex.position.z);
            }
        }

        std::unordered_map<Vertex, uint32_t> foundVertices{};
        std::vector<Vertex> vBuffer;
        std::vector<uint32_t> iBuffer;

        // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
        for (size_t i = 0; i < preVertexBuffer.size(); i += 3) {
            auto& p0 = preVertexBuffer[i + 0].position;
            auto& p1 = preVertexBuffer[i + 1].position;
            auto& p2 = preVertexBuffer[i + 2].position;

            auto& uv0 = preVertexBuffer[i + 0].uv;
            auto& uv1 = preVertexBuffer[i + 1].uv;
            auto& uv2 = preVertexBuffer[i + 2].uv;

            auto deltaPos1 = p1 - p0;
            auto deltaPos2 = p2 - p0;

            auto deltaUV1 = uv1 - uv0;
            auto deltaUV2 = uv2 - uv0;

            float r = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
            glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

            preVertexBuffer[i + 0].tangent = tangent;
            preVertexBuffer[i + 1].tangent = tangent;
            preVertexBuffer[i + 2].tangent = tangent;

            preVertexBuffer[i + 0].bitangent = bitangent;
            preVertexBuffer[i + 1].bitangent = bitangent;
            preVertexBuffer[i + 2].bitangent = bitangent;
        }

        for (const auto& vertex : preVertexBuffer) {
            if (!foundVertices.count(vertex)) {  // It's a new Vertex
                foundVertices[vertex] = static_cast<uint32_t>(vBuffer.size());
                vBuffer.emplace_back(vertex);
            } else {  // Seen before, average the the tangents
                auto& v = vBuffer[foundVertices[vertex]];
                v.tangent += vertex.tangent;
                v.bitangent += vertex.bitangent;
            }
            iBuffer.emplace_back(foundVertices[vertex]);
        }
        preVertexBuffer.clear();
        modelVertexCount = static_cast<uint32_t>(iBuffer.size());
        uint32_t vBufferSize = vBuffer.size() * sizeof(Vertex);
        uint32_t iBufferSize = iBuffer.size() * sizeof(uint32_t);

        vertexBuffer = tgai.createBuffer({tga::BufferUsage::vertex, (uint8_t *)vBuffer.data(), vBufferSize});
        indexBuffer = tgai.createBuffer({tga::BufferUsage::index, (uint8_t *)iBuffer.data(), iBufferSize});
    }

    void LoadData()
    {
        auto loadTex = [&](tga::Texture& tex, std::string const& filepath, tga::Format format) {
            if (filepath.size() > 0) {
                int width, height, channels;
                stbi_uc *pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
                if (pixels) {
                    tex = tgai.createTexture({static_cast<uint32_t>(width), static_cast<uint32_t>(height), format,
                                              static_cast<uint8_t *>(pixels), width * height * 4u,
                                              tga::SamplerMode::linear});
                    stbi_image_free(pixels);
                }
            }
        };
        loadTex(diffuseMap, diffuseMapPath, tga::Format::r8g8b8a8_srgb);
        loadTex(normalMap, normalMapPath, tga::Format::r8g8b8a8_unorm);
        camData = tgai.createBuffer({tga::BufferUsage::uniform, (uint8_t *)&cam, sizeof(cam)});
        modelData = tgai.createBuffer({tga::BufferUsage::uniform, (uint8_t *)&modelTransform, sizeof(modelTransform)});
    }

    void CreateRenderPass()
    {
        auto LoadShader = [&](const std::string& filename, tga::ShaderType type) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) throw std::runtime_error("failed to open file " + filename);
            size_t fileSize = (size_t)file.tellg();
            std::vector<char> shaderData(fileSize);
            file.seekg(0);
            file.read(shaderData.data(), fileSize);
            file.close();
            return tgai.createShader({type, (uint8_t *)shaderData.data(), shaderData.size()});
        };

        tga::InputLayout inputLayout({// Set = 0: Camera Data, Object Data
                                      {{{tga::BindingType::uniformBuffer}, {tga::BindingType::uniformBuffer}}},
                                      // Set = 1: Diffuse Map
                                      {{{tga::BindingType::sampler}}},
                                      // Set = 2: Normal Map
                                      {{{tga::BindingType::sampler}}}});

        tga::Shader vs, fs;

        if (diffuseMap && normalMap) {
            vs = LoadShader("shaders/objDNVert.spv", tga::ShaderType::vertex);
            fs = LoadShader("shaders/objDNFrag.spv", tga::ShaderType::fragment);
        } else if (diffuseMap) {
            vs = tgai.createShader({tga::ShaderType::vertex, objVertSPV.data(), objVertSPV.size()});
            fs = LoadShader("shaders/objDFrag.spv", tga::ShaderType::fragment);

        } else {
            vs = tgai.createShader({tga::ShaderType::vertex, objVertSPV.data(),
                                    objVertSPV.size()});  // LoadShader("shaders/objVert.spv",tga::ShaderType::vertex);
            fs = LoadShader("shaders/objFrag.spv", tga::ShaderType::fragment);
        }

        renderPass = tgai.createRenderPass({vs, fs,
                                            _frameworkWindow,
                                            tga::ClearOperation::all,
                                            {tga::FrontFace::counterclockwise, tga::CullMode::back},
                                            {tga::CompareOperation::less},
                                            inputLayout,
                                            {// Vertex Layout
                                             sizeof(Vertex),
                                             {{offsetof(Vertex, position), tga::Format::r32g32b32_sfloat},
                                              {offsetof(Vertex, uv), tga::Format::r32g32_sfloat},
                                              {offsetof(Vertex, normal), tga::Format::r32g32b32_sfloat},
                                              {offsetof(Vertex, tangent), tga::Format::r32g32b32_sfloat},
                                              {offsetof(Vertex, bitangent), tga::Format::r32g32b32_sfloat}}}});
    }

    void CreateInputSets()
    {
        camInputSet = tgai.createInputSet({renderPass, 0, {{camData, 0}, {modelData, 1}}});
        if (diffuseMap) diffuseMapInputSet = tgai.createInputSet({renderPass, 1, {{diffuseMap, 0}}});
        if (normalMap) normalMapInputSet = tgai.createInputSet({renderPass, 2, {{normalMap, 0}}});
    }

    void UpdateCam()
    {
        static float fullTime = 0.0;
        fullTime += deltaTime;
        const glm::vec3 position =
            glm::vec3(1.5 * circleRadius * std::sin(fullTime), 2.f, 1.5 * circleRadius * std::cos(fullTime));
        // const glm::vec3 lookDirection = glm::vec3(1.f,0.f,0.f);
        const glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);

        cam.projection = glm::perspective_vk(
            glm::radians(90.f), _frameworkWindowWidth / static_cast<float>(_frameworkWindowHeight), 0.1f, 1000.f);
        // cam.projection[1][1] *= -1;
        cam.view = glm::lookAt(position, glm::vec3(0, 0, 0), up);
        cam.lightPos = glm::vec3(2 * circleRadius);
        tgai.updateBuffer(camData, (uint8_t *)&cam, sizeof(cam), 0);
    }

    std::string objFilepath;
    std::string diffuseMapPath;
    std::string normalMapPath;
    tga::CommandBuffer cmdBuffer;
    tga::Buffer vertexBuffer;
    tga::Buffer indexBuffer;

    tga::Buffer camData;
    tga::Buffer modelData;

    tga::Texture diffuseMap;
    tga::Texture normalMap;

    tga::RenderPass renderPass;
    tga::InputSet camInputSet, diffuseMapInputSet, normalMapInputSet;

    Camera cam;
    Transform modelTransform;
    uint32_t modelVertexCount;
    float circleRadius = 1;
};

int main(int argc, char **argv)
{
    try {
        switch (argc) {
            case 1: {
                std::cerr << "No Files provided\n";
            } break;
            case 2: {
                ObjViewer objViewer(argv[1]);
                objViewer.run();
            } break;
            case 3: {
                ObjViewer objViewer(argv[1], argv[2]);
                objViewer.run();
            } break;

            default: {
                ObjViewer objViewer(argv[1], argv[2], argv[3]);
                objViewer.run();

            } break;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Done" << std::endl;
}