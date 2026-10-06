#include "ObjCoordinates.hpp"
#include "LegacyNoiseRandom.hpp"

// Build these production helpers as WebAssembly to run without Unity or ADB.
extern "C" void *memset(void *destination, int value, __SIZE_TYPE__ count) {
    auto *bytes = static_cast<volatile unsigned char *>(destination);
    for (__SIZE_TYPE__ i = 0; i < count; ++i)
        bytes[i] = static_cast<unsigned char>(value);
    return destination;
}
static double samples[32 * 32 * 32 * 3];
extern "C" double *NoiseSamples() {
    VainSabers::LegacyNoiseRandom random;
    for (double &sample : samples)
        sample = random.NextDouble();
    return samples;
}
extern "C" float ConvertedZ(float value, int version) {
    return VainSabers::ObjCoordinateZ(value, version);
}
extern "C" int ReversedWinding(int version) {
    return VainSabers::ReverseObjWinding(version);
}
