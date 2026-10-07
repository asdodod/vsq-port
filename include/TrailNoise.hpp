#pragma once
#include "LegacyNoiseRandom.hpp"

namespace VainSabers {
struct TrailNoiseVector {
    float x, y, z;
};

// Matches the seeded RGBA32 texture, repeat addressing and trilinear tex3Dlod
// sampling. Bake displacement into the ribbon already uploaded every frame,
// instead of fetching a 3D texture again for each eye and glow pass.
class TrailNoiseVolume {
    static constexpr int Side = 32;
    TrailNoiseVector samples[Side * Side * Side];

    static TrailNoiseVector Lerp(TrailNoiseVector a, TrailNoiseVector b, float t) {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
    }
    const TrailNoiseVector &At(int x, int y, int z) const {
        return samples[((z & 31) * Side + (y & 31)) * Side + (x & 31)];
    }

  public:
    TrailNoiseVolume() {
        LegacyNoiseRandom random;
        auto channel = [&] {
            int byte = static_cast<int>(static_cast<float>(random.NextDouble()) * 255.f + .5f);
            return (byte / 255.f) * 2.f - 1.f;
        };
        for (auto &sample : samples)
            sample = {channel(), channel(), channel()};
    }
    TrailNoiseVector Sample(float x, float y, float z) const {
        // Normalized texture coordinates address texel centers at (i + .5)/32.
        x = x * Side - .5f;
        y = y * Side - .5f;
        z = z * Side - .5f;
        int ix = static_cast<int>(__builtin_floorf(x));
        int iy = static_cast<int>(__builtin_floorf(y));
        int iz = static_cast<int>(__builtin_floorf(z));
        float fx = x - ix, fy = y - iy, fz = z - iz;
        auto bottom = Lerp(Lerp(At(ix, iy, iz), At(ix + 1, iy, iz), fx),
                           Lerp(At(ix, iy + 1, iz), At(ix + 1, iy + 1, iz), fx), fy);
        auto top = Lerp(Lerp(At(ix, iy, iz + 1), At(ix + 1, iy, iz + 1), fx),
                        Lerp(At(ix, iy + 1, iz + 1), At(ix + 1, iy + 1, iz + 1), fx), fy);
        return Lerp(bottom, top, fz);
    }
};

inline const TrailNoiseVolume &SharedTrailNoise() {
    static const TrailNoiseVolume volume;
    return volume;
}

template <typename V>
V DisplaceTrailVertex(V world, float scale, float scroll, float amount, const TrailNoiseVolume &volume) {
    if (amount <= .0001f)
        return world;
    auto noise = volume.Sample(world.x * scale * .03125f + scroll,
                               world.y * scale * .03125f + scroll,
                               world.z * scale * .03125f + scroll);
    return {world.x + noise.x * amount, world.y + noise.y * amount, world.z + noise.z * amount};
}
} // namespace VainSabers
