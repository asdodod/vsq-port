#pragma once
namespace VainSabers {
// Seeded System.Random sequence used by PC SaberRibbonTrail (seed 12345).
// Generate it natively once, avoiding 98,304 managed calls at scene startup.
class LegacyNoiseRandom {
    static constexpr int Max = 2147483647;
    int seed[56]{};
    int next = 0, nextPartner = 21;

  public:
    explicit LegacyNoiseRandom(int value = 12345) {
        int previous = 161803398 - value, current = 1;
        seed[55] = previous;
        for (int i = 1; i < 55; ++i) {
            int index = (21 * i) % 55;
            seed[index] = current;
            current = previous - current;
            if (current < 0)
                current += Max;
            previous = seed[index];
        }
        for (int pass = 0; pass < 4; ++pass)
            for (int i = 1; i < 56; ++i) {
                seed[i] -= seed[1 + (i + 30) % 55];
                if (seed[i] < 0)
                    seed[i] += Max;
            }
    }
    double NextDouble() {
        if (++next == 56)
            next = 1;
        if (++nextPartner == 56)
            nextPartner = 1;
        int value = seed[next] - seed[nextPartner];
        if (value == Max)
            --value;
        if (value < 0)
            value += Max;
        seed[next] = value;
        return value * (1.0 / Max);
    }
};
} // namespace VainSabers
