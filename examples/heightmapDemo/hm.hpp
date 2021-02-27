#pragma once
#include <cstdint>
#include <vector>

extern const float hm_data[512*512];
struct Image { uint32_t width, height; std::vector<float> data; };
inline Image hm()
{
	std::vector<float> data(512*512);
	std::copy(hm_data, hm_data + 512*512, data.data());
	return Image{ 512, 512, data};
}
