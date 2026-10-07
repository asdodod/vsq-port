#include "TrailNoise.hpp"
extern "C" void *memset(void *destination, int value, __SIZE_TYPE__ count) {
    auto *bytes = static_cast<volatile unsigned char *>(destination);
    for (__SIZE_TYPE__ i = 0; i < count; ++i)
        bytes[i] = static_cast<unsigned char>(value);
    return destination;
}
extern "C" float NoiseChannel(float x, float y, float z, int channel) {
    auto value = VainSabers::SharedTrailNoise().Sample(x, y, z);
    return channel == 0 ? value.x : channel == 1 ? value.y : value.z;
}
extern "C" float DisplacedChannel(float x, float y, float z, float scale, float scroll, float amount, int channel) {
    auto value = VainSabers::DisplaceTrailVertex(VainSabers::TrailNoiseVector{x, y, z}, scale, scroll, amount,
                                               VainSabers::SharedTrailNoise());
    return channel == 0 ? value.x : channel == 1 ? value.y : value.z;
}
