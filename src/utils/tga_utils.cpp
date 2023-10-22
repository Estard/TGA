#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tga/tga_utils.hpp"

#include <filesystem>

namespace tga
{
VertexLayout Vertex::layout()
{
    return {sizeof(Vertex),
            {{offsetof(Vertex, position), tga::Format::r32g32b32_sfloat},
             {offsetof(Vertex, uv), tga::Format::r32g32_sfloat},
             {offsetof(Vertex, normal), tga::Format::r32g32b32_sfloat},
             {offsetof(Vertex, tangent), tga::Format::r32g32b32_sfloat}}};
}

Shader loadShader(std::string const& filepath, ShaderType shaderType, tga::Interface& tgai)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("[Error]: Failed to open file " + filepath);
    auto fileSize = std::filesystem::file_size(filepath);
    std::vector<char> data;
    data.resize(fileSize);
    file.read(data.data(), fileSize);
    file.close();
    return tgai.createShader({shaderType, (uint8_t *)data.data(), data.size()});
}

int formatComponentCount(Format format)
{
    switch (format) {
        case Format::r8_uint:
        case Format::r8_sint:
        case Format::r8_srgb:
        case Format::r8_unorm:
        case Format::r8_snorm:
        case Format::r32_uint:
        case Format::r32_sint:
        case Format::r32_sfloat: return 1;
        case Format::r8g8_uint:
        case Format::r8g8_sint:
        case Format::r8g8_srgb:
        case Format::r8g8_unorm:
        case Format::r8g8_snorm:
        case Format::r32g32_uint:
        case Format::r32g32_sint:
        case Format::r32g32_sfloat: return 2;
        case Format::r8g8b8_uint:
        case Format::r8g8b8_sint:
        case Format::r8g8b8_srgb:
        case Format::r8g8b8_unorm:
        case Format::r8g8b8_snorm:
        case Format::r32g32b32_uint:
        case Format::r32g32b32_sint:
        case Format::r32g32b32_sfloat: return 3;
        case Format::r8g8b8a8_uint:
        case Format::r8g8b8a8_sint:
        case Format::r8g8b8a8_srgb:
        case Format::r8g8b8a8_unorm:
        case Format::r8g8b8a8_snorm:
        case Format::r32g32b32a32_uint:
        case Format::r32g32b32a32_sint:
        case Format::r32g32b32a32_sfloat: return 4;
        default: return 0;
    }
}

bool isFloatingPointFormat(Format format)
{
    switch (format) {
        case Format::r32_sfloat:
        case Format::r32g32_sfloat:
        case Format::r32g32b32_sfloat:
        case Format::r32g32b32a32_sfloat: return true;
        default: return false;
    }
}

TextureBundle loadTexture(std::string const& filepath, Format format, SamplerMode samplerMode, AddressMode addressMode,
                          tga::Interface& tgai, bool doGammaCorrection)
{
    int width, height, channels;
    int components = formatComponentCount(format);
    uint8_t *data;
    uint32_t dataSize = 0;

    if (isFloatingPointFormat(format)) {
        // Usually, we don't want any conversion when loading hdr files
        float currentGamma = 1 / stbi__l2h_gamma;
        if (stbi_is_hdr(filepath.c_str()) && !doGammaCorrection) stbi_hdr_to_ldr_gamma(1);

        data = reinterpret_cast<uint8_t *>(stbi_loadf(filepath.c_str(), &width, &height, &channels, components));
        dataSize = width * height * components * sizeof(float);

        stbi_hdr_to_ldr_gamma(currentGamma);
    } else {
        data = reinterpret_cast<uint8_t *>(stbi_load(filepath.c_str(), &width, &height, &channels, components));
        dataSize = width * height * components * sizeof(uint8_t);
    }

    if (!data) throw std::runtime_error("[TGA] Utils: Can't load image: " + filepath);

    auto staging = tgai.createStagingBuffer({dataSize,data});
    auto texture = tgai.createTexture(
        tga::TextureInfo{static_cast<uint32_t>(width), static_cast<uint32_t>(height), format, samplerMode, addressMode}
            .setSrcData(staging));
    tgai.free(staging);
    stbi_image_free(data);
    return {texture, static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

TextureBundle loadTexture(std::string const& filepath, Format format, SamplerMode samplerMode, tga::Interface& tgai,
                          bool doGammaCorrection)
{
    return loadTexture(filepath, format, samplerMode, tga::AddressMode::clampBorder, tgai, doGammaCorrection);
}

Image loadImage(std::string const& filepath)
{
    int width, height, channels;
    uint8_t *data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    Image image{static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels), {}};
    if (!data) throw std::runtime_error("[TGA] Utils: Can't load image: " + filepath);

    image.data.insert(image.data.end(), &data[0], &data[width * height * channels]);
    stbi_image_free(data);
    return image;
}

HDRImage loadHDRImage(std::string const& filepath, bool doGammaCorrection)
{
    int width, height, channels;

    float currentGamma = 1 / stbi__l2h_gamma;
    if (!doGammaCorrection) stbi_hdr_to_ldr_gamma(1);
    float *data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);
    stbi_hdr_to_ldr_gamma(currentGamma);

    HDRImage image{static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels), {}};
    if (!data) throw std::runtime_error("[TGA] Utils: Can't load image: " + filepath);

    image.data.insert(image.data.end(), &data[0], &data[width * height * channels]);
    stbi_image_free(data);
    return image;
}

void writeHDR(std::string const& filename, uint32_t width, uint32_t height, tga::Format format,
              std::vector<float> const& data)
{
    if (!isFloatingPointFormat(format)) {
        std::cerr << "[TGA] Warning: Specified format is not a floating point format and thus " << filename
                  << " will not be saved to disk\n";
        return;
    }
    int components = formatComponentCount(format);
    if (!(data.size() >= width * height * components)) {
        std::cerr << "[TGA] Warning: provided data not as much as expected, file " << filename
                  << " will not be saved to disk\n";
        return;
    }
    if (!stbi_write_hdr(filename.c_str(), static_cast<int>(width), static_cast<int>(height), components, data.data()))
        std::cerr << "[TGA] Warning: File " << filename << " could not be saved to disk\n";
}

void writePNG(std::string const& filename, uint32_t width, uint32_t height, tga::Format format,
              std::vector<uint8_t> const& data)
{
    if (isFloatingPointFormat(format)) {
        std::cerr << "[TGA] Warning: Specified format is a floating point format and thus " << filename
                  << " will not be saved to disk\n";
        return;
    }
    int components = formatComponentCount(format);
    if (!(data.size() >= width * height * components)) {
        std::cerr << "[TGA] Warning: provided data not as much as expected, file " << filename
                  << " will not be saved to disk\n";
        return;
    }
    if (!stbi_write_png(filename.c_str(), static_cast<int>(width), static_cast<int>(height), components, data.data(),
                        0))
        std::cerr << "[TGA] Warning: File " << filename << " could not be saved to disk\n";
}
}  // namespace tga
